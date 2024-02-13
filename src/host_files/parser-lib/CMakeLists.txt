add_library(parser-lib INTERFACE)
target_include_directories(parser-lib INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(parser-lib INTERFACE pico_stdlib)
target_sources(parser-lib INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/gamepad_parser.c
    ${CMAKE_CURRENT_LIST_DIR}/hidparser.c)
