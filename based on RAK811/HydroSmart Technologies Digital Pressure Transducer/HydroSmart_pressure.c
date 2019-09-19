#include "rui.h"
#include "board.h"
RUI_I2C_ST rui_i2c;
void Pressure_init(void)
{	
	rui_i2c.INSTANCE_ID = 1;
	rui_i2c.FREQUENCY = RUI_I2C_FREQ_100K; 
	rui_i2c.PIN_SDA = I2C_SDA;
	rui_i2c.PIN_SCL = I2C_SCL;
	rui_i2c_init(&rui_i2c);
}
void Get_Pressure(double* P_PSI,double* Temperature)
{
	uint8_t dat_temp[3]={0};
	
	rui_i2c_rw(&rui_i2c,RUI_IF_READ,0xDA,0x06,dat_temp,3);
	unsigned long dat=0;
	double fadc;
	dat =( dat_temp[0]<<16 ) | (dat_temp[1]<<8) | (dat_temp[2]);
	if(dat & 0x800000)
		fadc = dat-16777216.0;
	else
		fadc = dat;
		
	double ADC_p ;	
	
	ADC_p = 3.3 * fadc/8388608.0 ;
	*P_PSI =  (200 * (ADC_p-0.5)/2.0)+0 ;
	RUI_LOG_PRINTF("\r\nPressure: %d.%02d PSI\r\n",(uint32_t)*P_PSI,(uint32_t)(*P_PSI*100-((int32_t)*P_PSI) * 100));
	
	rui_i2c_rw(&rui_i2c,RUI_IF_READ,0xDA,0x09,dat_temp,3);
	dat=0;
	dat =( dat_temp[0]<<16 ) | (dat_temp[1]<<8) | (dat_temp[2]);
	if(dat & 0x800000)
	fadc = dat-16777216.0;
	else
	fadc = dat;
	*Temperature= 25.0 + fadc/65536.0;
	RUI_LOG_PRINTF("\r\nTemperature: %d.%02d degree\r\n",(uint32_t)*Temperature,(uint32_t)(*Temperature*100-((int32_t)*Temperature) * 100));
}