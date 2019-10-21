#include "rui.h"



void user_sensor_wakeup(void)
{
    // ...
}

void user_sensor_sleep(void)
{
    // ...
}

void main(void)
{
    //system init 
    rui_sensor_register_callback(user_sensor_wakeup, user_sensor_sleep);
    rui_init();

    while(1)
    {
        //do your work here, then call rui_device_sleep(1) to sleep

        //here run system work and do not modify
        rui_running();
    }
}
