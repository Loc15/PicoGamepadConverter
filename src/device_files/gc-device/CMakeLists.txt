# SPDX-License-Identifier: GPL-3.0-or-later
#
add_library(gc-device INTERFACE)
target_include_directories(gc-device INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(gc-device INTERFACE pico_stdlib hardware_pio hardware_gpio)
target_sources(gc-device INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/gamecube_device.c
    ${CMAKE_CURRENT_LIST_DIR}/gamecube_device.h)
pico_generate_pio_header(gc-device ${CMAKE_CURRENT_LIST_DIR}/joybus.pio)
