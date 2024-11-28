cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)

set(CMAKE_C_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

project(shift)

pico_sdk_init()

add_subdirectory(src)
