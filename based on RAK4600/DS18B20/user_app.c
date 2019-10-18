#include "rui.h"



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


RUI_GPIO_ST rui_gpio_ds18b20;
void ds18b20_init(void)
{
    rui_gpio_ds18b20.pin_num = 13;
    rui_gpio_ds18b20.dir = RUI_GPIO_PIN_DIR_OUTPUT;
    rui_gpio_ds18b20.pull = RUI_GPIO_PIN_NOPULL;

    rui_gpio_init(&rui_gpio_ds18b20);
}

void ds18b20_running(void)
{
    float temperature;
    uint8_t msg[32]={0};
    
    if (!timer_flag)
        return ;
    timer_flag = 0;

    DS18B20_GetTemp(&temperature);  // Ten-fold increase in temperature
    sprintf(msg, "Temperature is %.1f.", temperature/10.0);
    RUI_LOG_PRINTF("%s", msg);
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
    ds18b20_init();
    timer_init();

    while(1)
    {
        //do your work here, then call rui_device_sleep(1) to sleep
        ds18b20_running();

        //here run system work and do not modify
        rui_running();
    }
}

