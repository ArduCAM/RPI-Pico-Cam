#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/spi.h"
#include "pico/binary_info.h"
#include "src/arducam.h"

int main() {
    uint8_t id_H, id_L, spiTestVal;
    stdio_init_all();
       // This example will use I2C0 on GPIO4 (SDA) and GPIO5 (SCL)
    i2c_init(I2C_PORT, 100 * 1000);
    gpio_set_function(PIN_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PIN_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(PIN_SDA);
    gpio_pull_up(PIN_SCL);
    // Make the I2C pins available to picotool
    bi_decl( bi_2pins_with_func(PIN_SDA, PIN_SCL, GPIO_FUNC_I2C));
     // This example will use SPI0 at 0.5MHz.
    spi_init(SPI_PORT, 8 * 1000*1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);
// Set up our UART with the required speed.
    uart_init(UART_ID, BAUD_RATE);
    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    // Set our data format
    uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);
    // Turn off FIFO's - we want to do this character by character
    uart_set_fifo_enabled(UART_ID, false);
    

    rdSensorReg8_8(0x0A,&id_H);
    rdSensorReg8_8(0x0B,&id_L);
    if(id_H == 0x26 && (id_L == 0x40 || id_L == 0x42))
        printf("Find arducam ov2640 id_H: %x idL: %x\r\n",id_H, id_L);
    write_reg(0x00, 0x55);
    spiTestVal = read_reg(0x00);
    if(spiTestVal == 0x55)
        printf("SPI bus normal, write: 0x55 read: %x\r\n",spiTestVal);
    ov2640Init();
    OV2640_set_JPEG_size(OV2640_320x240);
    while (true) {
            singleCapture();
    }
    return 0;
}
