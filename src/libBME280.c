
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#include "libBME280.h"
#include "i2c_master.h"

/***************************
    Static definitions
****************************/
//Uncalibrated Data Struct
typedef struct {
    uint32_t    ut; //uncompensated-temperature
    uint32_t    uh; //uncompensated-humidity
    uint32_t    up; //uncompensated pressure
    int32_t     t_fine; //updated when we calibrate temperature. used to clibrate pressure
} BME280_UNCAL_DATA;

//initialise an empty uncalibrated data struct
//this is the preferred way of creating a new UNCAL_DATA struct
const BME280_UNCAL_DATA BME280_UNCAL_DATA_INIT = {0};


static void bme280_read_calibration_terms(uint8_t devaddr);
static BME280_UNCAL_DATA bme280_get_uncompensated_data(uint8_t devaddr);
static BME280_UNCAL_DATA bme280_get_compensated_data(BME280_UNCAL_DATA *ud);
static int32_t bme280_calc_t_fine(BME280_UNCAL_DATA *ud);
static int32_t bme280_compensated_temperature(BME280_UNCAL_DATA *ud);
static uint32_t bme280_compensated_humidity(BME280_UNCAL_DATA *ud);
static uint32_t bme280_compensated_pressure(BME280_UNCAL_DATA *ud);

//register addresses
#define BME280_REG_CAL1         0x88
#define BME280_REG_CAL1_LEN     25
#define BME280_REG_CAL2         0xE1
#define BME280_REG_CAL2_LEN     8
#define BME280_REG_CTRL         0xF4
#define BME280_REG_DATA         0xF7
#define BME280_REG_DATA_LEN     8

//calibration terms
static uint16_t     _cal_t1;
static int16_t      _cal_t2;
static int16_t      _cal_t3;
static uint16_t     _cal_p1;
static int16_t      _cal_p2;
static int16_t      _cal_p3;
static int16_t      _cal_p4;
static int16_t      _cal_p5;
static int16_t      _cal_p6;
static int16_t      _cal_p7;
static int16_t      _cal_p8;
static int16_t      _cal_p9;
static uint8_t      _cal_h1;
static int16_t      _cal_h2;
static uint8_t      _cal_h3;
static int16_t      _cal_h4;
static int16_t      _cal_h5;
static int8_t       _cal_h6;

/***************************
    Useful stuff
****************************/

void bme280_init(uint8_t devaddr){
    //fetch calibration terms
    bme280_read_calibration_terms(devaddr);
}

//might want to call this again later...
void bme280_read_calibration_terms(uint8_t devaddr){
    uint8_t cal1[BME280_REG_CAL1_LEN];
    uint8_t cal2[BME280_REG_CAL2_LEN];

    uint8_t i2c_readReg(devaddr, BME280_REG_CAL1, &cal1, BME280_REG_CAL1_LEN);
    uint8_t i2c_readReg(devaddr, BME280_REG_CAL2, &cal2, BME280_REG_CAL2_LEN);

    //cal1
    _cal_t1 = (uint16_t)(cal1[0]) << 8 | cal1[1];
    _cal_t2 = (int16_t) (cal1[2]) << 8 | cal1[3];
    _cal_t3 = (int16_t) (cal1[4]) << 8 | cal1[5];
    _cal_p1 = (uint16_t)(cal1[6]) << 8 | cal1[7];
    _cal_p2 = (int16_t) (cal1[8]) << 8 | cal1[9];
    _cal_p3 = (int16_t)(cal1[10]) << 8 | cal1[11];
    _cal_p4 = (int16_t)(cal1[12]) << 8 | cal1[13];
    _cal_p5 = (int16_t)(cal1[14]) << 8 | cal1[15];
    _cal_p6 = (int16_t)(cal1[16]) << 8 | cal1[17];
    _cal_p7 = (int16_t)(cal1[18]) << 8 | cal1[19];
    _cal_p8 = (int16_t)(cal1[20]) << 8 | cal1[21];
    _cal_p9 = (int16_t)(cal1[22]) << 8 | cal1[23];
    _cal_h1 = (uint8_t)(cal1[24]);

    //cal2
    _cal_h2 = (int16_t) (cal2[0]) << 8 | cal2[1];
    _cal_h3 = (uint8_t) (cal2[2]);
  //_cal_h4 = (int16_t) (cal2[3]) << 8 | cal2[4]; //D-S: dig_H4 [11:4]/[3:0]?
    _cal_h4 = (int16_t) (cal2[3]) << 4 | cal2[4]; //make 4bits of space for the lsb, only keep 4 smallest bits of lsb.
  //_cal_h5 = (int16_t) (cal2[5]) << 8 | cal2[6]; //D-S: dig_H5 [3:0]/[11:4]?
    _cal_h5 = (int16_t) (cal2[5]) << 4 | cal2[6]; //make 8 bits of space for lsb, only keep 4 smallest bits of msb
    _cal_h6 = (int8_t)  (cal2[7]);

}

//just a wrapper for the bme280_get_thp_bystruct if the user doesnt want to make a BME280_DATA
void bme280_get_thp_bypointer(uint8_t devaddr, int32_t *t, uint32_t *h, int32_t *p){
    BME280_DATA d;
    d = bme280_get_thp_bystruct(devaddr);
    *t = d.temperature;
    *h = d.humidity;
    *p = d.pressure;
}

