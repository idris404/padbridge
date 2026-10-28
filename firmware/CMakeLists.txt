cmake_minimum_required(VERSION 3.16)

set(PICO_BOARD pico2_w CACHE STRING "Target board: pico2_w or pico_w")

if(NOT DEFINED ENV{PICO_SDK_PATH})
    message(FATAL_ERROR "Set PICO_SDK_PATH to a Raspberry Pi Pico SDK checkout")
endif()

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(padbridge_switch_firmware C CXX ASM)

set(PADBRIDGE_STICK_DEADZONE 2500 CACHE STRING
    "Axial stick deadzone in signed 16-bit units (0..32766)")
if(PADBRIDGE_STICK_DEADZONE LESS 0 OR PADBRIDGE_STICK_DEADZONE GREATER 32766)
    message(FATAL_ERROR "PADBRIDGE_STICK_DEADZONE must be between 0 and 32766")
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

set(BLUEPAD32_ROOT ${CMAKE_CURRENT_LIST_DIR}/../third_party/bluepad32)
set(BTSTACK_ROOT $ENV{PICO_SDK_PATH}/lib/btstack)

# Bluepad32 and BTstack are separate CMake targets, so their configuration
# headers must be visible globally before Bluepad32 is added.
include_directories(
    ${CMAKE_CURRENT_LIST_DIR}
    ${BTSTACK_ROOT}/src
    ${BTSTACK_ROOT}/3rd-party/bluedroid/encoder/include
    ${BTSTACK_ROOT}/3rd-party/bluedroid/decoder/include
)

add_subdirectory(
    ${BLUEPAD32_ROOT}/src/components/bluepad32
    ${CMAKE_CURRENT_BINARY_DIR}/bluepad32)

add_executable(padbridge_switch
    main.c
    bluetooth_platform.c
    shared_state.c
    usb_callbacks.c
    ../src/input_filter.c
    ../src/rumble.c
    ../src/runtime.c
    ../src/switch_protocol.c
    ../src/switch_report.c
    ../src/switch_usb_descriptors.c
)

target_include_directories(padbridge_switch PRIVATE
    .
    ../include
    ${BLUEPAD32_ROOT}/src/components/bluepad32/include
    ${BTSTACK_ROOT}/src
    ${BTSTACK_ROOT}/3rd-party/bluedroid/encoder/include
    ${BTSTACK_ROOT}/3rd-party/bluedroid/decoder/include
)

target_compile_options(padbridge_switch PRIVATE
    -Wall
    -Wextra
)

target_compile_definitions(padbridge_switch PRIVATE
    PADBRIDGE_STICK_DEADZONE=${PADBRIDGE_STICK_DEADZONE}u
)

target_link_libraries(padbridge_switch PRIVATE
    pico_stdlib
    pico_multicore
    pico_cyw43_arch_none
    pico_btstack_classic
    pico_btstack_cyw43
    bluepad32
    tinyusb_device
    tinyusb_board
)

pico_enable_stdio_usb(padbridge_switch 0)
pico_enable_stdio_uart(padbridge_switch 0)
pico_add_extra_outputs(padbridge_switch)

