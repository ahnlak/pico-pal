# CMake to build all the examples for pico-pal

# Set basic stuff pre-project
cmake_minimum_required(VERSION 3.13)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
include(pico_sdk_import.cmake)

# Name the project
project(pal-examples C CXX)

# Initialise the Raspberry Pi Pico SDK and pico-pal
pico_sdk_init()
add_subdirectory(.. pico-pal)

# And then just include all our subdirs
add_subdirectory(ssd1306)
