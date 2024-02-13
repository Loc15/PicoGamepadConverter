add_library(bluehid-lib INTERFACE)
target_include_directories(bluehid-lib INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(bluehid-lib INTERFACE pico_stdlib pico_btstack_classic pico_btstack_cyw43
  	pico_cyw43_arch_none)
target_sources(bluehid-lib INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/blue_hid.c
    ${CMAKE_CURRENT_LIST_DIR}/blue_hid.h)
