# SPDX-License-Identifier: GPL-3.0-or-later
#
add_library(psx-device INTERFACE)
target_include_directories(psx-device INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(psx-device INTERFACE pico_stdlib hardware_pio hardware_gpio)
target_sources(psx-device INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/controller_simulator.c
    ${CMAKE_CURRENT_LIST_DIR}/controller_simulator.h)
pico_generate_pio_header(psx-device ${CMAKE_CURRENT_LIST_DIR}/psxSPI.pio)
