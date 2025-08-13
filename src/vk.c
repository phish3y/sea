#include "vk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <volk.h>

bool vk_init(Vk* r) {
    memset(r, 0, sizeof(*r));

    if (volkInitialize() != VK_SUCCESS) {
        fprintf(stderr, "failed to init volk\n");
        return false;
    }

    VkApplicationInfo app_info = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "sea",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_2
    };

    VkInstanceCreateInfo inst_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &app_info
    };

    if (vkCreateInstance(&inst_info, NULL, &r->instance) != VK_SUCCESS) {
        fprintf(stderr, "failed to create instance\n");
        return false;
    }

    volkLoadInstance(r->instance);

    // // Pick physical device
    // uint32_t dev_count = 0;
    // vkEnumeratePhysicalDevices(r->instance, &dev_count, NULL);
    // if (dev_count == 0) {
    //     fprintf(stderr, "No Vulkan physical devices found\n");
    //     return false;
    // }
    // VkPhysicalDevice devices[16];
    // vkEnumeratePhysicalDevices(r->instance, &dev_count, devices);

    // r->phys_dev = devices[0]; // Just pick first for now

    // // Find graphics queue family
    // uint32_t qf_count = 0;
    // vkGetPhysicalDeviceQueueFamilyProperties(r->phys_dev, &qf_count, NULL);
    // VkQueueFamilyProperties qf_props[16];
    // vkGetPhysicalDeviceQueueFamilyProperties(r->phys_dev, &qf_count, qf_props);

    // bool found = false;
    // for (uint32_t i = 0; i < qf_count; i++) {
    //     if (qf_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
    //         r->graphics_queue_family = i;
    //         found = true;
    //         break;
    //     }
    // }
    // if (!found) {
    //     fprintf(stderr, "No graphics queue found\n");
    //     return false;
    // }

    // // Logical device + queue
    // float q_priority = 1.0f;
    // VkDeviceQueueCreateInfo q_info = {
    //     .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
    //     .queueFamilyIndex = r->graphics_queue_family,
    //     .queueCount = 1,
    //     .pQueuePriorities = &q_priority
    // };

    // VkDeviceCreateInfo dev_info = {
    //     .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    //     .queueCreateInfoCount = 1,
    //     .pQueueCreateInfos = &q_info
    // };

    // if (vkCreateDevice(r->phys_dev, &dev_info, NULL, &r->device) != VK_SUCCESS) {
    //     fprintf(stderr, "Failed to create device\n");
    //     return false;
    // }

    // volkLoadDevice(r->device);

    // vkGetDeviceQueue(r->device, r->graphics_queue_family, 0, &r->graphics_queue);

    // printf("Vulkan initialized successfully\n");
    return true;
}

void vk_cleanup(Vk* r) {
    if (r->device) vkDestroyDevice(r->device, NULL);
    if (r->instance) vkDestroyInstance(r->instance, NULL);
    memset(r, 0, sizeof(*r));
}
