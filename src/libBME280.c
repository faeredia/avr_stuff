
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
    int32_t     ut; //uncompensated-temperature
    uint32_t    uh; //uncompensated-humidity
    int32_t     up; //uncompensated pressure
    int32_t     t_fine; //updated when we calibrate temperature. used to clibrate pressure
} BME280_UNCAL_DATA;

//initialise an empty uncalibrated data struct
//this is the preferred way of creating a new UNCAL_DATA struct
const BME280_UNCAL_DATA BME280_UNCAL_DATA_INIT = {
    0,
    0,
    0,
    0
};

static int32_t bme280_compensated_temperature(BME280_UNCAL_DATA *ud);
static uint32_t bme280_compensated_humidity(BME280_UNCAL_DATA *ud);

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
}


//just a wrapper for the bme280_get_thp_bystruct if the user doesnt want to make a BME280_DATA
void bme280_get_thp_bypointer(uint8_t devaddr, int32_t *t, uint32_t *h, int32_t *p){
    BME280_DATA d;
    d = bme280_get_thp_bystruct(devaddr);
    *t = d.temperature;
    *h = d.humidity;
    *p = d.pressure;
}

//receive a pointer to the uncalibrated data.
//we will set t_fine while we are here 
//therefore, this ***MUST** be called before trying to calibrated pressure
static int32_t bme280_compensated_temperature(BME280_UNCAL_DATA *ud){
    int32_t var1, var2;

    var1 = ((((ud->ut>>3) - ((int32_t)_cal_t1<<1))) * ((int32_t)_cal_t2)) >>11;
    var2 = (((((ud->ut>>4) - ((int32_t)_cal_t1)) * ((ud->ut>>4) - ((int32_t)_cal_t1))) >> 12) * ((int32_t)_cal_t3)) >> 14;
    ud->t_fine = var1 + var2;

    return (ud->t_fine * 5 + 128) >> 8;
}


static uint32_t bme280_compensated_humidity(BME280_UNCAL_DATA *ud){
    if(ud->t_fine == 0){
        //t_fine has not been set, do temperature compensation routine
        bme280_compensated_temperature(ud);
    }
    

}