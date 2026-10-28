cmake_minimum_required(VERSION 3.16)

project(padbridge_core VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(padbridge_core
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



