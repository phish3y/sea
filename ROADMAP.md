# project concept

**Remote 3D Renderer**
A C backend renders a scene with Vulkan **offscreen** and streams frames to a browser over **WebSocket**. The browser displays frames and sends camera/interaction commands back.

Why this stack?

* **C + Vulkan**: modern graphics + low-level chops.
* **POSIX sockets ⇢ WebSocket**: browsers can’t open raw TCP; using WebSocket still proves your networking skills (framing, backpressure, latency).
* **No heavy engines**: showcases systems skills, not framework glue.

---

# tech choices (lean + C-first)

* **Language**: C17
* **Build**: CMake
* **Vulkan**: Vulkan SDK + **volk** (loader) + **cglm** (math)
* **Headless/offscreen**: render to a `VkImage`/`VkBuffer` (no window system)
* **Image encode**: **stb\_image\_write** to start; optional **libjpeg-turbo** later
* **Networking**: **libwebsockets** (pure C) for WebSocket server (wss-ready)
* **Serialization**: tiny **JSON** with **cJSON** (for control messages)
* **Frontend**: simple HTML + JS (Canvas) — no framework needed

---

# repo layout

```
remote-3d-renderer/
  CMakeLists.txt
  third_party/ (volk, cglm, stb, libwebsockets, cJSON as submodules or vendored)
  src/
    main.c
    net_ws.c / net_ws.h        (WebSocket server, framing, control loop)
    render_vk.c / render_vk.h  (instance/dev, pipeline, offscreen pass)
    image_io.c / image_io.h    (RGBlike->PNG/JPEG)
    proto.h                    (message structs)
    util/...
  shaders/
    vert.spv
    frag.spv
  web/
    index.html
    client.js
    styles.css
```

---

# message & streaming protocol (simple, debuggable)

**Transport:** one WebSocket with two logical channels (type-tagged messages).
**Frames → client** (binary):

```
[1 byte type = 0x01][4 bytes seq][8 bytes timestamp_ns][4 bytes w][4 bytes h]
[1 byte fmt=1(PNG)|2(JPEG)|3(RGB)][payload...]
```

**Control ←→ both ways** (text JSON):

```json
{ "type":"camera", "orbit_deg": [az, el], "distance": d }
{ "type":"resize", "width": 1280, "height": 720 }
{ "type":"stats_req" }
{ "type":"stats", "fps": 58.2, "encode_ms": 2.4, "latency_ms": 34.1 }
```

**Backpressure:** if `libwebsockets` send buffer is full, drop oldest un-sent frame (configurable), always deliver the newest.

---

# milestone plan

## M0 — scaffold

* CMake config; add/verify third\_party.
* `render_vk.c`: init Vulkan (volk), pick device/queue, create command pool.
* Build & run a smoke test.

**Done when**: app starts and exits cleanly with Vulkan instance/device created.

## M1 — headless triangle → PNG

* Create offscreen color image + render pass + framebuffer.
* Full pipeline (triangle), render one frame, copy to mapped buffer.
* Write `frame_0001.png` (stb).

**Done when**: PNG on disk looks right.

## M2 — mesh + basic lighting

* Add cglm; MVP matrix; simple Phong or lambert.
* Hardcode a cube or load a tiny `.obj` parser (or keep it hardcoded for “pure C”).
* Render N frames with rotation; save PNGs.

**Done when**: a rotating lit cube renders to PNG sequence.

## M3 — WebSocket streaming (server)

* Add libwebsockets; spin up `ws://localhost:8080/stream`.
* Convert rendered frame → PNG in-memory.
* Send frame messages per the binary format above.
* Keep a ring buffer for frames, send on writeable callback.

**Done when**: a test client receives binary frames.

## M4 — Browser client

* `web/index.html + client.js`:

  * Connect WebSocket, parse headers, draw to `<canvas>`:

    * For PNG/JPEG: create `Blob` → `ImageBitmap` or `<img>` → drawImage to Canvas.
  * Keyboard/mouse → JSON control messages (`camera`, `resize`).
  * Show live stats overlay (fps, latency from message headers).

