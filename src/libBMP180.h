
/* USAGE
//assuming device address is 0x77 (default)
bmp180_init(0x77);
bmp180_get_temperature(0x77, &temperature_variable);
*/

#ifndef _LIB_BMP180_H
#define _LIB_BMP180_H

#include <stdint.h>

//some useful registers
#define BMP180_REG_CONTROL 0xF4
#define BMP180_REG_TEMPERATURE 0xF6
#define BMP180_REG_PRESSURE 0xF6 

//COMMANDS to send to CONTROL
#define BMP180_MEASURE_TEMPERATURE 0x2E
#define BMP180_MEASURE_PRESSURE 0x34

#define BMP180_CAL_AC1 0xAA //signed 16bit
#define BMP180_CAL_AC2 0xAC //^
#define BMP180_CAL_AC3 0xAE //^^
#define BMP180_CAL_AC4 0xB0 //unsigned 16bit
#define BMP180_CAL_AC5 0xB2
#define BMP180_CAL_AC6 0xB4
#define BMP180_CAL_B1 0xB6  //signed 16bit
#define BMP180_CAL_B2 0xB8
#define BMP180_CAL_MB 0xBA
#define BMP180_CAL_MC 0xBC
#define BMP180_CAL_MD 0xBE

uint8_t bmp180_get_cal_param(uint8_t addr);
#define bmp180_init bmp180_get_cal_param //convenience wrapper

uint8_t bmp180_raw_temperature(uint8_t addr, uint16_t *result);
//public function to get the calibrated pressure
uint8_t bmp180_get_temperature(uint8_t addr, int16_t *temperature);

uint8_t bmp180_raw_pressure(uint8_t addr, uint32_t *up);
//public function to get the calibrated pressure
uint8_t bmp180_get_pressure(uint8_t addr, int32_t *pressure);

#endif //_LIB_BMP180_H