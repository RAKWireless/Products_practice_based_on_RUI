#include "board.h"
#include "rui.h"

#define VREFINT_CAL       ( *( uint16_t* )0x1FF80078 )  //VREF calibration value
extern RUI_GPIO_ST Bat_level;
extern RUI_GPIO_ST Adc_vref;

uint32_t BoardBatteryMeasureVolage( float *voltage )
{
    uint16_t vcal = VREFINT_CAL;
    uint16_t vref = 0;
    uint16_t vdiv = 0;

	rui_adc_get(&Bat_level,&vdiv);
	rui_adc_get(&Adc_vref,&vref);

    *voltage = (float)3000 * vcal * vdiv / (float)( vref * 4096);
			
    //                                vDiv
    // Divider bridge  VBAT <-> 100k -<--|-->- 150k <-> GND => vBat = (5 * vDiv )/3
    *voltage = (5 * *voltage )/3;
    DUartPrint("vdiv= %d ,vref= %d ,Vcal=%d\r\n",vdiv, vref,vcal);

}

int BME680_get_data(uint32_t *humidity,int16_t* temp,uint32_t* pressure)
{
		#include "BME680.h"
		uint32_t  resis;
		if(SUCCESS == BME680_read(temp, pressure, humidity, &resis))
		{		
			RUI_LOG_PRINTF("Humidity:%d.%d %%RH\r\n",(uint32_t)(*humidity/1000),(uint32_t)(*humidity%1000));		
			RUI_LOG_PRINTF("Temperature:%d.%d degree\r\n",(uint32_t)(*temp/100),(uint32_t)(*temp%100));	
			RUI_LOG_PRINTF("Pressure:%d.%d hPa\r\n",(uint32_t)(*pressure/100),(uint32_t)(*pressure%100));					
		}	
		else
        {
            RUI_LOG_PRINTF("BME680 Error.\r\n");
            return -1;
        } 
				
	return 0;
}