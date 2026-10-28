cmake_minimum_required(VERSION 3.16)

project(padbridge_core VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(padbridge_core
    src/input_filter.c
    src/rumble.c
    src/runtime.c
    src/switch_report.c
    src/switch_protocol.c
    src/switch_usb_descriptors.c
)

target_include_directories(padbridge_core PUBLIC include)

target_compile_options(padbridge_core PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

enable_testing()

add_executable(test_switch_report tests/test_switch_report.c)
target_link_libraries(test_switch_report PRIVATE padbridge_core)
add_test(NAME switch_report COMMAND test_switch_report)

add_executable(test_switch_protocol tests/test_switch_protocol.c)
target_link_libraries(test_switch_protocol PRIVATE padbridge_core)
add_test(NAME switch_protocol COMMAND test_switch_protocol)

add_executable(test_switch_usb_descriptors tests/test_switch_usb_descriptors.c)
target_link_libraries(test_switch_usb_descriptors PRIVATE padbridge_core)
add_test(NAME switch_usb_descriptors COMMAND test_switch_usb_descriptors)

add_executable(test_runtime tests/test_runtime.c)
target_link_libraries(test_runtime PRIVATE padbridge_core)
add_test(NAME runtime COMMAND test_runtime)

add_executable(test_input_filter tests/test_input_filter.c)
target_link_libraries(test_input_filter PRIVATE padbridge_core)
add_test(NAME input_filter COMMAND test_input_filter)

add_executable(test_rumble tests/test_rumble.c)
target_link_libraries(test_rumble PRIVATE padbridge_core)
add_test(NAME rumble COMMAND test_rumble)



