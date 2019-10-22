#include "rui.h"
#include "sensor.h"

bsp_sensor_data_t bsp_sensor;
/********************************************************************************************************
 * get Battery Volage
********************************************************************************************************/
// #define VREFINT_CAL       ( *( uint16_t* )0x1FF80078 )  //VREF calibration value
extern RUI_GPIO_ST Bat_level;
uint32_t BoardBatteryMeasureVolage( float *voltage )
{
    uint16_t vdiv = 0;
	rui_adc_get(&Bat_level,&vdiv);
    *voltage = (5 * vdiv )/3;
}
/********************************************************************************************************
 * get BME680 sensor data
********************************************************************************************************/
int BME680_get_data(uint32_t *humidity,int16_t* temp,uint32_t* pressure,uint32_t * resis)
{
		#include "BME680.h"
		if(BME680_read(temp, pressure, humidity, resis) == 0)
		{		
			RUI_LOG_PRINTF("Humidity:%d.%d %%RH\r\n",(int32_t)(*humidity/1000),(int32_t)(*humidity%1000));		
			RUI_LOG_PRINTF("Temperature:%d.%d degree\r\n",(int32_t)(*temp/100),(int32_t)(*temp%100));	
			RUI_LOG_PRINTF("Pressure:%d.%d hPa\r\n",(int32_t)(*pressure/100),(int32_t)(*pressure%100));	
			RUI_LOG_PRINTF("Gas_resistance: %d ohms \r\n", *resis);				
		}	
		else
        {
            RUI_LOG_PRINTF("BME680 Error.\r\n");
            return -1;
        } 
				
	return 0;
}