#include "rui.h"
#include "bsp.h"

bsp_sensor_data_t bsp_sensor;
user_store_data_t user_store_data;
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
		RUI_LOG_PRINTF("BME680 sensor data:\r\n");
		if(BME680_read(temp, pressure, humidity, resis) == 0)
		{		
			RUI_LOG_PRINTF("  Humidity:%d.%d %%RH\r\n",(int32_t)(*humidity/1000),(int32_t)(*humidity%1000));		
			RUI_LOG_PRINTF("  Temperature:%d.%d degree\r\n",(int32_t)(*temp/100),(int32_t)(*temp%100));	
			RUI_LOG_PRINTF("  Pressure:%d.%d hPa\r\n",(int32_t)(*pressure/100),(int32_t)(*pressure%100));	
			RUI_LOG_PRINTF("  Gas_resistance: %d ohms \r\n", *resis);				
		}	
		else
        {
            RUI_LOG_PRINTF("  Read BME680 Error.\r\n");
            return -1;
        } 
				
	return 0;
}

/********************************************************************************************************
 * get GPS data
********************************************************************************************************/
typedef struct
{
	double lat;
	double lon;
	int32_t alt;
	bool hasfix;
}GPS_DATA_T;
extern bool HasFix;
extern RUI_LORA_STATUS_T app_lora_status; //record lora status 
RUI_GPIO_ST Gps_Power_Ctl;
volatile bool gps_timeout_flag = true;  //If satellite detected fail,this flag will be true
TimerEvent_t Gps_Cnt_Timer;  //search satellite timer
#define GPS_SAMPLE_CNT 10
GPS_DATA_T gps_data[GPS_SAMPLE_CNT];

void Gps_Timeout_Event(void)
{
	gps_timeout_flag = true;
}

void median(GPS_DATA_T* data_t)
{
	int i, j;
	GPS_DATA_T tmp;
	for (i = 0; i < GPS_SAMPLE_CNT-1; i++) 
	{
		for (j = i + 1; j < GPS_SAMPLE_CNT; j++) 
		{
			if (data_t[j].lat < data_t[i].lat) 
			{
				tmp = data_t[j];
				data_t[j] = data_t[i];
				data_t[i] = tmp;  
			}
		}      
	}
}

int GPS_get_data(double* latitude,double* longitude,int16_t* altitude)
{

	uint8_t gpio_status;
	double latitude_tmp, longitude_tmp = 0;
	gps_timeout_flag = false;
	GpsStart();																		
	rui_timer_init( &Gps_Cnt_Timer, Gps_Timeout_Event ); 
	rui_timer_setvalue( &Gps_Cnt_Timer, user_store_data.gps_timeout_cnt * 1000 );  
	RUI_LOG_PRINTF("Start Search Satellite(about %d seconds) ...\r\n",user_store_data.gps_timeout_cnt);
	rui_timer_start(&Gps_Cnt_Timer); //start search satellite timer

	while((gps_timeout_flag == false) && !HasFix)
	{
		/***************************************************************************
		 * wait for searching satellite
		****************************************************************************/
		rui_running();  
	};
	rui_timer_stop(&Gps_Cnt_Timer);  //stop search satellite timer

	if(HasFix)	
	{
		for(uint8_t i=0;i<GPS_SAMPLE_CNT;i++)
		{
			GpsGetLatestGpsPositionDouble(&latitude_tmp, &longitude_tmp);
			gps84_To_Gcj02(latitude_tmp, longitude_tmp, &gps_data[i].lat, &gps_data[i].lon);
			GpsGetLatestGpsAltitude(&(gps_data[i].alt));
			gps_data[i].hasfix = HasFix;
			rui_delay_ms(500);
		}
		median(gps_data);

		if(gps_data[GPS_SAMPLE_CNT/2].hasfix)
		{
			* latitude = gps_data[GPS_SAMPLE_CNT/2].lat;
			* longitude = gps_data[GPS_SAMPLE_CNT/2].lon;
			* altitude = gps_data[GPS_SAMPLE_CNT/2].alt;
			RUI_LOG_PRINTF("GPS data:");
			RUI_LOG_PRINTF("latitude: %d.%d, longitude: %d.%d, altitude: %d.%dm \r\n",
						(int32_t)*latitude,abs((int32_t)(*latitude*1000000-((int32_t)*latitude) * 1000000)),
						(int32_t)*longitude,abs((int32_t)(*longitude*1000000-((int32_t)*longitude) * 1000000)),    
						*altitude/10,abs(*altitude%10));
			return 0;
		}else
		{
			RUI_LOG_PRINTF("FAIL.The Satellite signal not found!\r\n");
			return -1;
		}
	}else
	{
		RUI_LOG_PRINTF("FAIL.The Satellite signal not found!\r\n");
		return -1;
	}
	return 0;
}

/********************************************************************************************************
 * get LIS3DH data
********************************************************************************************************/
#include "lis3dh_reg.h"
extern lis3dh_ctx_t dev_ctx;
int lis3dh_get_data(float* lis_X,float* lis_Y,float* lis_Z)
{
	axis3bit16_t data_raw_acceleration;
	lis3dh_reg_t reg;
	lis3dh_xl_data_ready_get(&dev_ctx, &reg.byte);
	if (reg.byte)
    {
		/* Read accelerometer data */
		memset(data_raw_acceleration.u8bit, 0x00, 3*sizeof(int16_t));
		lis3dh_acceleration_raw_get(&dev_ctx, data_raw_acceleration.u8bit);
		*lis_X = lis3dh_from_fs2_hr_to_mg(data_raw_acceleration.i16bit[0]);
		*lis_Y = lis3dh_from_fs2_hr_to_mg(data_raw_acceleration.i16bit[1]);
		*lis_Z = lis3dh_from_fs2_hr_to_mg(data_raw_acceleration.i16bit[2]);
		RUI_LOG_PRINTF("LIS3DH sensor data:");
		RUI_LOG_PRINTF("ACC_X: %dmg, ACC_Y: %dmg, ACC_Z: %dmg\r\n",(int32_t)*lis_X , (int32_t)*lis_Y , (int32_t)*lis_Z );

		return 0;
    }else
	{
		RUI_LOG_PRINTF("  LIS3DH Error.\r\n");
		return -1;
	}
}
