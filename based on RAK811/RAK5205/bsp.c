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

extern bool HasFix;
extern double Latitude;
extern double Longitude;
extern RUI_DEVICE_STATUS_T app_device_status; //record device status 
extern RUI_LORA_STATUS_T app_lora_status; //record lora status 
RUI_GPIO_ST Gps_Power_Ctl;
bool gps_timeout_flag = false;
TimerEvent_t Gps_Cnt_Timer;
#define GPS_TIMER_VALUE 120
void Gps_Timeout_Event(void)
{
	gps_timeout_flag = true;
}
int GPS_get_data(double* latitude,double* longitude,int16_t* altitude)
{
	uint8_t gpio_status;
	HasFix = false;
	GpsStart();																		
	rui_timer_init( &Gps_Cnt_Timer, Gps_Timeout_Event );
	rui_timer_setvalue( &Gps_Cnt_Timer, GPS_TIMER_VALUE * 1000 );  
	RUI_LOG_PRINTF("Start Search Satellite(about %d seconds) ...\r\n",GPS_TIMER_VALUE);
	rui_timer_start(&Gps_Cnt_Timer); 

	while((gps_timeout_flag == false) && !HasFix)
	{
		/***************************************************************************
		 * wait for searching satellite
		****************************************************************************/
		rui_running();
	};

	if(HasFix)rui_delay_ms(2000);  //delay for smoothing

	gps_timeout_flag = false;

	if(HasFix)
	{
		rui_delay_ms(1000);
		RUI_LOG_PRINTF("Gps normal.\r\n");
		*latitude = Latitude;
		*longitude = Longitude;
		*altitude = GpsGetLatestGpsAltitude();
		RUI_LOG_PRINTF("latitude: %d.%d, longitude: %d.%d , altitude: %d.%dm \r\n",
                    (uint32_t)*latitude,(uint32_t)(*latitude*1000000-((int32_t)*latitude) * 1000000),
                    (uint32_t)*longitude,(uint32_t)(*longitude*1000000-((int32_t)*longitude) * 1000000),    
                    *altitude/10,*altitude%10);
		return 0;
	}else
	{
		RUI_LOG_PRINTF("FAIL.The Satellite signal not found!\r\n");
		return -1;
	}
	return 0;
}
