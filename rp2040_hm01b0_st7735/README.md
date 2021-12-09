# Pico4MLcbot hm01b0 camera and st7735 display demo 


## Hardware Connection
![IMAGE ALT TEXT](photos/Pico4MLcbot.15pc.jpg)
```
int PIN_LED = 25;
int PIN_CAM_SIOC = 5; // I2C0 SCL
int PIN_CAM_SIOD = 4; // I2C0 SDA
int PIN_CAM_RESETB = 2;
int PIN_CAM_XCLK = 3;
int PIN_CAM_VSYNC = 16;     //GP15 hsync  GP14 pixel clock     
int PIN_CAM_Y2_PIO_BASE = 6;   // data GPIO6

int EPD_RST_PIN     = 7;  // st7735 display
int EPD_DC_PIN      = 9;
int EPD_CS_PIN      = 13;
int EPD_CLK_PIN     = 10;
int EPD_MOSI_PIN    = 11;

int DRV8833_AIN2    = 21; // forw left
int DRV8833_AIN1    = 20; // back left
int DRV8833_BIN1    = 19; // forw right
int DRV8833_BIN2    = 18; // back right

int ESP1_RX         = 0;  // ESP8285 for Wifi
int ESP1_TX         = 1;
```

## Clone repo 
```bash
    git clone https://github.com/Hermann-SW/RPI-Pico-Cam.git
```
## Compile rp2040_hm01b0_st7735 demo
```bash
    cd RPI-Pico-Cam/rp2040_hm01b0_st7735
    mkdir build
    cd build
    cmake ..
    cd build 
    make 
```
## Copy the arducam_firmware.uf2 to pico
```bash
    cp arducam_firmware.uf2 /media/pi/RPI-RP2/
```

## Comments

It turned out that "capture camera frame, then show on display" loop runs sqrt(2) times faster on core1 (higher processing framerate). In addition another factor 2 was achieved by going to the "safe" double frequency 250MHz overclocking. With that, now 15.6 frames per second get shown on 160x80 display.

## test
[https://www.youtube.com/watch?v=CXhm8GdwkNc](https://www.youtube.com/watch?v=CXhm8GdwkNc)  
![IMAGE ALT TEXT](photos/Pico4ML-BLE.anim.gif)  
![IMAGE ALT TEXT](photos/20211207_202449.part.50pc.jpg)  
![IMAGE ALT TEXT](photos/20211207_220948.part.25pc.jpg)  
![IMAGE ALT TEXT](photos/20211124_204656.part.20pc.jpg)

## Thanks
 Special thanks to Hermann-SW for providing the code
