#include "rui.h"
#include "ds3231.h"


RUI_TIMER_ST rui_timer;
uint8_t timer_flag=0;
void timer_callback(void)
{
    timer_flag = 1;
}

void timer_init()
{
    rui_timer.timer_mode = RUI_TIMER_MODE_REPEATED;
    rui_timer_init(&rui_timer, timer_callback);
    rui_timer_setvalue(&rui_timer, 1000);
    rui_timer_start(&rui_timer);
}


RUI_I2C_ST rui_i2c_ds3231;
void i2c_init(void)
{
    rui_i2c_ds3231.PIN_SDA = 13;
    rui_i2c_ds3231.PIN_SCL = 12;
    rui_i2c_ds3231.FREQUENCY = RUI_I2C_FREQ_100K;
    rui_i2c_init(&rui_i2c_ds3231);
}

void i2c_running(void)
{
    tm tm_st;
    
    if (!timer_flag)
        return ;
    timer_flag = 0;

    ds3231_get_time(&tm_st);
    ds3231_rtc_print(tm_st);
}


/*  the function will run before sleep, 
    user can add code to make sensor into low power mode */
void user_sensor_sleep(void)
{
    // ...
}

/*  the function will run after wake up, 
    user can add code to wake up and init sensor module. */
void user_sensor_wakeup(void)
{
    // ...
}


void main(void)
{
    //system init 
    rui_sensor_register_callback(user_sensor_wakeup, user_sensor_sleep);
    rui_init();

    //you can add your init code here, like timer, uart, spi...
    timer_init();
    i2c_init();

    while(1)
    {
        //do your work here, then call rui_device_sleep(1) to sleep
        i2c_running();

        //here run system work and do not modify
        rui_running();
    }
}

