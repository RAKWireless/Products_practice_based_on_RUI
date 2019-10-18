#include "board_basic.h"
#include "boards.h"
#include "nrf_delay.h"
#include "nrf_rtc.h"
#include <string.h>
#include "rui.h"
#include "nrf_log.h"

//the task is the main period task for cellular, user can set on/off and period via at cmd
//do not modify

int RUI_CALLBACK_REGE_FLAG = 0;

//lora recieve callback
void LoRaReceive_callback(RUI_RECEIVE_T* Receive_datapackage)
{
    //user define
}

//here will excute the period task to send data to user's server
void app_task(void * p_context)
{
    uint8_t send_data[256] = {0};

    RUI_LOG_PRINTF("app_task!!!");

    if (RUI_CALLBACK_REGE_FLAG == 0)
    {
        rui_lora_register_recv_callback(LoRaReceive_callback);
        RUI_CALLBACK_REGE_FLAG = 1;
    }

    // sensors init
    rui_device_sleep(0);

    // send test data
    memset(send_data,0x41,50);
    rui_lora_send(1,send_data,50);

    // system run to sleep
    rui_device_sleep(1);
}