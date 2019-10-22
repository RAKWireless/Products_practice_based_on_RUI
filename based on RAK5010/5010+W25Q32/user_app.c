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
    lis3dh_sleep_init();
    sensorOpt3001Enable(0);
    SHTC3_Sleep();
    lps22hb_mode(0);
}


/*This demo shows how to connect to external spi flash on 5010.
W25Q32 4M flash with spi. Pin map is below and VREF on 5010 must
be powered by external 3.3V.

   W25Q32       5010
   GND --------- GND
   VCC --------- VBAT
   CS  --------- IO4 P1.01  //33
   DO  --------- IO3 P1.02  //34
   DI  --------- IO2 P0.20
   CLK --------- IO1 P0.19
*/
RUI_SPI_ST w_st = {0};

void spi_init()
{
    w_st.PIN_CS = 33;
    w_st.PIN_MISO = 34;
    w_st.PIN_MOSI = 20;
    w_st.PIN_SCL = 19;
    rui_spi_init(&w_st);
}

void W25Q32_get_id()
{
    uint8_t cmd[6] = {0x90,0x00,0x00,0x00,0xFF,0xFF};
    uint8_t rsp[6] = {0};
    rui_spi_rw(RUI_IF_READ,cmd,6,rsp,6);
    NRF_LOG_INFO("%X%X",rsp[4],rsp[5]);
}

void main(void)
{
    //system init 
    rui_sensor_register_callback(sensor_on,sensor_off);
    rui_init();
    spi_init();
    while(1)
    {
        //do your work here, then call rui_device_sleep(1) to sleep
        W25Q32_get_id();
        //here run system work and do not modify
        rui_running();
    }
}