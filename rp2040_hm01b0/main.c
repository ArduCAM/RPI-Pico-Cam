#include <stdio.h>
#include "pico/stdlib.h"
#include "arducam/arducam.h"
uint8_t image_buf[324*324];
uint8_t image_tmp[162*162];
uint8_t image[96*96];
uint8_t header[2] = {0x55,0xAA};

int main() {
	stdio_uart_init();
	//printf("\n\nBooted!\n");
	gpio_init(PIN_LED);
	gpio_set_dir(PIN_LED, GPIO_OUT);
	struct arducam_config config;
	config.sccb = i2c0;
	config.sccb_mode = I2C_MODE_16_8;
	config.sensor_address = 0x24;
	config.pin_sioc = PIN_CAM_SIOC;
	config.pin_siod = PIN_CAM_SIOD;
	config.pin_resetb = PIN_CAM_RESETB;
	config.pin_xclk = PIN_CAM_XCLK;
	config.pin_vsync = PIN_CAM_VSYNC;
	config.pin_y2_pio_base = PIN_CAM_Y2_PIO_BASE;

	config.pio = pio0;
	config.pio_sm = 0;

	config.dma_channel = 0;
	config.image_buf = image_buf;
	config.image_buf_size = sizeof(image_buf);

	arducam_init(&config);
	uint16_t x, y,i, j, index;
	while (true) {
		gpio_put(PIN_LED, !gpio_get(PIN_LED));
		arducam_capture_frame(&config);
	i = 0;
	index = 0;
	uint8_t temp = 0;
	for(y = 0; y< 324; y++){
		for(x = (1+x)%2; x<324; x+=2){
			image_tmp[index++] = config.image_buf[y*324+x];
		}
	}
	index = 0;
	for(y = 33; y< 129; y ++){
		for(x = 33; x<129; x++){
			image[index++] = image_tmp[y*324+x];
		}
	}
		uart_write_blocking(uart0, header, 2);
		//uart_write_blocking(uart0, config.image_buf, config.image_buf_size);
		uart_write_blocking(uart0, image, 96*96);	
	}

	return 0;
}
