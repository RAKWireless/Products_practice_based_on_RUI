#include "rui.h"

/*This demo shows how to connect to external I2C RTC on 8212
DS3231 RTC with I2C. Pin map is below.

   DS3231		8212
   GND --------- GND
   VCC --------- VDD
   SCL --------- IO2 P0.28
   SDA --------- IO1 P0.29
*/

#define ADDR  0x68 
#define s_addr 0x00 
RUI_I2C_ST st = {0};

void main(void)
{
    //system init 
    rui_init();

    //DS3231 init
    uint8_t rsp = 0;
    st.PIN_SDA = 29;
    st.PIN_SCL = 28;
    st.FREQUENCY = RUI_I2C_FREQ_400K;
    rui_i2c_init(&st);
    while(1)
    {
        //do your work here, then call rui_device_sleep(1) to sleep
    	//read second and then sleep
    	rui_i2c_rw(&st,RUI_IF_READ,ADDR,s_addr,&rsp,1);
    	NRF_LOG_INFO("%d",rsp);
    	rui_delay_ms(1000);
        rui_device_sleep(1);
        //here run system work and do not modify
        rui_running();
    }
}