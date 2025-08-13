#pragma once
#include <vulkan/vulkan.h>
#include <stdbool.h>

typedef struct {
    VkInstance instance;
    VkPhysicalDevice physical_device;
    VkDevice device;
    VkQueue graphics_queue;
    uint32_t graphics_queue_family;
} Vk;

bool vk_init(Vk* r);
void vk_cleanup(Vk* r);