cmake_minimum_required(VERSION 3.12)

#include(pico-sdk/pico_sdk_init.cmake)
# Pull in PICO SDK (must be before project)
include(pico_sdk_import.cmake)

project(rp2040_arducam)

pico_sdk_init()

add_executable(arducam_firmware
	arducam/arducam.c
	main.c
)

pico_generate_pio_header(arducam_firmware ${CMAKE_CURRENT_LIST_DIR}/image.pio)

target_link_libraries(arducam_firmware
	pico_stdlib
	hardware_dma
	hardware_i2c
	hardware_pio
	hardware_pwm
)
pico_add_extra_outputs(arducam_firmware)
