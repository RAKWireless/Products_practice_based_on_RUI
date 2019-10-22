#include <string.h>
#include <stdbool.h>
#include "inner.h"
#include "rui.h"
#include "lis3dh.h"
#include "opt3001.h"
#include "lis2mdl.h"
#include "bme280.h"

//the task is the main period task for cellular, user can set on/off and period via at cmd
//do not modify

int RUI_CALLBACK_REGE_FLAG = 0;


//here will excute the period task to send data to user's server
void app_task(void * p_context)
{
    uint8_t gsm_cmd[100] = {0};
    uint8_t gsm_rsp[256] = {0};
    uint8_t send_data[256] = {0}; 
    uint8_t cellular_status = 1;
   	float temp = 0;
    float humidity = 0;
    float pressure = 0;
    int x = 0;
    int y = 0;
    int z = 0;
    float magnetic_x = 0;
    float magnetic_y = 0;
    float magnetic_z = 0;
    float light = 0;
    float voltage = 0;
    RUI_GPS_DATA g_gps_data = {0};
    uint8_t lat_data[20] = {0}; 
    uint8_t lon_data[20] = {0}; 
    RUI_LOG_PRINTF("app_task!!!");

    //sensors init
    rui_device_sleep(0);

    //connect to server
    rui_cellular_join();

    //get sensor data
    get_bme280_data(&temp,&humidity,&pressure);
    RUI_LOG_PRINTF("temperature = "NRF_LOG_FLOAT_MARKER"",NRF_LOG_FLOAT(temp));
    RUI_LOG_PRINTF("humidity = "NRF_LOG_FLOAT_MARKER"",NRF_LOG_FLOAT(humidity));
    RUI_LOG_PRINTF("pressure = "NRF_LOG_FLOAT_MARKER"",NRF_LOG_FLOAT(pressure)); 
        float _x = 0;
        float _y = 0;
        float _z = 0;
    lis3dh_twi_init();
    get_lis3dh_data(&x,&y,&z);
        _x =x * 4000/65536;
        _y =y * 4000/65536;
        _z =z * 4000/65536;   
        RUI_LOG_PRINTF("acceleration x = "NRF_LOG_FLOAT_MARKER"",NRF_LOG_FLOAT(_x));
        RUI_LOG_PRINTF("acceleration y = "NRF_LOG_FLOAT_MARKER"",NRF_LOG_FLOAT(_y));
        RUI_LOG_PRINTF("acceleration z = "NRF_LOG_FLOAT_MARKER"",NRF_LOG_FLOAT(_z));
        lis2mdl_twi_init();
    get_lis2mdl_data(&magnetic_x,&magnetic_y,&magnetic_z);
    RUI_LOG_PRINTF("magnetic x = "NRF_LOG_FLOAT_MARKER"",NRF_LOG_FLOAT(magnetic_x));
    RUI_LOG_PRINTF("magnetic y = "NRF_LOG_FLOAT_MARKER"",NRF_LOG_FLOAT(magnetic_y));
    RUI_LOG_PRINTF("magnetic z = "NRF_LOG_FLOAT_MARKER"",NRF_LOG_FLOAT(magnetic_z));
    opt3001_twi_init();
    get_opt3001_data(&light);
    RUI_LOG_PRINTF("light strength = "NRF_LOG_FLOAT_MARKER"",NRF_LOG_FLOAT(light));

    memset(lat_data,0,20);        
    rui_gps_get(&g_gps_data);
    sprintf(lat_data,"%lf",g_gps_data.Latitude);
    RUI_LOG_PRINTF("gps Latitude(0-N,1-S):%d,%s",g_gps_data.LatitudeNS,lat_data);
    memset(lon_data,0,20);
    sprintf(lon_data,"%lf",g_gps_data.Longitude);
    RUI_LOG_PRINTF("gps Longitude(0-E,1-W):%d,%s",g_gps_data.LongitudaEW,lon_data);
memset(send_data,0,256);

	sprintf(send_data,"Acc:%.2f,%.2f,%.2f; ",_x,_y,_z);
    sprintf(send_data+strlen(send_data),"Tem:%.2f;Hum:%.2f;Pre:%.2f; ",temp,humidity,pressure);
    sprintf(send_data+strlen(send_data),"Lig:%.2f; ",light);
    sprintf(send_data+strlen(send_data),"Mag:%.2f,%.2f,%.2f; ",magnetic_x,magnetic_y,magnetic_z);
    sprintf(send_data+strlen(send_data),"Lat(0-N,1-S):%d,%s,Lon(0-E,1-W):%d,%s; ",g_gps_data.LatitudeNS,lat_data,g_gps_data.LongitudaEW,lon_data); 


	//open tcp client with remote server
	memset(gsm_rsp,0,256);
	memset(gsm_cmd,0,100);	
    rui_cellular_connect_status(&cellular_status);
if (cellular_status == 1)
{
    rui_cellular_open_socket(gsm_cmd);
    RUI_LOG_PRINTF("gsm_cmd: %s",gsm_cmd);
    rui_cellular_send(gsm_cmd);
    rui_cellular_response(gsm_rsp, 256, 500 * 60);
    memset(gsm_rsp,0,256);
    rui_cellular_response(gsm_rsp, 256, 500 * 20);    

    
    //send
    memset(gsm_cmd,0,100);
    sprintf(gsm_cmd,"AT+QISEND=0,%d",strlen(send_data));
    RUI_LOG_PRINTF("gsm_cmd: %s",gsm_cmd);
    rui_cellular_send(gsm_cmd);
    rui_delay_ms(2000);
    rui_cellular_send(send_data);
    memset(gsm_rsp,0,256);
    rui_cellular_response(gsm_rsp, 256, 500 * 20);  

    //close socket
    rui_cellular_send("AT+QICLOSE=0");
    memset(gsm_rsp,0,256);
    rui_cellular_response(gsm_rsp, 256, 500 * 30);
    rui_device_sleep(1);
}
    
else
{
    rui_cellular_hologram_send();
    rui_device_sleep(1);
}

}