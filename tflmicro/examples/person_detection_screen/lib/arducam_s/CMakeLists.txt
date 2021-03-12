if (NOT TARGET arducam_s)
    add_library(arducam_s INTERFACE)

    pico_generate_pio_header(arducam_s ${CMAKE_CURRENT_LIST_DIR}/image.pio)

    target_sources(arducam_s INTERFACE
            ${CMAKE_CURRENT_LIST_DIR}/arducam_s.c)

    target_link_libraries(arducam_s INTERFACE pico_stdlib hardware_i2c hardware_pio hardware_dma)
endif()
