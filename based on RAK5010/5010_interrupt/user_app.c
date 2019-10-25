#include "rui.h"
#include "lis3dh.h"
#include "opt3001.h"
#include "shtc3.h"
#include "lps22hb.h"

RUI_I2C_ST st = {0};


void sensor_on(void)
{

    st.PIN_SDA = 14;
    st.PIN_SCL = 13;
    st.FREQUENCY = RUI_I2C_FREQ_400K;
    rui_i2c_init(&st);

    //lis3dh init
    lis3dh_init();
    //opt3001 init
    opt3001_init();
	//shtc3 init
    SHTC3_Wakeup();
    //lps22hb init 1 wake up
    lps22hb_mode(1);

}

void sensor_off(void)
{
    //lis3dh_sleep_init();
    sensorOpt3001Enable(0);
    SHTC3_Sleep();
    lps22hb_mode(0);
}


RUI_GPIO_ST lis3dh_st;
uint8_t wake_flag = 0;

void test_lis3dh()
{
    RUI_LOG_PRINTF("enter!!!");
    if (wake_flag == 0)
    {
        RUI_LOG_PRINTF("sleep!!!");
        rui_device_sleep(1);
        wake_flag = 1;
    }
    else
    {
        RUI_LOG_PRINTF("wake!!!");
        rui_device_sleep(0);
        wake_flag = 0;
    }

}

void main(void)
{
    //system init 
    lis3dh_st.pin_num = 16;
    lis3dh_st.dir = RUI_GPIO_PIN_DIR_INPUT;
    lis3dh_st.pull = RUI_GPIO_PIN_PULLDOWN;
    rui_gpio_interrupt(true, lis3dh_st, RUI_GPIO_EDGE_RAISE, RUI_GPIO_IRQ_HIGH_PRIORITY,test_lis3dh);

    rui_sensor_register_callback(sensor_on,sensor_off);

    rui_init();
    while(1)
    {
        //do your work here, then call rui_device_sleep(1) to sleep

        //here run system work and do not modify
        rui_running();
    }
}