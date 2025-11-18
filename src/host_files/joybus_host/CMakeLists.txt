add_library(joybus_host INTERFACE)
target_include_directories(joybus_host INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(joybus_host INTERFACE pico_stdlib hardware_pio)
target_sources(joybus_host INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/joybus_controller.c
        ${CMAKE_CURRENT_LIST_DIR}/joybus_controller.h
        ${CMAKE_CURRENT_LIST_DIR}/n64_controller.c
        ${CMAKE_CURRENT_LIST_DIR}/n64_controller.h)
#add_subdirectory(gamecube)
#add_subdirectory(n64)
pico_generate_pio_header(joybus_host ${CMAKE_CURRENT_LIST_DIR}/controller.pio)
