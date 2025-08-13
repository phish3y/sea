VOLK_DIR = external/volk

.PHONY: volk
volk:
	@git submodule add https://github.com/zeux/volk $(VOLK_DIR)
	@git submodule update --init --recursive