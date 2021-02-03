aux_source_directory(. DIR_LIB_SRCS)
add_library(arducam ${DIR_LIB_SRCS})
target_link_libraries(arducam pico_stdlib hardware_i2c hardware_spi)

