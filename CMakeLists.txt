cmake_minimum_required(VERSION 3.20)

set(CMAKE_C_COMPILER gcc)

project(sea C)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(VULKAN_SDK_DIR "${CMAKE_SOURCE_DIR}/external/vulkan/x86_64")
set(VULKAN_INCLUDE_DIR "${VULKAN_SDK_DIR}/include")
set(VULKAN_LIB_DIR "${VULKAN_SDK_DIR}/lib")

add_subdirectory(external/volk)

add_executable(sea
    src/main.c
    src/vk.c
)

target_compile_options(sea PRIVATE -Wall -Wextra -Wpedantic)

target_include_directories(sea PRIVATE
    src
    external/stb_image
    ${VULKAN_INCLUDE_DIR}
)

target_link_libraries(sea PRIVATE 
    volk
    ${VULKAN_LIB_DIR}/libvulkan.so
)
