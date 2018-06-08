/*library for interfacing with Bosch BME280 over i2c
Uses the i2c_master library
*/

#ifndef _LIB_BME280_H
#define _LIB_BME280_H

#include <stdint.h>

//little struct to easily pass BME280 data around
//investigate opportunities to reduce the variable size of each....
typedef struct {
    int32_t     temperature;
    uint32_t    humidity;
    int32_t     pressure;
} BME280_DATA;

//preferred type when creating a new BME280_DATA, init all values to 0
//const BME280_DATA BME280_DATA_INIT = {0};

//call after i2c_init()
//Called to initialise the sensor/loads calibration data into memory
void bme280_init(uint8_t devaddr);

//returns a BME280_DATA struct full of calibrated data.
BME280_DATA bme280_get_thp_bystruct(uint8_t devaddr);

//alternate way to fetch the data,
//pass in pointers to temperature humidity and pressure and the funciton will fill
void bme280_get_thp_bypointer(uint8_t devaddr, int32_t *t, uint32_t *h, int32_t *p);

//for user convenience. DONOT rely on this in libBME280.c
#define bme280_get_thp bme280_get_thp_bystruct
//#define bme280_get_thp bme280_get_thp_bypointer




#endif //_LIB_BME280_H
