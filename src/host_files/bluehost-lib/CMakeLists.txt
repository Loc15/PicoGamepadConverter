add_library(bluehost-lib INTERFACE)
target_include_directories(bluehost-lib INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(bluehost-lib INTERFACE pico_stdlib pico_btstack_classic pico_btstack_cyw43
  	pico_cyw43_arch_none)
target_sources(bluehost-lib INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/blue_host.c
    ${CMAKE_CURRENT_LIST_DIR}/blue_host.h)
