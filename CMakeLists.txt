cmake_minimum_required(VERSION 3.20)
project(sea C)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_subdirectory(external/volk)

add_executable(sea
    src/main.c
    src/render_vk.c
)

target_include_directories(sea PRIVATE
    external/volk
    src
)

target_link_libraries(sea PRIVATE volk)
