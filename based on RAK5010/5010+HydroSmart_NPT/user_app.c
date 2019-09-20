#include "rui.h"

extern RUI_I2C_ST rui_i2c_module;

/**         FED-XXXD-NPT                        */

#define NPT_ADDR            0x6D // 7bits addr

#define NPT_PRESSURE1       0x06
#define NPT_PRESSURE2       0x07
#define NPT_PRESSURE3       0x08

#define NPT_TEMPERATURE1    0x09
#define NPT_TEMPERATURE2    0x0A
#define NPT_TEMPERATURE3    0x0B

void npt_get_pressure(double *pressure)
{
    uint8_t buf[3];
	uint32_t dat=0;
	double fadc;

    rui_i2c_rw(&rui_i2c_module, RUI_IF_READ, NPT_ADDR, NPT_PRESSURE1, buf, sizeof(buf));

	dat =( buf[0]<<16 ) | (buf[1]<<8) | (buf[2]);
	if(dat & 0x800000)
    	fadc = dat-16777216.0;
	else
	    fadc = dat;

    double ADC_p ;	

    ADC_p = 3.3 * fadc/8388608.0 ;
    *pressure =  (200 * (ADC_p-0.5)/2.0)+0;
}

void npt_get_temperature(double *temperature)
{
    uint8_t buf[3];
	uint32_t dat=0;
	double fadc;

    rui_i2c_rw(&rui_i2c_module, RUI_IF_READ, NPT_ADDR, NPT_TEMPERATURE1, buf, sizeof(buf));

	dat =( buf[0]<<16 ) | (buf[1]<<8) | (buf[2]);
	if(dat & 0x800000)
    	fadc = dat-16777216.0;
	else
	    fadc = dat;

	*temperature= 25.0 + fadc/65536.0;
}

/************************************************/


RUI_TIMER_ST rui_timer;
uint8_t timer_flag=0;
void timer_callback(void)
{
    timer_flag = 1;
}

void timer_init()
{
    rui_timer.timer_mode = RUI_TIMER_MODE_REPEATED;
    rui_timer_init(&rui_timer, timer_callback);
    rui_timer_setvalue(&rui_timer, 1000);
    rui_timer_start(&rui_timer);
}


RUI_I2C_ST rui_i2c_module;
void module_init(void)
{
    rui_i2c_module.PIN_SDA = 19;
    rui_i2c_module.PIN_SCL = 20;
    rui_i2c_module.FREQUENCY = RUI_I2C_FREQ_400K;
    rui_i2c_init(&rui_i2c_module);
}

void module_running(void)
{
    double pressure,temperature;


    if (!timer_flag)
        return ;
    timer_flag = 0;

    npt_get_pressure(&pressure);
    RUI_LOG_PRINTF("Pressure is %d.", (uint32_t)(pressure*10));
    npt_get_temperature(&temperature);
    RUI_LOG_PRINTF("temperature is %d.", (uint32_t)(temperature*10));
}


void main(void)
{
    //system init 
    rui_init();

    //you can add your init code here, like timer, uart, spi...
    timer_init();
    module_init();

    while(1)
    {
        //do your work here, then call rui_device_sleep(1) to sleep
//        module_running();

        //here run system work and do not modify
        rui_running();
    }
}
