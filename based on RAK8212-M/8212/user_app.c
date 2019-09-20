#include "rui.h"

void main(void)
{
    //system init 
    rui_init();
    //you can add your init code here, like timer, uart, spi...

    while(1)
    {
        //do your work here, then call rui_device_sleep(1) to sleep

        //here run system work and do not modify
        rui_running();
    }
}