BME280_DATA bme280_get_thp_bystruct(uint8_t devaddr){
    BME280_UNCAL_DATA_INIT ud;
    ud = bme280_get_uncompensated_data(devaddr);

    BME280_DATA_INIT d;
    d =  bme280_get_compensated_data(&ud);
    return d;
}

BME280_UNCAL_DATA bme280_get_uncompensated_data(uint8_t devaddr){
    //write to the control register to take a reading
    uint8_t rd[8];
    BME280_UNCAL_DATA_INIT ud;
    uint32_t msb, lsb, xlsb;

    // 0b 001    001     01
    //   p_oss  t_oss  measure
    msb = 0x01 << 5;
    lsb = 0x01 << 2;
    xlsb = 0x01;
    i2c_write8(devaddr, BME280_REG_CTRL, (uint8_t)(msb | lsb | xlsb));

    i2c_readREG(devaddr, BME280_REG_DATA, &rd, BME280_REG_DATA_LEN);

    msb = (uint32_t)rd[0] << 12;
    lsb = (uint32_t)rd[1] << 4;
    xlsb = (uint32_t)rd[2] >> 4;
    ud.pressure = msb | lsb | xlsb;

    msb = (uint32_t)rd[3] << 12;
    lsb = (uint32_t)rd[4] << 4;
    xlsb = (uint32_t)rd[5] >> 4;
    ud.temperature = msb | lsb | xlsb;

    msb = (uint32_t)rd[6] << 8;
    lsb = (uint32_t)rd[7];
    ud.humidity = msb | lsb;
    
    return ud;
}

BME280_DATA bme280_get_compensated_data(BME280_UNCAL_DATA *ud){
    //set t_fine before doing other calccs.
    BME280_DATA_INIT d;
    ud->t_fine = bme280_calc_t_fine(ud);
    d.temperature = bme280_compensated_temperature(ud);
    d.humidity = bme280_compensated_humidity(ud);
    d.pressure = bme280_compensated_pressure(ud);
}

/**************************************************
 * Below functions are really gross.
 * Direct from the datasheet.
 * Assume they work and don't try deciphering them.
 *************************************************/

int32_t bme280_calc_t_fine(BME280_UNCAL_DATA *ud){
    //should this return t_fine, or set it directly in the struct pointer??????
    //this way is more verbose....
    int32_t var1, var2;
    var1 = ((((ud->ut>>3) - ((int32_t)_cal_t1<<1))) * ((int32_t)_cal_t2)) >>11;
    var2 = (((((ud->ut>>4) - ((int32_t)_cal_t1)) * ((ud->ut>>4) - ((int32_t)_cal_t1))) >> 12) * ((int32_t)_cal_t3)) >> 14;
    return = var1 + var2;
}

int32_t bme280_compensated_temperature(BME280_UNCAL_DATA *ud){
    if(ud->t_fine == 0){
        bme280_calc_t_fine(ud);
    }
    //we assume t_fine is set now. danger, danger if its not
    return (ud->t_fine * 5 + 128) >> 8;
}

uint32_t bme280_compensated_humidity(BME280_UNCAL_DATA *ud){
    if(ud->t_fine == 0){
        bme280_calc_t_fine(ud);
    }
    //we assume t_fine is set now. danger, danger if its not
    int32_t var1;

    var1 = (ud->t_fine - ((int32_t)76800));
    var1 = (((((ud->humidity << 14) - (((int32_t)_cal_h4) << 20) - (((int32_t)_cal_h5) * var1)) + 
        ((int32_t_16384)) >> 15) * ((((((var1 * ((int32_t)_cal_h6)) >> 10) + (((var1 *
        ((int32_t)_cal_h3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) * 
        ((int32_t)_cal_h2) + 8192) >> 14));
    var1 = (var1 - (((((var1 >> 15) * (var1 >> 15)) >> 7) * ((int32_t)_cal_h1)) >> 4));
    var1 = (var1 < 0 ? 0 : var1);
    var1 = (var1 > 419430400 ? 419430400 : var);
    return (uint32_t)(var1 >> 12);
}

uint32_t bme280_compensated_pressure(BME280_UNCAL_DATA *ud){
    if(ud->t_fine == 0){
        bme280_calc_t_fine(ud);
    }    
    int32_t var1, var2;
    uint32_t p;

    var1 = (((int32_t)ud->t_fine) >> 1) - (int32_t)64000;
    var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * ((int32_t)_cal_p6);
    var2 = var2 + ((var1*((int32_t)_cal_p5)) << 1);
    var2 = (var2 >> 2) + (((int32_t)_cal_p4) << 16);
    var1 = (((_cal_p3 * (((var1 >> 2) * (var1 >> 2)) >> 13)) >> 3) + ((((int32_t)_cal_p2) * var1) >> 1 )) >> 18;
    var1 = ((((32768 + var1)) * ((int32_t)_cal_p1)) >> 15);
    if(var1 == 0) return 0; //avoid divide by 0
    p = (((uint32_t)(((int32_t)1048576) - ud->pressure) - (var2>>12))) * 3125;
    if (p < 0x80000000){
        p = (p << 1) / ((uint32_t)var1);
    } else {
        p = (p / (uint32_t)var1) * 2;
    }
    var1 = (((int32_t)_cal_p9) * ((int32_t)(((p>>3) * (p>>3)) >> 13))) >> 12;
    var2 = (((int32_t)(p >> 2)) * ((int32_t)_cal_p8)) >> 13;
    p = (uint32_t)((int32_t)p + ((var1 + var2 + _cal_p7) >> 4));
    return p;
}