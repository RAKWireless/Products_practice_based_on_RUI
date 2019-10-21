#include "board_basic.h"
#include "boards.h"
#include <string.h>
#include "rui.h"

//the task is the main period task , user can set on/off and period via at cmd, do not modify.


//here will excute the period task 
void app_task(void * p_context)
{
	RUI_LOG_PRINTF("app_task!!!");

    //sensors init
    rui_device_sleep(0);

    rui_delay_ms(1000);
    
    rui_device_sleep(1);
}