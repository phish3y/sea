#include "vk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <volk.h>

static void gpu_info(const Vk* v);

bool vk_init(Vk* v)
{
    memset(v, 0, sizeof(*v));

    if (volkInitialize() != VK_SUCCESS) {
        fprintf(stderr, "failed to init volk\n");
        return false;
    }

    // Instance
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

    if (vkCreateInstance(&inst_info, NULL, &v->instance) != VK_SUCCESS) {
        fprintf(stderr, "failed to create instance\n");
        return false;
    }

    volkLoadInstance(v->instance);

    // -----

    // Physical Device
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(v->instance, &device_count, NULL);

    if (device_count == 0) {
        fprintf(stderr, "no vulkan physical devices found\n");
        return false;
    }

    VkPhysicalDevice devices[device_count];
    vkEnumeratePhysicalDevices(v->instance, &device_count, devices);

    v->physical_device = devices[0]; // FLAG hardcoded to first device

    // -----

    // Graphics queue
    uint32_t qf_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(v->physical_device, &qf_count, NULL);
    VkQueueFamilyProperties qf_props[qf_count];
    vkGetPhysicalDeviceQueueFamilyProperties(v->physical_device, &qf_count, qf_props);

    bool found = false;

    for (uint32_t i = 0; i < qf_count; i++) {
        if (qf_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            v->graphics_queue_family = i;
            found = true;
            break;
        }
    }

    if (!found) {
        fprintf(stderr, "no graphics queue found\n");
        return false;
    }

    // -----

    // Logical device + queue
    float q_priority = 1.0f;
    VkDeviceQueueCreateInfo q_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = v->graphics_queue_family,
        .queueCount = 1,
        .pQueuePriorities = &q_priority
    };

    VkDeviceCreateInfo dev_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &q_info
    };

    if (vkCreateDevice(v->physical_device, &dev_info, NULL, &v->device) != VK_SUCCESS) {
        fprintf(stderr, "failed to create device\n");
        return false;
    }

    volkLoadDevice(v->device);

    vkGetDeviceQueue(v->device, v->graphics_queue_family, 0, &v->graphics_queue);

    // -----

    gpu_info(v);

    return true;
}

static void gpu_info(const Vk* v)
{
    uint32_t qf_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(v->physical_device, &qf_count, NULL);
    VkQueueFamilyProperties qf_props[qf_count];
    vkGetPhysicalDeviceQueueFamilyProperties(v->physical_device, &qf_count, qf_props);

    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(v->physical_device, &props);

    const char* dev_type_str = "Unknown";

    switch (props.deviceType) {
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        dev_type_str = "Integrated GPU";
        break;

    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        dev_type_str = "Discrete GPU";
        break;

    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
        dev_type_str = "Virtual GPU";
        break;

    case VK_PHYSICAL_DEVICE_TYPE_CPU:
        dev_type_str = "CPU";
        break;

    default:
        break;
    }

    uint32_t api_version = props.apiVersion;
    uint32_t api_major = VK_VERSION_MAJOR(api_version);
    uint32_t api_minor = VK_VERSION_MINOR(api_version);
    uint32_t api_patch = VK_VERSION_PATCH(api_version);

    fprintf(stdout, "Selected GPU: %s\n", props.deviceName);
    fprintf(stdout, "  Type: %s\n", dev_type_str);
    fprintf(stdout, "  Vendor ID: 0x%04x, Device ID: 0x%04x\n", props.vendorID, props.deviceID);
    fprintf(stdout, "  Vulkan API Version: %u.%u.%u\n", api_major, api_minor, api_patch);

    fprintf(stdout, "Queue families:\n");

    for (uint32_t i = 0; i < qf_count; i++) {
        fprintf(stdout,
                "  #%u: flags=0x%x, count=%u\n",
                i,
                qf_props[i].queueFlags,
                qf_props[i].queueCount);
    }

    fprintf(stdout, "Selected graphics queue family: %u\n", v->graphics_queue_family);
}

void vk_cleanup(Vk* r)
{
    if (r->device)
        vkDestroyDevice(r->device, NULL);

    if (r->instance)
        vkDestroyInstance(r->instance, NULL);

    memset(r, 0, sizeof(*r));
}

static bool find_memory_type(
    VkPhysicalDevice pd,
    uint32_t type_bits,
    VkMemoryPropertyFlags props,
    uint32_t* memory_type
)
{
    VkPhysicalDeviceMemoryProperties mp;
    vkGetPhysicalDeviceMemoryProperties(pd, &mp);

    for (uint32_t i = 0; i < mp.memoryTypeCount; i++) {
        if ((type_bits & (1u << i)) && (mp.memoryTypes[i].propertyFlags & props) == props) {
            *memory_type = i;
            return true;
        }
    }

    fprintf(stderr, "no suitable memory type found\n");
    return false;
}

bool triangle(const Vk* v, uint32_t w, uint32_t h)
{
    // Command Pool
    VkCommandPoolCreateInfo pool_ci = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = v->graphics_queue_family
    };
    VkCommandPool cmd_pool;

    if (vkCreateCommandPool(v->device, &pool_ci, NULL, &cmd_pool) != VK_SUCCESS) {
        fprintf(stderr, "failed to create command pool\n");
        return false;
    }

    // -----

    // Command Buffers
    VkCommandBufferAllocateInfo cb_ai = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = cmd_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };

    VkCommandBuffer cmd_buffer;

    if (vkAllocateCommandBuffers(v->device, &cb_ai, &cmd_buffer) != VK_SUCCESS) {
        fprintf(stderr, "failed to create command buffers\n");
        return false;
    }

    // -----

    // Offscreen image
    VkImageCreateInfo img_ci = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .extent = { w, h, 1 },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };
    VkImage color_img;

    if (vkCreateImage(v->device, &img_ci, NULL, &color_img) != VK_SUCCESS) {
        fprintf(stderr, "failed to create image\n");
        return false;
    }

    VkMemoryRequirements img_reqs;
    vkGetImageMemoryRequirements(v->device, color_img, &img_reqs);

    uint32_t memory_type;

    if (!find_memory_type(v->physical_device, img_reqs.memoryTypeBits,
                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memory_type)) {
        fprintf(stderr, "failed to find memory type for image");
        return false;
    }

    VkMemoryAllocateInfo img_ai = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = img_reqs.size,
        .memoryTypeIndex = memory_type
    };

    VkDeviceMemory color_mem;

    if (vkAllocateMemory(v->device, &img_ai, NULL, &color_mem) != VK_SUCCESS) {
        fprintf(stderr, "failed to allocate memory for image");
        return false;
    }

    if (vkBindImageMemory(v->device, color_img, color_mem, 0) != VK_SUCCESS) {
        fprintf(stderr, "failed to bind memory for image");
        return false;
    }

    // -----

    VkImageViewCreateInfo view_ci = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = color_img,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0, .levelCount = 1,
            .baseArrayLayer = 0, .layerCount = 1
        }
    };
    VkImageView color_view;

    if (vkCreateImageView(v->device, &view_ci, NULL, &color_view) != VK_SUCCESS) {
        fprintf(stderr, "failed to create image view\n");
        return false;
    }

    return true;
}
