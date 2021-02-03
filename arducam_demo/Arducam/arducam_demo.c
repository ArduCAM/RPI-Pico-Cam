#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/spi.h"
#include "pico/binary_info.h"
#include "src/arducam.h"
int main() {
    uint8_t id_H, id_L, spiTestVal;
    arducam.systemInit();
    if(arducam.busDetect()){
        return 1;
    }
    if(arducam.cameraProbe()){
        return 1;
    }
    arducam.cameraInit();
    arducam.setJpegSize(res_320x240);
    while (true) {
         singleCapture();
    }
    return 0;
}
