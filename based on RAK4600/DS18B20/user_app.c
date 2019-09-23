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

void ds18b20_test(void)
{
    float temperature;
    
    if (!timer_flag)
        return ;
    timer_flag = 0;


    DS18B20_GetTemp(&temperature);  // Ten-fold increase in temperature
    RUI_LOG_PRINTF("Temperature is %d.", (uint32_t)(temperature/1));
}


void main(void)
{
    //system init 
    rui_init();

    //you can add your init code here, like timer, uart, spi...
    timer_init();
    
    while(1)
    {
        //do your work here, then call rui_device_sleep(1) to sleep
        ds18b20_test();

        //here run system work and do not modify
        rui_running();
    }
}
