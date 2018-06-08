#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdint.h>

#include "uart.h"
#include "i2c_master.h"
#include "libBMP180.h"
#include "libBME280.h"

#define BMP180_ADDR 0x77
#define BME280_ADDR 0x76

void print_bmp180(uint8_t devaddr){
  int16_t temperature = 0;
  int32_t pressure = 0;
  temperature = bmp180_get_temperature(BMP180_ADDR);
  pressure = bmp180_get_pressure(BMP180_ADDR);
  printf("BMP180,Temperature,%d.%d,c\n", (int16_t)(temperature/10), (int16_t)(temperature % 10));
  printf("BMP180,Pressure,%d.%d,kPa\n", (uint16_t)(pressure/1000), (uint16_t)(pressure % 1000));
}

void print_bme280(uint8_t devaddr){
  BME280_DATA bme_d = {0};
  bme_d = bme280_get_thp(devaddr);
  printf("BME280,Temperature,%d.%d,c\n", (int16_t)(bme_d.temperature/10), (int16_t)(bme_d.temperature % 10));
  printf("BME280,Pressure,%d.%d,kPa\n", (int16_t)(bme_d.pressure/10), (int16_t)(bme_d.pressure % 10));
  printf("BME280,Humidity,%d.%d,%%\n", (int16_t)(bme_d.humidity/10), (int16_t)(bme_d.humidity % 10));
}

int main (void)
{
  //bring up uart.
  uart_init(19200);
  i2c_init();
  
  //bmp180_get_cal_param(BMP180_ADDR);
  bmp180_init(BMP180_ADDR);
  bme280_init(BME280_ADDR);
  
  printf("Sensor,Type,Value,Units\n");
  while(1){
    print_bmp180(BMP180_ADDR);
    print_bme280(BME280_ADDR);
    _delay_ms(5*1000);
  }

  return 0;
}
