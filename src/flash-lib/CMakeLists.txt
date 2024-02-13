add_library(flash-lib INTERFACE)
target_include_directories(flash-lib INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(flash-lib INTERFACE pico_stdlib hardware_flash hardware_sync)
target_sources(flash-lib INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/pico_flash.c)