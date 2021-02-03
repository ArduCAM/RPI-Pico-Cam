aux_source_directory(. DIR_SRCS)
add_subdirectory(src)
add_executable(arducam_demo ${DIR_SRCS})
# Pull in our pico_stdlib which pulls in commonly used features
target_link_libraries(arducam_demo pico_stdlib hardware_i2c hardware_spi arducam)
# enable usb output, disable uart output
pico_enable_stdio_usb(arducam_demo 1)
pico_enable_stdio_uart(arducam_demo 0)
# create map/bin/hex file etc.
pico_add_extra_outputs(arducam_demo)

# add url via pico_set_program_url
example_auto_set_url(arducam_demo)
