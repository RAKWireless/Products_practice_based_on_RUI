#include "rui.h"

/*This demo shows how to connect to external spi flash on 5010.
W25Q32 4M flash with spi. Pin map is below and VREF on 5010 must
be powered by external 3.3V.

   W25Q32		5010
   GND --------- GND
   VCC --------- VBAT
   CS  --------- IO4 P1.01
   DO  --------- IO3 P1.02
   DI  --------- IO2 P0.20
   CLK --------- IO1 P0.19
*/

RUI_SPI_ST st = {0};

void main(void)
{
    //system init 
    rui_init();

    //W25Q32 init
    rui_device_sleep(1);
    uint8_t cmd[6] = {0x90,0x00,0x00,0x00,0xFF,0xFF};
    uint8_t rsp[6] = {0};
    st.PIN_CS = 33;
    st.PIN_MISO = 34;
    st.PIN_MOSI = 20;
    st.PIN_SCL = 19;
    rui_spi_init(&st);
    while(1)
    {
        //do your work here, then call rui_device_sleep(1) to sleep
    	//read flash ID and then sleep
    	rui_spi_rw(cmd,6,rsp,6);
    	NRF_LOG_INFO("%X%X",rsp[4],rsp[5]);
        rui_device_sleep(1);
        //here run system work and do not modify
        rui_running();
    }
}