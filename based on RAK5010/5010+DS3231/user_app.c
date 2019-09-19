#include "rui.h"

/*This demo shows how to connect to external I2C RTC on 5010.
DS3231 RTC with I2C. Pin map is below and VREF on 5010 must
be powered by external 3.3V.

   W25Q32		5010
   GND --------- GND
   VCC --------- VBAT
   SCL --------- IO2 P0.20
   SDA --------- IO1 P0.19
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
    st.PIN_SDA = 19;
    st.PIN_SCL = 20;
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