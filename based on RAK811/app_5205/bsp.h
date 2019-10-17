#ifndef __BSP_H__
#define __BSP_H__

extern const uint8_t level[2];
#define low     &level[0]
#define high    &level[1]

uint32_t BoardBatteryMeasureVolage( float *voltage );
int BME680_get_data(uint32_t *humidity,int16_t* temp,uint32_t* pressure,uint32_t * resis);
int GPS_get_data(double* latitude,double* longitude,int16_t* altitude);
int lis3dh_get_data(float* lis_X,float* lis_Y,float* lis_Z);

#endif  //__BSP_H__