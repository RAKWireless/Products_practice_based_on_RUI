#include "rui.h"


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
    rui_init();
    
    //you can add your init code here, like timer, uart, spi...
    rui_sensor_register_callback(user_sensor_wakeup, user_sensor_sleep);

    while(1)
    {
        //do your work here, then call rui_device_sleep(1) to sleep

        //here run system work and do not modify
        rui_running();
    }
}

