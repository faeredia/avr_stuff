#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdint.h>

#include "src/uart.h"
#include "src/i2c_master.h"
#include "src/libBMP180.h"

#define BMP180_ADDR 0x77

int main (void)
{
  //enable uart comms.
  uart_init(19200);
  //printf("UART up!\n");

  i2c_init();
  //printf("I2C up!\n");

  //bmp180_get_cal_param(BMP180_ADDR);
  bmp180_init(BMP180_ADDR);

  int16_t temperature =0;
  int32_t pressure =0;
  //bmp180_get_temperature(BMP180_ADDR, &temperature);

  //printf("BMP180 says: %d.%d\n", (int16_t)(temperature/10), (int16_t)(temperature % 10));
 
  printf("Sensor,Value,Units\n");
  while(1){
    temperature = bmp180_get_temperature(BMP180_ADDR);
    printf("BMP180_Temperature,%d.%d,c\n", (int16_t)(temperature/10), (int16_t)(temperature % 10));
    pressure = bmp180_get_pressure(BMP180_ADDR);
    printf("BMP180_Pressure,%d.%d,kPa\n", (uint16_t)(pressure/1000), (uint16_t)(pressure % 1000)) ;
    _delay_ms(5*1000);
  }

  return 0;
}