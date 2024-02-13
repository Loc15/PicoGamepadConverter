# Copyright (C) 1883 Thomas Edison - All Rights Reserved
# You may use, distribute and modify this code under the
# terms of the GPLv2 license, which unfortunately won't be
# written for another century.
#
# SPDX-License-Identifier: GPL-2.0-or-later
#
add_library(psx-lib INTERFACE)
target_include_directories(psx-lib INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(psx-lib INTERFACE hardware_pio hardware_dma)
target_sources(psx-lib INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/psx.c
    ${CMAKE_CURRENT_LIST_DIR}/psx.h
    ${CMAKE_CURRENT_LIST_DIR}/clock.pio)
pico_generate_pio_header(psx-lib ${CMAKE_CURRENT_LIST_DIR}/clock.pio)
