#ifndef __SENSOR_H__
#define __SENSOR_H__

typedef struct
{
	float voltage;
	uint32_t humidity;
    int16_t temperature;
    uint32_t pressure;  
    uint32_t  resis; 

}bsp_sensor_data_t;

uint32_t BoardBatteryMeasureVolage( float *voltage );
int BME680_get_data(uint32_t *humidity,int16_t* temp,uint32_t* pressure,uint32_t * resis);

#endif