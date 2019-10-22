#include "rui.h"
#include "lis3dh.h"
#include "opt3001.h"
#include "lis2mdl.h"
#include "bme280.h"

//because of the I2C pin is different, init the bus is need before handling every sensor on 8212
void sensor_on(void)
{
    //lis3dh init
    lis3dh_twi_init();
    lis3dh_init();
    //opt3001 init 
    opt3001_twi_init();
    opt3001_init();
    //lis2mdl init
    lis2mdl_twi_init();
    lis2mdl_init();
    //bme280 init
    _bme280_init();
     ds3231_init();
}

void sensor_off(void)
{
    lis3dh_twi_init();
    lis3dh_sleep_init();
    opt3001_twi_init();
    sensorOpt3001Enable(0);
    lis2mdl_twi_init();
    lis2mdl_sleep_init();
    _bme280_sleep_init();
}

void main(void)
{
    //system init 
    rui_sensor_register_callback(sensor_on,sensor_off);
    rui_init();

    while(1)
    {
        //do your work here, then call rui_device_sleep(1) to sleep
        ds3231_get_time();
        ds3231_rtc_print();
        //here run system work and do not modify
        rui_running();
    }
}