# PICO Arducam Examples

Table of contents
=================

   * [Table of contents](#table-of-contents)
   * [Getting Started](#Getting-Started)
   * [Quick Pico Setup](#Quick-Pico-Setup)
   * [Arducam Demo](#Get-Arducam)
      * [Download and Build](#Get-Arducam)
      * [Test Arducam demo](#Test-Arducam-demo)
   * [Person Detection](#Person-Detection)
      * [Download and Build](#Person-Detection)
      * [Test Person Detection](#Test-Person-Detection)
        * [View output](#View-output)
      * [Person Detection Diagram](#Person-Detection-Diagram)

# Getting started

See [Getting Started with the Raspberry Pi Pico](https://rptl.io/pico-get-started) and the README in the [pico-sdk](https://github.com/raspberrypi/pico-sdk) for information
on getting up and running.

# Quick Pico Setup
If you are developing for Raspberry Pi Pico on the Raspberry Pi 4B, or the Raspberry Pi 400, most of the installation steps
in this Getting Started guide can be skipped by running the setup script. You can get this script by doing the following:
```bash
git clone https://github.com/raspberrypi/pico-setup.git
```
Then run: 
```bash
 pico-setup/pico_setup.sh
```
The script will:

- Create a directory called pico
- Install required dependencies
- Download the pico-sdk, pico-examples, pico-extras, and pico-playground repositories
- Define PICO_SDK_PATH, PICO_EXAMPLES_PATH, PICO_EXTRAS_PATH, and PICO_PLAYGROUND_PATH in your ~/.bashrc
- Build the blink and hello_world examples in pico-examples/build/blink and pico-examples/build/hello_world
- Download and build picotool (see Appendix B). Copy it to /usr/local/bin. • Download and build picoprobe (see Appendix A).
- Download and compile OpenOCD (for debug support)
- Download and install Visual Studio Code
- Install the required Visual Studio Code extensions (see Chapter 6 for more details)
- Configure the Raspberry Pi UART for use with Raspberry Pi Pico

Once it has run, you will need to reboot your Raspberry Pi,
```bash
sudo reboot
```

### Get Arducam
- Download RPI-Pico-Cam
```bash 
git clone https://github.com/ArduCAM/RPI-Pico-Cam.git
```
- Compile 
```bash
cd RPI-Pico-Cam/arducam_demo
mkdir build 
cd build 
cmake ..
```
![IMAGE ALT TEXT](data/1.png)
```bash
make
```
![IMAGE ALT TEXT](data/2.png)  
Then you will creat some files under RPI-Pico-Cam/arducam_demo/build/Arducam path 
Bin|Description
---|---
[arducam_demo.elf](arducam_demo) | which is used by the debugger.
[arducam_demo.uf2](arducam_demo) | which can be dragged onto the RP2040 USB Mass Storage Device.

![IMAGE ALT TEXT](data/3.png)

### Test Arducam demo 

App|Description
---|---
[arducam_demo](arducam_demo) | This is a video streaming demo.

- Hardware connection 

![IMAGE ALT TEXT](data/5.png)

- Load and run arducam_demo 
The simplest method to load software onto a RP2040-based board is by mounting it as a USB Mass Storage Device.
Doing this allows you to drag a file onto the board to program the flash. Go ahead and connect the Raspberry Pi Pico to
your Raspberry Pi using a micro-USB cable, making sure that you hold down the BOOTSEL button to force it into
USB Mass Storage Mode.
![IMAGE ALT TEXT](data/4.png)

If you are logged in via ssh for example, you may have to mount the mass storage device manually:
```bash
$ dmesg | tail
[ 371.973555] sd 0:0:0:0: [sda] Attached SCSI removable disk
$ sudo mkdir -p /mnt/pico
$ sudo mount /dev/sda1 /mnt/pico
```
If you can see files in /mnt/pico then the USB Mass Storage Device has been mounted correctly:
```bash
$ ls /mnt/pico/
INDEX.HTM INFO_UF2.TXT
Copy your arducam_demo.uf2 onto RP2040:
sudo cp arducam_demo.uf2 /mnt/pico
sudo sync
```

## Person Detection
- Download RPI-Pico-Cam
```bash 
git clone https://github.com/ArduCAM/RPI-Pico-Cam.git
```
- Compile 
```bash
cd RPI-Pico-Cam/tflmicro
mkdir build 
cd build 
cmake ..
```
![IMAGE ALT TEXT](data/tflmicro_cmake_output.png)
```bash
make
```
![IMAGE ALT TEXT](data/tflmicro_make_output.png)  
Then you will creat some files under RPI-Pico-Cam/tflmicro/build/examples/person_detection path 
Bin|Description
---|---
[person_detection_int8.uf2](tflmicro/bin/person_detection_int8.uf2) | This is the main program of person_detection, which can be dragged onto the RP2040 USB Mass Storage Device.
[person_detection_benchmark.uf2](tflmicro/bin/person_detection_benchmark.uf2) | This is the benchmark program of person_detection, you can use it to test the performance of person_detection on pico.
[image_provider_benchmark.uf2](tflmicro/bin/image_provider_benchmark.uf2) | This is the benchmark program of image_provider, you can use it to test the performance of image data acquisition.
**Tips: If you don't want to compile, you can use the above pre-built uf2 file, you only need to wire the hardware and download uf2 to the device.**

![IMAGE ALT TEXT](data/tflmicro_output.png)

### Test Person Detection

App|Description
---|---
[person_detection_int8](tflmicro/examples/person_detection/main_functions.cpp) | This is a person detection demo.

- Hardware connection 

![IMAGE ALT TEXT](data/5.png)

- Load and run person_detection 
The simplest method to load software onto a RP2040-based board is by mounting it as a USB Mass Storage Device.
Doing this allows you to drag a file onto the board to program the flash. Go ahead and connect the Raspberry Pi Pico to
your Raspberry Pi using a micro-USB cable, making sure that you hold down the BOOTSEL button to force it into
USB Mass Storage Mode.
![IMAGE ALT TEXT](data/4.png)

If you are logged in via ssh for example, you may have to mount the mass storage device manually:
```bash
$ dmesg | tail
[ 371.973555] sd 0:0:0:0: [sda] Attached SCSI removable disk
$ sudo mkdir -p /mnt/pico
$ sudo mount /dev/sda1 /mnt/pico
```
If you can see files in /mnt/pico then the USB Mass Storage Device has been mounted correctly:
```bash
$ ls /mnt/pico/
INDEX.HTM INFO_UF2.TXT
```
Copy your person_detection_int8.uf2 onto RP2040:
```bash
sudo cp examples/person_detection/person_detection_int8.uf2 /mnt/pico
sudo sync
```

### View output

The person detection example outputs some information through usb, you can use minicom to view:
```bash
minicom -b 115200 -o -D /dev/ttyACM0
```
![minicom_output](data/minicom_output.png)

The person detection example also outputs the image data and person detection results to the UART, and we provide [a processing program](tflmicro/person_detection_display/person_detection_display.pde) to display them:

![no-person](data/no-person.png)
![person](data/person.png)

**Tips: You can download the Processing [here](https://processing.org/download/) or [Processing for Pi](https://pi.processing.org/download/).**


### Person Detection Diagram
![Person Detection Diagram](data/diagram.png)


## Person Detection Screen
- Download RPI-Pico-Cam
```bash 
git clone https://github.com/ArduCAM/RPI-Pico-Cam.git
```
- Compile
```bash
cd RPI-Pico-Cam/tflmicro
mkdir build 
cd build 
cmake ..
```
![IMAGE ALT TEXT](data/tflmicro_cmake_output.png)
```bash
make
```
![IMAGE ALT TEXT](data/tflmicro_make_output.png)  
Then you will creat some files under RPI-Pico-Cam/tflmicro/build/examples/person_detection_screen path

Bin|Description
---|---
[person_detection_screen_int8.uf2](tflmicro/bin/person_detection_screen_int8.uf2) | This is the main program of person_detection_screen, which can be dragged onto the RP2040 USB Mass Storage Device.

**Tips: If you don't want to compile, you can use the above pre-built uf2 file, you only need to wire the hardware and download uf2 to the device.**

![IMAGE ALT TEXT](data/tflmicro_output_screen.png)

### Test Person Detection Screen

App|Description
---|---
[person_detection_screen_int8](tflmicro/examples/person_detection_screen/main_functions.cpp) | This is a person detection screen demo.

- Hardware requirements

![IMAGE ALT TEXT](data/Pico4ML.png)

Learn more here: [pico4ml-an-rp2040-based-platform-for-tiny-machine-learning](https://www.arducam.com/pico4ml-an-rp2040-based-platform-for-tiny-machine-learning/)

- Load and run person_detection_screen
  The simplest method to load software onto a RP2040-based board is by mounting it as a USB Mass Storage Device.
  Doing this allows you to drag a file onto the board to program the flash. Go ahead and connect the Raspberry Pi Pico to
  your Raspberry Pi using a micro-USB cable, making sure that you hold down the BOOTSEL button to force it into
  USB Mass Storage Mode.
  ![IMAGE ALT TEXT](data/4.png)

If you are logged in via ssh for example, you may have to mount the mass storage device manually:
```bash
$ dmesg | tail
[ 371.973555] sd 0:0:0:0: [sda] Attached SCSI removable disk
$ sudo mkdir -p /mnt/pico
$ sudo mount /dev/sda1 /mnt/pico
```
If you can see files in /mnt/pico then the USB Mass Storage Device has been mounted correctly:
```bash
$ ls /mnt/pico/
INDEX.HTM INFO_UF2.TXT
```
Copy your person_detection_screen_int8.uf2 onto RP2040:
```bash
sudo cp examples/person_detection_screen/person_detection_screen_int8.uf2 /mnt/pico
sudo sync
```

### View output

The person detection screen example outputs some information through usb, you can use minicom to view:
```bash
minicom -b 115200 -o -D /dev/ttyACM0
```
![minicom_output](data/minicom_output_screen.png)

The person detection screen example also outputs the image data and person detection screen results to the UART, and you can see them directly on the screen.
![no-person](data/no_person_screen.jpg){}
![person](data/person_screen.jpg)
