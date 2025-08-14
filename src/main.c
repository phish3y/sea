#include "vk.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <stdio.h>

int main(void)
{
    Vk v;

    if (!vk_init(&v)) {
        fprintf(stderr, "failed to init vulkan\n");
        return 1;
    }

    fprintf(stdout, "vulkan ready\n");

    if (!triangle(&v, 800, 600)) {
        fprintf(stderr, "failed to create triangle\n");
        return 1;
    }

    vk_cleanup(&v);
    return 0;
}