**Done when**: open `index.html`, see live stream & can orbit camera.

## M5 — performance pass

* Switch PNG → **JPEG** (libjpeg-turbo) for 2–5× faster encode and less bandwidth.
* Use **double-buffered command buffers** to overlap render/encode/send.
* Optional: add a **raw RGB** path (fmt=3) and decode client-side with **WebCodecs** (if you want max perf without implementing a video codec).

## M6 — polish

* **wss** with TLS (OpenSSL via libwebsockets).
* Config flags: `--width`, `--height`, `--fps`, `--jpeg-quality`.
* Minimal logging + `/metrics` (Prometheus text over HTTP or JSON).
* Dockerfile + README + demo GIF.

---

# build tips (Linux)

```bash
# deps (Ubuntu-ish)
sudo apt-get install build-essential cmake ninja-build libssl-dev
# Vulkan SDK: install from LunarG or distro; ensure VK layers visible

git submodule add https://github.com/zeux/volk third_party/volk
git submodule add https://github.com/recp/cglm third_party/cglm
# stb: just vendor stb_image_write.h
git submodule add https://github.com/warmcat/libwebsockets third_party/libwebsockets
git submodule add https://github.com/DaveGamble/cJSON third_party/cJSON
# optional later
sudo apt-get install libjpeg-turbo8-dev
```

CMake (sketch):

```cmake
cmake_minimum_required(VERSION 3.20)
project(remote_3d_renderer C)
set(CMAKE_C_STANDARD 17)
add_subdirectory(third_party/libwebsockets)
add_library(volk STATIC third_party/volk/volk.c)
target_compile_definitions(volk PUBLIC VOLK_IMPLEMENTATION)

add_executable(r3d
  src/main.c src/net_ws.c src/render_vk.c src/image_io.c src/proto.h)
target_include_directories(r3d PRIVATE third_party/volk third_party/cglm third_party)
target_link_libraries(r3d PRIVATE volk cglm websockets ssl crypto m)
```

---

# key implementation notes

* **Headless render**: no surface needed. Render to a color `VkImage` (transfer-src capable), then `vkCmdCopyImageToBuffer` → CPU encode.
* **Frame pacing**: render loop targets `--fps`, but never blocks the network thread; use a lock-free ring or `mpsc` queue (even in C you can do a simple single-producer/single-consumer).
* **Latency stamps**: put `clock_gettime(CLOCK_MONOTONIC, …)` in the header, compute one-way latency in JS (`Date.now()` is fine for demo).
* **Backpressure**: if `lws_write()` says not writeable, keep last frame and drop older queued frames — viewers prefer freshness over completeness.
* **Security**: validate JSON sizes; cap frame size; consider per-message deflate disabled (you’re already compressing images).

---

# stretch goals (pick a couple for resume bullets)

* **gRPC-Web variant**: run an Envoy proxy in front; server in C using gRPC C core (harder, but nice extra bullet).
* **Multi-client**: broadcast last frame + incremental diffs (e.g., region hashes) or just independent encoding per client (simpler).
* **Video**: integrate a C H.264 encoder (e.g., x264) → dramatic bandwidth reduction; decode via `<video>` or WebCodecs.
* **Asset server**: tiny HTTP route `/mesh` and `/shader` with hot-reload.
* **Auth**: token in query string, check at handshake; rotate.

---

# demo script (what recruiters will see)

1. `./r3d --width 1280 --height 720 --fps 60 --jpeg-quality 85`
2. open `web/index.html` → it autoconnects to `ws://localhost:8080/stream`
3. drag to orbit, scroll to zoom; watch live **FPS** and **latency** overlay.
4. toggle wireframe / lighting with `L`/`W` keys (send JSON control).

---

# next step (today)

If you want, I can:

* generate a minimal **CMake starter** with `render_vk.c` that writes a triangle PNG (Milestone M1), and
* a tiny `web/index.html + client.js` that connects and shows a placeholder until frames arrive.

Say the word and I’ll drop that starter scaffold so you can `cmake .. && ninja && ./r3d`.

