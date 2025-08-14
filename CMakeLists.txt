cmake_minimum_required(VERSION 3.20)

set(CMAKE_C_COMPILER gcc)

project(sea C)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)

add_subdirectory(external/volk)

add_executable(sea
    src/main.c
    src/vk.c
)

target_compile_options(sea PRIVATE -Wall -Wextra -Wpedantic)

target_include_directories(sea PRIVATE
    src
    external/stb_image
)

target_link_libraries(sea PRIVATE 
    volk
    Vulkan::Vulkan
)
