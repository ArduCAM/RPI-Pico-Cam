/*
  This sketch reads a raw Stream of RGB565 pixels
 from the Serial port and displays the frame on
 the window.
 
 Use with the Examples -> CameraCaptureRawBytes Arduino sketch.
 
 This example code is in the public domain.
 */

import processing.serial.*;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

Serial myPort;

// must match resolution used in the sketch
final int cameraWidth = 96;
final int cameraHeight = 96;
final int cameraBytesPerPixel = 1;
final int bytesPerFrame = cameraWidth * cameraHeight * cameraBytesPerPixel;

PImage myImage;
byte[] frameBuffer = new byte[bytesPerFrame];
byte[] header = new byte[3];
byte[] score = new byte[2];

void setup()
{

  size(320, 320);

  // if you have only ONE serial port active
  //myPort = new Serial(this, Serial.list()[0], 9600);          // if you have only ONE serial port active

  // if you know the serial port name
  myPort = new Serial(this, "COM11", 921600);                    // Windows
  //  myPort = new Serial(this, "/dev/ttyUSB0", 921600);            // Linux
  // myPort = new Serial(this, "/dev/cu.usbmodem14401", 9600);     // Mac

  // wait for full frame of bytes
  myPort.buffer(bytesPerFrame);  
  myImage = createImage(cameraWidth, cameraHeight, GRAY);
  
  fill(255, 0, 0);
}

void draw()
{
  image(myImage, 0, 0, 320, 320);
}
int state = 0;
int read = 0;
int result = 0;
int startbyte;
void serialEvent(Serial myPort) {
  if (read == 0) {
    startbyte = myPort.read();
    if (startbyte == 0x55) {
      state = 1;
    }
    if (startbyte == 0xAA && state == 1) {
      read = 1; 
    }
    if (startbyte == 0xBB && state == 1) {
      result = 1; 
    }
  }
  if (result == 1) {
     myPort.readBytes(score);
     result = 0;
  }
  if (read ==1) {
    // read the saw bytes in
    myPort.readBytes(frameBuffer);
    // access raw bytes via byte buffer
    ByteBuffer bb = ByteBuffer.wrap(frameBuffer);
    bb.order(ByteOrder.BIG_ENDIAN);
    int i = 0;
    while (bb.hasRemaining()) {
      
      // read 16-bit pixel
      short p = bb.getShort();
      int p1 = (p>>8)&0xFF;
      int p2 = p&0xFF;
      // convert RGB565 to RGB 24-bit
      int r = p1;//((p >> 11) & 0x1f) << 3;
      int g = p1;//((p >> 5) & 0x3f) << 2;
      int b = p1;//((p >> 0) & 0x1f) << 3;

      // set pixel color
      myImage .pixels[i++] = color(r, g, b);
      r = p2;//((p >> 11) & 0x1f) << 3;
      g = p2;//((p >> 5) & 0x3f) << 2;
      b = p2;//((p >> 0) & 0x1f) << 3;

      // set pixel color
      myImage .pixels[i++] = color(r, g, b);
    }
    read = 0;
  }
  myImage .updatePixels();
}
