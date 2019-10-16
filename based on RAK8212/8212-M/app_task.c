#include <string.h>
#include "rui.h"
#include "lis3dh.h"

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
    float x = 0;
    float y = 0;
    float z = 0;
    RUI_GPS_DATA g_gps_data = {0};
    uint8_t lon_data[20] = {0}; 
    uint8_t lat_data[20] = {0}; 
	RUI_LOG_PRINTF("app_task!!!");

    //sensors init
    rui_device_sleep(0);

    //connect to server
    rui_cellular_join();

    //get sensor data

    lis3dh_twi_init();
    get_lis3dh_data(&x,&y,&z);
    x =x * 4000/65536;
    y =y * 4000/65536;
    z =z * 4000/65536;   
    RUI_LOG_PRINTF("acceleration x = "NRF_LOG_FLOAT_MARKER"",NRF_LOG_FLOAT(x));
    RUI_LOG_PRINTF("acceleration y = "NRF_LOG_FLOAT_MARKER"",NRF_LOG_FLOAT(y));
    RUI_LOG_PRINTF("acceleration z = "NRF_LOG_FLOAT_MARKER"",NRF_LOG_FLOAT(z)); 

    memset(lat_data,0,20);        
    rui_gps_get(&g_gps_data);
    sprintf(lat_data,"%lf",g_gps_data.Latitude);
    RUI_LOG_PRINTF("gps Latitude(0-N,1-S):%d,%s",g_gps_data.LatitudeNS,lat_data);
    memset(lon_data,0,20);
    sprintf(lon_data,"%lf",g_gps_data.Longitude);
    RUI_LOG_PRINTF("gps Longitude(0-E,1-W):%d,%s",g_gps_data.LongitudaEW,lon_data);


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

    memset(send_data,0,256);

	sprintf(send_data,"Acc:%.2f,%.2f,%.2f; ",x,y,z);

    sprintf(send_data+strlen(send_data),"Lat(0-N,1-S):%d,%s,Lon(0-E,1-W):%d,%s; ",g_gps_data.LatitudeNS,lat_data,g_gps_data.LongitudaEW,lon_data); 

    //send
    memset(gsm_cmd,0,100);
    sprintf(gsm_cmd,"AT+QISEND=0,%d",strlen(send_data));
    RUI_LOG_PRINTF("gsm_cmd: %s",gsm_cmd);
    rui_cellular_send(gsm_cmd);
    delay_ms(2000);
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