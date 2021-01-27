
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/spi.h"
#include "pico/binary_info.h"
#include "arducam.h"
#include "ov2640.h"



// Define sensor slave address
static int slave_addr = 0x30;


//int i2c_write_blocking(i2c_inst_t *i2c, uint8_t addr, const uint8_t *src, size_t len, bool nostop);

int rdSensorReg8_8(uint8_t regID, uint8_t* regDat ){
    i2c_write_blocking(I2C_PORT, slave_addr, &regID, 1, true );
    i2c_read_blocking(I2C_PORT, slave_addr, regDat,  1, false );
}
 int wrSensorReg8_8(uint8_t regID, uint8_t regDat ){
    uint8_t buf[2];
    buf[0] = regID;
    buf[1] = regDat;
    i2c_write_blocking(I2C_PORT, slave_addr, buf,  2, true );
}

 void cs_select() {
    asm volatile("nop \n nop \n nop");
    gpio_put(PIN_CS, 0);  // Active low
    asm volatile("nop \n nop \n nop");
}

 void cs_deselect() {
    asm volatile("nop \n nop \n nop");
    gpio_put(PIN_CS, 1);
    asm volatile("nop \n nop \n nop");
}

 void write_reg(uint8_t address, uint8_t value){
    uint8_t buf[2];
    buf[0] = address|WRITE_BIT ;  // remove read bit as this is a write
    buf[1] = value;
    cs_select();
    spi_write_blocking(SPI_PORT, buf, 2);
    cs_deselect();
    sleep_ms(10); 
}

 uint8_t read_reg(uint8_t address)
{
    uint8_t value = 0;
	address = address& 0x7f;
     cs_select();
	spi_write_blocking(SPI_PORT, &address, 1);
	sleep_ms(10);
    spi_read_blocking(SPI_PORT, 0, &value, 1);
	cs_deselect();
    sleep_ms(10);
	return value;
}

int wrSensorRegs8_8(const struct sensor_reg reglist[])
{
  int err = 0;
  unsigned int reg_addr = 0;
  unsigned int reg_val = 0;
  const struct sensor_reg *next = reglist;
  while ((reg_addr != 0xff) | (reg_val != 0xff))
  {
    reg_addr =next->reg;
    reg_val = next->val;
    err = wrSensorReg8_8(reg_addr, reg_val);
    sleep_ms(10);
    next++;
  }
  return err;
}

unsigned char read_fifo(void)
{
	unsigned char data;
	data = read_reg(SINGLE_FIFO_READ);
	return data;
}
void set_fifo_burst()
{
    uint8_t value;
    spi_read_blocking(SPI_PORT, BURST_FIFO_READ, &value, 1);
}


void flush_fifo(void)
{
	write_reg(ARDUCHIP_FIFO, FIFO_CLEAR_MASK);
}

void start_capture(void)
{
	write_reg(ARDUCHIP_FIFO, FIFO_START_MASK);
}

void clear_fifo_flag(void )
{
	write_reg(ARDUCHIP_FIFO, FIFO_CLEAR_MASK);
}

unsigned int read_fifo_length()
{
    unsigned int len1,len2,len3,len=0;
    len1 = read_reg(FIFO_SIZE1);
  len2 = read_reg(FIFO_SIZE2);
  len3 = read_reg(FIFO_SIZE3) & 0x7f;
  len = ((len3 << 16) | (len2 << 8) | len1) & 0x07fffff;
	return len;	
}

//Set corresponding bit  
void set_bit(unsigned char addr, unsigned char bit)
{
	unsigned char temp;
	temp = read_reg(addr);
	write_reg(addr, temp | bit);
}
//Clear corresponding bit 
void clear_bit(unsigned char addr, unsigned char bit)
{
	unsigned char temp;
	temp = read_reg(addr);
	write_reg(addr, temp & (~bit));
}

//Get corresponding bit status
unsigned char get_bit(unsigned char addr, unsigned char bit)
{
  unsigned char temp;
  temp = read_reg(addr);
  temp = temp & bit;
  return temp;
}

void OV2640_set_JPEG_size(unsigned char size)
{
	switch(size)
	{
		case OV2640_160x120:
			wrSensorRegs8_8(OV2640_160x120_JPEG);
			break;
		case OV2640_176x144:
			wrSensorRegs8_8(OV2640_176x144_JPEG);
			break;
		case OV2640_320x240:
			wrSensorRegs8_8(OV2640_320x240_JPEG);
			break;
		case OV2640_352x288:
	  	wrSensorRegs8_8(OV2640_352x288_JPEG);
			break;
		case OV2640_640x480:
			wrSensorRegs8_8(OV2640_640x480_JPEG);
			break;
		case OV2640_800x600:
			wrSensorRegs8_8(OV2640_800x600_JPEG);
			break;
		case OV2640_1024x768:
			wrSensorRegs8_8(OV2640_1024x768_JPEG);
			break;
		case OV2640_1280x1024:
			wrSensorRegs8_8(OV2640_1280x1024_JPEG);
			break;
		case OV2640_1600x1200:
			wrSensorRegs8_8(OV2640_1600x1200_JPEG);
			break;
		default:
			wrSensorRegs8_8(OV2640_320x240_JPEG);
			break;
	}
}

void ov2640Init(){
    wrSensorReg8_8(0xff, 0x01);
    wrSensorReg8_8(0x12, 0x80);
    wrSensorRegs8_8(OV2640_JPEG_INIT);
    wrSensorRegs8_8(OV2640_YUV422);
    wrSensorRegs8_8(OV2640_JPEG);
    wrSensorReg8_8(0xff, 0x01);
    wrSensorReg8_8(0x15, 0x00);
    wrSensorRegs8_8(OV2640_320x240_JPEG);
}

 void singleCapture(void){
   int i , count;
   uint8_t value[1024*10];
   //Flush the FIFO
   flush_fifo();
   //Start capture
   start_capture(); 
   while(!get_bit(ARDUCHIP_TRIG , CAP_DONE_MASK)){;
       //printf("wait\r\n");
    }
   int length = read_fifo_length();
  // printf("length: %d\r\n",length);
   count = length;
   i = 0 ;
   cs_select();
   set_fifo_burst();//Set fifo burst mode
  // while (count--) {
    spi_read_blocking(SPI_PORT, BURST_FIFO_READ,value, length);
     uart_write_blocking(UART_ID, value, length);
   	//}
	count = 0;
	 cs_deselect();
}
