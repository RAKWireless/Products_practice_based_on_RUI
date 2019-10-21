#ifndef __BSP_H__
#define __BSP_H__

extern const uint8_t level[2];
#define low     &level[0]
#define high    &level[1]

typedef struct
{
	float voltage;
	uint32_t humidity;
    int16_t temperature;
    uint32_t pressure;  
    uint32_t  resis; 
	float triaxial_x;
	float triaxial_y;
	float triaxial_z;
	double latitude;
    double longitude;
    int16_t altitude;
}bsp_sensor_data_t;

typedef struct 
{
	uint16_t gps_timeout_cnt;	//record gps search satellite timer 
}user_store_data_t;


uint32_t BoardBatteryMeasureVolage( float *voltage );
int BME680_get_data(uint32_t *humidity,int16_t* temp,uint32_t* pressure,uint32_t * resis);
int GPS_get_data(double* latitude,double* longitude,int16_t* altitude);
int lis3dh_get_data(float* lis_X,float* lis_Y,float* lis_Z);

#endif  //__BSP_H__