aux_source_directory(. DIR_LIB_SRCS)
add_library(arducam ${DIR_LIB_SRCS})
target_include_directories(arducam
  PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}/.
)

target_link_libraries(arducam pico_stdlib hardware_i2c hardware_spi)
# enable usb output, disable uart output


