#Hasta que se realicen los cambios
#set(PICO_TINYUSB_PATH ${CMAKE_CURRENT_LIST_DIR}/../tinyusb)
add_library(rndis-lib INTERFACE)
target_include_directories(rndis-lib INTERFACE ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/mongoose)
target_link_libraries(rndis-lib INTERFACE pico_stdlib tinyusb_device flash-lib)
target_sources(rndis-lib INTERFACE
    rndis.c
    mongoose/mongoose.c
    packed_fs.c
    ${PICO_TINYUSB_PATH}/lib/networking/rndis_reports.c)

target_include_directories(rndis-lib INTERFACE
  .
  ${PICO_TINYUSB_PATH}/lib/networking
  ${CMAKE_CURRENT_SOURCE_DIR})

# Mongoose build flags
target_compile_definitions(rndis-lib INTERFACE -DMG_ENABLE_TCPIP=1)
target_compile_definitions(rndis-lib INTERFACE -DMG_ENABLE_PACKED_FS=1)
target_compile_definitions(rndis-lib INTERFACE -DMG_ENABLE_FILE=0)
target_compile_definitions(rndis-lib INTERFACE -DDISABLE_ROUTING=1)

# Example build options
target_compile_definitions(rndis-lib INTERFACE -DHTTP_URL="http://0.0.0.0/")

# Extra build flags (enable if needed)
target_compile_definitions(rndis-lib INTERFACE -DDUAL_CONFIG=0)