

.PHONY: vulkan
vulkan:
	mkdir -p external/vulkan
	curl -Lo external/vulkan/vulkan.tar https://sdk.lunarg.com/sdk/download/1.4.321.1/linux/vulkansdk-linux-x86_64-1.4.321.1.tar.xz
	tar -xf external/vulkan/vulkan.tar -C external/vulkan --strip-components=1

.PHONY: volk
volk:
	@git submodule update --init --recursive

.PHONY: stb
stb:
	mkdir -p external/stb_image
	curl -Lo external/stb_image/stb_image_write.h https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h

.PHONY: deps
deps: volk stb

.PHONY: shaders
shaders:
	external/vulkan/x86_64/bin/glslc src/shaders/triangle.vert -o src/shaders/triangle.vert.spv
	external/vulkan/x86_64/bin/glslc src/shaders/triangle.frag -o src/shaders/triangle.frag.spv

.PHONY: build
build: shaders
	mkdir -p build && cd build && cmake .. -G Ninja && ninja

.PHONY: run
run: build
	./build/sea

.PHONY: fmt
fmt:
	astyle --style=kr --indent=spaces=4 --break-blocks --max-code-length=100 --suffix=none src/*.c