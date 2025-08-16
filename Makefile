VOLK_DIR = external/volk

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
	glslc shaders/triangle.vert -o shaders/triangle.vert.spv
	glslc shaders/triangle.frag -o shaders/triangle.frag.spv	

.PHONY: build
build:
	mkdir -p build && cd build && cmake .. -G Ninja && ninja

.PHONY: run
run: build
	./build/sea

.PHONY: fmt
fmt:
	astyle --style=kr --indent=spaces=4 --break-blocks --max-code-length=100 --suffix=none src/*.c