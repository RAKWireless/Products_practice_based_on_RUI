#include "board_basic.h"
#include "boards.h"
#include "nrf_delay.h"
#include "nrf_rtc.h"
#include <string.h>
#include "rui.h"
#include "nrf_log.h"

//the task is the main period task for cellular, user can set on/off and period via at cmd
//do not modify and just add own code inside.

int RUI_CALLBACK_REGE_FLAG = 0;

#if defined(LORA_4600_TEST)
//lora recieve callback
void LoRaReceive_callback(RUI_RECEIVE_T* Receive_datapackage)
{
    //user define 
}
#endif

//here will excute the period task to send data to user's server
void app_task(void * p_context)
{
    uint8_t gsm_cmd[100] = {0};
    uint8_t gsm_rsp[256] = {0};
    uint8_t send_data[256] = {0}; 
    uint8_t cellular_status = 1;
   	double temp = 0;
    double humidity = 0;
    double pressure = 0;
    float x = 0;
    float y = 0;
    float z = 0;
    float magnetic_x = 0;
    float magnetic_y = 0;
    float magnetic_z = 0;
    float light = 0;
    float voltage = 0;
    RUI_GPS_DATA g_gps_data = {0};
    uint8_t lat_data[20] = {0}; 
    uint8_t lon_data[20] = {0}; 
	NRF_LOG_INFO("app_task!!!");

#if defined(LORA_4600_TEST)
    if (RUI_CALLBACK_REGE_FLAG == 0)
    {
        rui_lora_register_recv_callback(LoRaReceive_callback);
        RUI_CALLBACK_REGE_FLAG = 1;
    }

#endif
    //sensors init
    rui_device_sleep(0);
	
#if defined(BC95G_TEST) || defined(M35_TEST) || defined(BG96_TEST)
    //connect to server
    rui_cellular_join();
#endif
    //get sensor data
#ifdef BEM280_TEST
    rui_temperature_get(&temp);
    NRF_LOG_INFO("temperature = "NRF_LOG_FLOAT_MARKER"",NRF_LOG_FLOAT(temp));
    rui_humidity_get(&humidity);
    NRF_LOG_INFO("humidity = "NRF_LOG_FLOAT_MARKER"",NRF_LOG_FLOAT(humidity));
    rui_pressure_get(&pressure);
    NRF_LOG_INFO("pressure = "NRF_LOG_FLOAT_MARKER"",NRF_LOG_FLOAT(pressure));
#endif
        
#ifdef LPS22HB_TEST
    rui_pressure_get(&pressure);
    NRF_LOG_INFO("pressure = %d hPa\r\n",pressure); 
#endif
#ifdef LIS3DH_TEST
    rui_acceleration_get(&x,&y,&z);
    NRF_LOG_INFO("acceleration x = "NRF_LOG_FLOAT_MARKER"",NRF_LOG_FLOAT(x));
    NRF_LOG_INFO("acceleration y = "NRF_LOG_FLOAT_MARKER"",NRF_LOG_FLOAT(y));
    NRF_LOG_INFO("acceleration z = "NRF_LOG_FLOAT_MARKER"",NRF_LOG_FLOAT(z));
        
#endif
#ifdef LIS2MDL_TEST
    rui_magnetic_get(&magnetic_x,&magnetic_y,&magnetic_z);
    NRF_LOG_INFO("magnetic x = "NRF_LOG_FLOAT_MARKER"",NRF_LOG_FLOAT(magnetic_x));
    NRF_LOG_INFO("magnetic y = "NRF_LOG_FLOAT_MARKER"",NRF_LOG_FLOAT(magnetic_y));
    NRF_LOG_INFO("magnetic z = "NRF_LOG_FLOAT_MARKER"",NRF_LOG_FLOAT(magnetic_z));
#endif
#ifdef OPT3001_TEST
    rui_light_get_strength(&light);
    NRF_LOG_INFO("light strength = "NRF_LOG_FLOAT_MARKER"",NRF_LOG_FLOAT(light));
#endif
        
#if defined(SHT31_TEST) || defined(SHTC3_TEST)
    rui_temperature_get(&temp);
    NRF_LOG_INFO("temperature = "NRF_LOG_FLOAT_MARKER"",NRF_LOG_FLOAT(temp));
    rui_humidity_get(&humidity);
    NRF_LOG_INFO("humidity = "NRF_LOG_FLOAT_MARKER"",NRF_LOG_FLOAT(humidity));
#endif

#if defined(L70R_TEST) ||  defined(BG96_TEST) || defined(MAX7_TEST)
    memset(lat_data,0,20);        
    rui_gps_get(&g_gps_data);
    sprintf(lat_data,"%lf",g_gps_data.Latitude);
    NRF_LOG_INFO("gps Latitude(0-N,1-S):%d,%s",g_gps_data.LatitudeNS,lat_data);
    memset(lon_data,0,20);
    sprintf(lon_data,"%lf",g_gps_data.Longitude);
    NRF_LOG_INFO("gps Longitude(0-E,1-W):%d,%s",g_gps_data.LongitudaEW,lon_data);
#endif
#ifdef  BATTERY_LEVEL_SUPPORT
    rui_device_get_battery_level(&voltage);
    NRF_LOG_INFO("Battery Voltage = "NRF_LOG_FLOAT_MARKER" V !\r\n", NRF_LOG_FLOAT(voltage));
#endif

#if defined(BC95G_TEST) || defined(M35_TEST) || defined(BG96_TEST)
	//open tcp client with remote server
	memset(gsm_rsp,0,256);
	memset(gsm_cmd,0,100);	
    rui_cellular_connect_status(&cellular_status);
if (cellular_status == 1)
{
    rui_cellular_open_socket(gsm_cmd);
    NRF_LOG_INFO("gsm_cmd: %s",gsm_cmd);
    rui_cellular_send(gsm_cmd);
    rui_cellular_response(gsm_rsp, 256, 500 * 60);
    memset(gsm_rsp,0,256);
    rui_cellular_response(gsm_rsp, 256, 500 * 20);    

    memset(send_data,0,256);
#ifdef LIS3DH_TEST	
	sprintf(send_data,"Acc:%.2f,%.2f,%.2f; ",x,y,z);
#endif

#if defined(SHT31_TEST) || defined(SHTC3_TEST)
	sprintf(send_data+strlen(send_data),"Tem:%.2f;Hum:%.2f; ",temp,humidity);
#endif

#ifdef LPS22HB_TEST	
	sprintf(send_data+strlen(send_data),"Pre:%.2f; ",pressure);
#endif

#ifdef BEM280_TEST	
	sprintf(send_data+strlen(send_data),"Tem:%.2f;Hum:%.2f;Pre:%.2f; ",temp,humidity,pressure);
#endif

#ifdef OPT3001_TEST
    sprintf(send_data+strlen(send_data),"Lig:%.2f; ",light);
#endif

#ifdef LIS2MDL_TEST
	sprintf(send_data+strlen(send_data),"Mag:%.2f,%.2f,%.2f; ",magnetic_x,magnetic_y,magnetic_z);
#endif

#if defined(L70R_TEST) ||  defined(BG96_TEST) || defined(MAX7_TEST)
    sprintf(send_data+strlen(send_data),"Lat(0-N,1-S):%d,%s,Lon(0-E,1-W):%d,%s; ",g_gps_data.LatitudeNS,lat_data,g_gps_data.LongitudaEW,lon_data); 
#endif

#ifdef  BATTERY_LEVEL_SUPPORT
    if(voltage>0)
    {   
	   sprintf(send_data+strlen(send_data),"Battery:%.2f; ",voltage);
    }

#endif
    //send
    memset(gsm_cmd,0,100);
    sprintf(gsm_cmd,"AT+QISEND=0,%d",strlen(send_data));
    NRF_LOG_INFO("gsm_cmd: %s",gsm_cmd);
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
#endif

#if defined(LORA_4600_TEST)

	memset(send_data,0x41,50);
    rui_lora_send(1,send_data,50);
    rui_device_sleep(1);
#endif
}