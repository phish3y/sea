#pragma once

#define VK_NO_PROTOTYPES
#include <stdbool.h>
#include <volk.h>

typedef struct {
    VkInstance instance;
    VkPhysicalDevice physical_device;
    VkDevice device;
    VkQueue graphics_queue;
    uint32_t graphics_queue_family;
} Vk;

bool vk_init(Vk* v);
void vk_cleanup(Vk* v);
bool triangle(const Vk* v, uint32_t w, uint32_t h);