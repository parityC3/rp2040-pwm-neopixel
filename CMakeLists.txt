cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(pico_examples C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# Initialize the SDK
pico_sdk_init()

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-maybe-uninitialized
        )

add_executable(neo_pixel
        neo_pixel.c
        )

# Add pico_multicore which is required for multicore functionality
target_link_libraries(neo_pixel pico_stdlib hardware_pwm hardware_dma pico_rand)

# enable usb output, disable uart output
pico_enable_stdio_usb(neo_pixel 1)
pico_enable_stdio_uart(neo_pixel 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(neo_pixel)
