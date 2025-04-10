cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

#set(PICO_BOARD pico_w)

set(PICO_SDK_PATH ${CMAKE_CURRENT_LIST_DIR}/../pico-sdk)
set(PICOTOOL_FETCH_FROM_GIT_PATH ${CMAKE_CURRENT_LIST_DIR}/../picotool)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

set(target_name PicoGamepadConverter)

set(PICO_PIO_USB_DIR "${CMAKE_CURRENT_LIST_DIR}/../Pico-PIO-USB/")

#Until pico-sdk is updated
set(PICO_TINYUSB_PATH "${CMAKE_CURRENT_LIST_DIR}/../tinyusb")

# the second argument to add_subdirectory is needed here because this isn't
# a subdirectory, it's out of tree.

project(${target_name} C CXX ASM)

pico_sdk_init()

add_executable(${target_name})

target_sources(${target_name} PRIVATE
 main.c
 convert_data.c
 utils.c
 device_files/usb_descriptors.c
 device_files/xinput/xinput_driver.c
 device_files/switch/hid_driver.c
 host_files/xinput_host.c
 # can use 'tinyusb_pico_pio_usb' library later when pico-sdk is updated
 ${PICO_TINYUSB_PATH}/src/portable/raspberrypi/pio_usb/dcd_pio_usb.c
 #${PICO_TINYUSB_PATH}/src/portable/raspberrypi/pio_usb/hcd_pio_usb.c
 #Use modify version to enable native and pio
 hcd_pio_usb.c
 )

add_subdirectory(${PICO_PIO_USB_DIR} pico_pio_usb)

add_subdirectory(host_files/ps2kbd-lib)
add_subdirectory(host_files/psx-lib)
add_subdirectory(host_files/parser-lib)
add_subdirectory(host_files/bluehost-lib)
add_subdirectory(device_files/rndis-lib)
add_subdirectory(device_files/bluehid-lib)
add_subdirectory(device_files/psx-device)
add_subdirectory(device_files/gc-device)
add_subdirectory(device_files/wiimote-lib)
add_subdirectory(flash-lib)

pico_enable_stdio_uart(${target_name} 1)

 # print memory usage, enable all warnings
target_link_options(${target_name} PRIVATE -Xlinker --print-memory-usage)
target_compile_options(${target_name} PRIVATE -Wall -Wextra)

# use tinyusb implementation
target_compile_definitions(${target_name} PRIVATE PIO_USB_USE_TINYUSB)

# needed so tinyusb can find tusb_config.h
target_include_directories(${target_name} PRIVATE ${CMAKE_CURRENT_LIST_DIR} device_files/switch device_files/xinput host_files)

target_link_libraries(${target_name} PRIVATE pico_stdlib pico_pio_usb tinyusb_device tinyusb_host ps2kbd-lib psx-lib rndis-lib parser-lib
        flash-lib psx-device gc-device)

IF(${PICO_BOARD} STREQUAL "pico_w" OR ${PICO_BOARD} STREQUAL "pico2_w")
    target_link_libraries(${target_name} PRIVATE pico_cyw43_arch_none bluehost-lib bluehid-lib wiimote-lib)
    add_compile_definitions(PICO_W=1)
endif()


pico_add_extra_outputs(${target_name})

