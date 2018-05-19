
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#include "libBMP180.h"

#include "i2c_master.h"

//#define BMP180_DEBUG

//if memory becomes a problem later, burst read these on demand.
//given the time taken for the calcs, this will barely be noticeable....
static int16_t ac1;
static int16_t ac2;
static int16_t ac3;
static uint16_t ac4;
static uint16_t ac5;
static uint16_t ac6;
static int16_t b1;
static int16_t b2;
static int16_t mb;
static int16_t mc;
static int16_t md;

static uint8_t oss = 0;

uint8_t bmp180_get_cal_param(uint8_t addr){

    #ifdef BMP180_DEBUG //use the datasheet cal vals
    ac1 = 408;
    ac2 = -72;
    ac3 = -14383;
    ac4 = 32741;
    ac5 = 32757;
    ac6 = 23153;
    b1 = 6190;
    b2 = 4;
    mb = -32768;
    mc = -8711;
    md = 2868;

    #else
    //use d to read the value, then cast to the right type.
    uint16_t d;
    i2c_readReg16(addr, BMP180_CAL_AC1, &d);    ac1 = (int16_t)d;
    i2c_readReg16(addr, BMP180_CAL_AC2, &d);    ac2 = (int16_t)d;
    i2c_readReg16(addr, BMP180_CAL_AC3, &d);    ac3 = (int16_t)d;
    i2c_readReg16(addr, BMP180_CAL_AC4, &ac4);
    i2c_readReg16(addr, BMP180_CAL_AC5, &ac5);
    i2c_readReg16(addr, BMP180_CAL_AC6, &ac6);
    i2c_readReg16(addr, BMP180_CAL_B1, &d);     b1 = (int16_t)d;
    i2c_readReg16(addr, BMP180_CAL_B2, &d);     b2 = (int16_t)d;
    i2c_readReg16(addr, BMP180_CAL_MB, &d);     mb = (int16_t)d;
    i2c_readReg16(addr, BMP180_CAL_MC, &d);     mc = (int16_t)d;
    i2c_readReg16(addr, BMP180_CAL_MD, &d);     md = (int16_t)d;

    #endif //BMP180_DEBUG

    return 0;
}

uint8_t bmp180_raw_temperature(uint8_t addr, uint16_t *ut){
    i2c_writeReg8(addr, 0xF4, 0x2E);
    _delay_ms(5);
    i2c_readReg16(addr, 0xF6, ut);

    return 0;
}


//public function to get calibrated temperature data
// addr is the 8-bit address of the bmp you want to interface
//result is a pointer to a float to store the result temperature
//return is 0 on success, greater than 0 on failure.
uint8_t bmp180_get_temperature(uint8_t addr, int16_t *temperature){
    //datasheet advises to use an int32_t, but thats just massive....
    //do the mem-expensive 32bit int calcs, then cast down to a 16bit..
    uint16_t ut = 0;
    bmp180_raw_temperature(addr, &ut);
    
    int32_t x1 = ((int32_t)ut - (int32_t)ac6) * ((int32_t)ac5) >>15;
    int32_t x2 = ((int32_t)mc << 11) / (x1 + (int32_t)md);
    int32_t b5 = x1 + x2;
    *temperature = (int16_t)((b5 + 8) >> 4);

    return 0;
}

//private function to get the uncalibrated pressure data from the sensor
uint8_t bmp180_raw_pressure(uint8_t addr, uint32_t *up){
    i2c_write8(addr, BMP180_REG_CONTROL, BMP180_MEASURE_PRESSURE + (oss<<6));
    _delay_ms(5);
    uint16_t d;
    i2c_read16(addr, BMP180_REG_PRESSURE, &d);
    *up = d;
    
    uint8_t xlsb;
    i2c_read8(addr, BMP180_REG_PRESSURE + 2, &xlsb);
    *up <<= 8;
    *up |= xlsb;
    *up >>= (8 - oss);
    return 0;
}

//public function to get the calibrated pressure
uint8_t bmp180_get_pressure(uint8_t addr, int32_t *pressure){
    uint16_t ut_temp = 0;
    bmp180_raw_temperature(addr, &ut_temp);
    int32_t ut = (int32_t)ut_temp;
    
    uint32_t up = 0;
    bmp180_raw_pressure(addr, &up);

    int32_t x1, x2, b5, b6, x3, b3, p;
    uint32_t b4, b7;

    #ifdef BMP180_DEBUG
    ut = 27898;
    up = 23843;
    ac6 = 23153;
    ac5 = 32757;
    mc = -8711;
    md = 2868;
    b1 = 6190;
    b2 = 4;
    ac3 = -14383;
    ac2 = -72;
    ac1 = 408;
    ac4 = 32741;
    oss = 0;
    #endif //BMP180_DEBUG

    x1 = (ut - (int32_t)ac6) * ((int32_t)ac5) >>15;
    x2 = ((int32_t)mc << 11) / (x1 + (int32_t)md);
    b5 = x1 + x2;

    b6 = b5 - 4000;
    x1 = ((int32_t)b2 * ((b6 * b6) >> 12)) >> 11;
    x2 = ((int32_t)ac2 * b6) >> 11;
    x3 = x1 + x2;
    b3 = ((((int32_t)ac1*4+x3) << oss) +2) / 4;

    x1 = ((int32_t)ac3*b6) >> 13;
    x2 = ((int32_t)b1 * ((b6*b6) >> 12)) >> 16;
    x3 = ((x1 + x2) + 2) >> 2;

    b4 = ((uint32_t)ac4 * (uint32_t)(x3 + 32768)) >> 15;
    b7 = ((uint32_t)up - b3) * (uint32_t)(50000UL >> oss);

    if(b7 < 0x80000000){
        p = (b7 * 2) / b4;
    } else {
        p = (b7 / b4) * 2;
    }
    x1 = (p >> 8) * (p >> 8);
    x1 = (x1 * 3038) >> 16;
    x2 = (-7357 * (p)) >> 16;

    p = p + ((x1 + x2 + (int32_t)3791) >> 4);

    *pressure = p;

    return 0;
}