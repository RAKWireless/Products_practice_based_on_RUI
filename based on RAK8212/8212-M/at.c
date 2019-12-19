#include <stdbool.h>
#include <string.h>
#include "inner.h"
#include "at.h"
#include "lis3dh.h"

/***************below is the inner rui_config_struct**********************
typedef struct {
        uint8_t sof;
        uint8_t join_mode;
        uint8_t work_mode;
        uint8_t class;
        uint8_t confirm;
        uint8_t region;
        uint8_t dev_eui[8];
        uint8_t app_eui[8];
        uint8_t app_key[16];
        uint32_t dev_addr;
        uint8_t nwkskey[16];
        uint8_t appskey[16];
} lora_cfg_t;

typedef struct {
        uint8_t server_ip[20];
        uint8_t server_port[20];        
        uint8_t operator_long_data[20]; 
        uint8_t operator_short_data[20]; 
        uint8_t operator_apn_data[20];
        uint8_t operator_net_data[20];
        uint8_t hologram_card_num[20];
} cellular_cfg_t;


typedef struct{
    uint8_t work_mode;  // 0:ble peripheral  1:ble central  2:ble observer 
    uint8_t long_range_enable;
    uint16_t reserve;
}ble_central_cfg_t;


typedef struct {
        uint8_t sof;
        uint8_t sleep_enable;
        uint32_t sleep_period;
        lora_cfg_t g_lora_cfg_t;
        S_LORAP2P_PARAM g_lora_p2p_cfg_t;
        cellular_cfg_t g_cellular_cfg_t;
        ble_central_cfg_t g_ble_cfg_t;
        uint8_t user_data[128];
} rui_cfg_t;
******************************************************************************/

rui_cfg_t g_rui_cfg_t = {0};

int at_flag = 0;
int power_flag = 0;


uint8_t hologram_cmd[256] = {0};
void hologram_cmd_packet(uint8_t *key, uint8_t *data)
{
    uint8_t i = 0;
    uint8_t j = 0;
    hologram_cmd[0]= '{';
    hologram_cmd[1]= '\"';
    hologram_cmd[2]= 'k';
    hologram_cmd[3]= '\"';
    hologram_cmd[4]= ':';  
    hologram_cmd[5]= '\"';
    for (i = 0; i < 8; i++)
    {
        hologram_cmd[6+i] = key[j++];
    }
    hologram_cmd[14] = '\"';
    hologram_cmd[15] = ',';
    hologram_cmd[16]= '\"';
    hologram_cmd[17]= 'd';
    hologram_cmd[18]= '\"';
    hologram_cmd[19]= ':';  
    hologram_cmd[20]= '\"';
    j = 0;
    for (i = 0; i < 256; i++)
    {
        if (data[j] != 0)
        {
            hologram_cmd[21+i] = data[j++];
        }
        else
        {
            break;
        }
    }    
    hologram_cmd[21+j]='\"';
    hologram_cmd[22+j]=',';
    hologram_cmd[23+j]='\"'; 
    hologram_cmd[24+j]='t';       
    hologram_cmd[25+j]='\"';
    hologram_cmd[26+j]=':';
    hologram_cmd[27+j]='\"';  
    hologram_cmd[28+j]='T'; 
    hologram_cmd[29+j]='O'; 
    hologram_cmd[30+j]='P'; 
    hologram_cmd[31+j]='I'; 
    hologram_cmd[32+j]='C';
    hologram_cmd[33+j]='1'; 
    hologram_cmd[34+j]='\"';
    hologram_cmd[35+j]='}';                
}

void StrToHex(char *pbDest, char *pbSrc, int nLen)
{
    char h1,h2;
    char s1,s2;
    int i;
    
    for (i=0; i<nLen; i++)
    {
        h1 = pbSrc[2*i];
        h2 = pbSrc[2*i+1];
        
        s1 = toupper(h1) - 0x30;
        if (s1 > 9) 
            s1 -= 7;
        
        s2 = toupper(h2) - 0x30;
        if (s2 > 9) 
            s2 -= 7;
        
        pbDest[i] = s1*16 + s2;
    }
}

void at_parse(char *cmd)
{
	char  *ptr = NULL;
    uint8_t gsm_cmd[100] = {0};
    uint8_t gsm_rsp[256] = {0};
    uint8_t at_rsp[1536] = {0};
    uint8_t ip_data[20] = {0};
    uint8_t port_data[20] = {0};
    uint8_t operator_long_data[20] = {0};  
    uint8_t operator_short_data[20] = {0};
    uint8_t operator_apn_data[20] = {0}; 
    uint8_t operator_net_data[20] = {0};            
    uint8_t send_data[256] = {0};  
    uint8_t send_p2p_data[256] = {0};     
    uint8_t lora_port[5] = {0};
    uint8_t sleep_data[10] = {0};    
    uint8_t index = 0;
    uint32_t err_code = 0;
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
    uint8_t lora_config_data[10] = {0};
    uint8_t msg_flash_success[]="Config success.\r\n";
    uint8_t msg_flash_failed[]="Config failed.\r\n";
    RUI_LOG_PRINTF("at_parse: %s",cmd);

	if(cmd[0] == 0)
    {
    	return;
    }    

    if(strstr(cmd,"at+version")==NULL && strstr(cmd,"at+set_config=device:sleep:")==NULL && strstr(cmd,"at+set_config=device:gps:")==NULL && 
       strstr(cmd,"at+set_config=cellular:send_interval:")==NULL && strstr(cmd,"at+set_config=device:restart")==NULL && strstr(cmd,"at+get_config=device:status")==NULL && 
       strstr(cmd,"at+set_config=device:cellular:")==NULL && strstr(cmd,"at+scan=cellular")==NULL && strstr(cmd,"at+set_config=cellular:")==NULL && 
       strstr(cmd,"at+send=cellular:")==NULL && strstr(cmd,"at+set_config=cellular:(")==NULL && strstr(cmd,"at+set_config=lora:dev_eui:")==NULL && 
       strstr(cmd,"at+set_config=lora:app_eui:")==NULL && strstr(cmd,"at+set_config=lora:app_key:")==NULL && strstr(cmd,"at+set_config=lora:dev_addr:")==NULL &&
       strstr(cmd,"at+set_config=lora:apps_key:")==NULL && strstr(cmd,"at+set_config=lora:nwks_key:")==NULL && strstr(cmd,"at+set_config=lora:region:")==NULL &&
       strstr(cmd,"at+set_config=lora:join_mode:")==NULL && strstr(cmd,"at+join")==NULL && strstr(cmd,"at+send=lora:")==NULL &&
       strstr(cmd,"at+set_config=lora:work_mode:")==NULL && strstr(cmd,"at+set_config=lora:class:")==NULL && strstr(cmd,"at+set_config=lora:confirm:")==NULL &&
       strstr(cmd,"at+set_config=lora:send_interval:")==NULL && strstr(cmd,"at+get_config=lora:status")==NULL && strstr(cmd,"at+get_config=lora:channel")==NULL &&
       strstr(cmd,"at+set_config=lora:ch_mask:")==NULL && strstr(cmd,"at+set_config=ble:work_mode:")==NULL && strstr(cmd,"at+set_config=lorap2p:")==NULL &&
       strstr(cmd,"at+send=lorap2p:")==NULL  && strstr(cmd,"at+set_config=device:boot")==NULL  && strstr(cmd,"at+run")==NULL  &&
       strstr(cmd,"at+set_config=lora:adr:")==NULL && strstr(cmd,"at+set_config=lora:dr:")==NULL && strstr(cmd,"at+set_config=hologram:")==NULL &&
       strstr(cmd,"at+send=hologram:sensor")==NULL && strstr(cmd,"at+send=hologram:user:")==NULL && strstr(cmd,"at+help")==NULL)
    {
        memset(at_rsp,0,1536);
        //RUI_LOG_PRINTF("Invalid at command!!");
        memcpy(at_rsp,cmd,strlen(cmd));
        memcpy(at_rsp+strlen(at_rsp),"ERROR:RUI_AT_UNSUPPORT",strlen("ERROR:RUI_AT_UNSUPPORT"));
        rui_at_response(false, at_rsp, RUI_AT_UNSUPPORT);
        return;
    }  
    if(strstr(cmd,"version")!= 0)
    {
        char ver[48]="Firmware Version: RUI v";
        strcat(ver, RUI_VERSION);
		//RUI_LOG_PRINTF("%s", ver);
        memset(at_rsp,0,1536);
        memcpy(at_rsp,"at+version\r\n",strlen("at+version\r\n"));
        sprintf(at_rsp+strlen(at_rsp),"%s\r\n",ver);
        rui_at_response(true, at_rsp, RAK_OK);
        return;
    }
    if(strstr(cmd,"device:sleep:1")!= 0)
    {
		//RUI_LOG_PRINTF("Device has been sleep!");
        if(strstr(cmd,"1") == NULL && strstr(cmd,"0") == NULL)
        {
            memset(at_rsp,0,1536);
            memcpy(at_rsp,cmd,strlen(cmd));
            memcpy(at_rsp+strlen(at_rsp),"ERROR:RUI_AT_PARAMETER_INVALID",strlen("ERROR:RUI_AT_PARAMETER_INVALID"));
            rui_at_response(false, at_rsp, RUI_AT_PARAMETER_INVALID);
            return;
        }
        memset(at_rsp,0,1536);
        sprintf(at_rsp,"%s",cmd);
        //sprintf(at_rsp+strlen(at_rsp),"%s\r\n",ver);
        rui_at_response(true, at_rsp, RAK_OK);

		if(power_flag == 0)
		{
			rui_device_sleep(1);
			power_flag =1;
		}
        return;
    }
    if(strstr(cmd,"device:sleep:0")!= 0)
    {
        if(strstr(cmd,"1") == NULL && strstr(cmd,"0") == NULL)
        {
            memset(at_rsp,0,1536);
            memcpy(at_rsp,cmd,strlen(cmd));
            memcpy(at_rsp+strlen(at_rsp),"ERROR:RUI_AT_PARAMETER_INVALID",strlen("ERROR:RUI_AT_PARAMETER_INVALID"));
            rui_at_response(false, at_rsp, RUI_AT_PARAMETER_INVALID);
            return;
        }
		rui_device_sleep(0);
		power_flag = 0;
        memset(at_rsp,0,1536);
        sprintf(at_rsp,"%s",cmd);
        //sprintf(at_rsp+strlen(at_rsp),"%s\r\n",ver);
        rui_at_response(true, at_rsp, RAK_OK);
        return;
    }
    if(strstr(cmd,"device:gps:0")!= 0)
    {
        //bg96 inner gps will automatically off when not use
        if(strstr(cmd,"1") == NULL && strstr(cmd,"0") == NULL)
        {
            memset(at_rsp,0,1536);
            memcpy(at_rsp,cmd,strlen(cmd));
            memcpy(at_rsp+strlen(at_rsp),"ERROR:RUI_AT_PARAMETER_INVALID",strlen("ERROR:RUI_AT_PARAMETER_INVALID"));
            rui_at_response(false, at_rsp, RUI_AT_PARAMETER_INVALID);
            return;
        }
        memset(at_rsp,0,1536);
        sprintf(at_rsp,"%s",cmd);
        rui_at_response(true, at_rsp, RAK_OK);
        return;
    }
    if(strstr(cmd,"device:gps:1")!= 0)
    {
        //bg96 inner gps will automatically off when not use
        if(strstr(cmd,"1") == NULL && strstr(cmd,"0") == NULL)
        {
            memset(at_rsp,0,1536);
            memcpy(at_rsp,cmd,strlen(cmd));
            memcpy(at_rsp+strlen(at_rsp),"ERROR:RUI_AT_PARAMETER_INVALID",strlen("ERROR:RUI_AT_PARAMETER_INVALID"));
            rui_at_response(false, at_rsp, RUI_AT_PARAMETER_INVALID);
            return;
        }
        memset(at_rsp,0,1536);
        sprintf(at_rsp,"%s",cmd);
        rui_at_response(true, at_rsp, RAK_OK);
        return;
    }

    if(strstr(cmd,"cellular:send_interval")!= 0)
    {
		//start app timer and send data to server
		ptr = NULL;
        index = 0;
        memset(sleep_data,0,10);
        ptr = strstr(cmd,"interval:");
        for(index;index<9;index++)
        {
           ptr++;
        }
        index = 0;
		if(*ptr == '1')
        {
			g_rui_cfg_t.sleep_enable = 1;
        }
        if(*ptr == '0')
        {
			g_rui_cfg_t.sleep_enable = 0;
        }
		ptr++;
        ptr++;
        index = 0;
        for(ptr;*ptr !='\0';ptr++)
        {
            sleep_data[index++] = *ptr;
        }
		if(atoi(sleep_data)<150000)
		{
            memset(at_rsp,0,1536);
            memcpy(at_rsp,cmd,strlen(cmd));
            memcpy(at_rsp+strlen(at_rsp),"ERROR:RUI_AT_PARAMETER_INVALID",strlen("ERROR:RUI_AT_PARAMETER_INVALID"));
            rui_at_response(false, at_rsp, RUI_AT_PARAMETER_INVALID);
            return;
		}
        g_rui_cfg_t.sleep_period = atoi(sleep_data);

        RUI_LOG_PRINTF("g_rui_cfg_t.sleep_enable = %d",g_rui_cfg_t.sleep_enable); 
        RUI_LOG_PRINTF("g_rui_cfg_t.sleep_period = %d",g_rui_cfg_t.sleep_period);
        err_code = rui_flash_write(RUI_FLASH_ORIGIN,NULL,0);
        memset(at_rsp,0,1536);
        if (err_code != 0)
        {
            memcpy(at_rsp,cmd,strlen(cmd));
            memcpy(at_rsp+strlen(at_rsp),"ERROR:RUI_AT_RW_FLASH_ERROR",strlen("ERROR:RUI_AT_RW_FLASH_ERROR"));
            rui_at_response(false, at_rsp, RUI_AT_RW_FLASH_ERROR);
        }
        else
        {
            sprintf(at_rsp,"%s",cmd);
            rui_at_response(true, at_rsp, RAK_OK);   
        }
        return;
    }    
    if(strstr(cmd,"device:restart")!= NULL)
    {
        uint8_t msg[64];
        
        sprintf(msg, "%s", "Device will Reset after 3s...\r\n");
        RUI_LOG_PRINTF("%s", msg);
        #ifdef USER_UART
        rui_uart_send(RUI_UART1, msg, strlen(msg));
        #endif
    	rui_delay_ms(1000);
        
    	sprintf(msg, "Device will Reset after 2s...\r\n");
        RUI_LOG_PRINTF("%s", msg);
        #ifdef USER_UART
        rui_uart_send(RUI_UART1, msg, strlen(msg));
        #endif
   	 	rui_delay_ms(1000);    

        sprintf(msg, "Device will Reset after 1s...\r\n");
        RUI_LOG_PRINTF("%s", msg);
        #ifdef USER_UART
        rui_uart_send(RUI_UART1, msg, strlen(msg));
        #endif
    	rui_delay_ms(1000);
        memset(at_rsp,0,1536);
        sprintf(at_rsp,"%s",cmd);
        rui_at_response(true, at_rsp, RAK_OK);
		rui_device_reset();
        return;
    }
    if(strstr(cmd,"device:status")!= NULL)
    {
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
        memset(lat_data,0,20);        
        rui_gps_get(&g_gps_data);
        sprintf(lat_data,"%lf",g_gps_data.Latitude);
        RUI_LOG_PRINTF("gps Latitude(0-N,1-S):%d,%s",g_gps_data.LatitudeNS,lat_data);
        memset(lon_data,0,20);
        sprintf(lon_data,"%lf",g_gps_data.Longitude);
        RUI_LOG_PRINTF("gps Longitude(0-E,1-W):%d,%s",g_gps_data.LongitudaEW,lon_data);
		
    memset(send_data,0,256); 
    sprintf(send_data,"Acc:%.2f,%.2f,%.2f; ",_x,_y,_z);
    sprintf(send_data+strlen(send_data),"Lat(0-N,1-S):%d,%s,Lon(0-E,1-W):%d,%s; ",g_gps_data.LatitudeNS,lat_data,g_gps_data.LongitudaEW,lon_data); 


        memset(at_rsp,0,1536);
        memcpy(at_rsp,cmd,strlen(cmd));
        sprintf(at_rsp+strlen(at_rsp),"%s\r\n",send_data);
        rui_at_response(true, at_rsp, RAK_OK);
        return;
    }
    //at+set_config=device:cellular:0
    if(strstr(cmd,"device:cellular:0")!= NULL)
    {
        if(strstr(cmd,"1") == NULL && strstr(cmd,"0") == NULL)
        {
            memset(at_rsp,0,1536);
            memcpy(at_rsp,cmd,strlen(cmd));
            memcpy(at_rsp+strlen(at_rsp),"ERROR:RUI_AT_PARAMETER_INVALID",strlen("ERROR:RUI_AT_PARAMETER_INVALID"));
            rui_at_response(false, at_rsp, RUI_AT_PARAMETER_INVALID);
            return;
        }
        rui_cellular_set_mode(RUI_POWER_OFF_MODE);
        memset(at_rsp,0,1536);
        memcpy(at_rsp,cmd,strlen(cmd));
        rui_at_response(true, at_rsp, RAK_OK);
        return;
	}
    //at+set_config=device:cellular:1
    if(strstr(cmd,"device:cellular:1")!= NULL)
    {
        if(strstr(cmd,"1") == NULL && strstr(cmd,"0") == NULL)
        {
            memset(at_rsp,0,1536);
            memcpy(at_rsp,cmd,strlen(cmd));
            memcpy(at_rsp+strlen(at_rsp),"ERROR:RUI_AT_PARAMETER_INVALID",strlen("ERROR:RUI_AT_PARAMETER_INVALID"));
            rui_at_response(false, at_rsp, RUI_AT_PARAMETER_INVALID);
            return;
        }
        rui_cellular_set_mode(RUI_POWER_ON_MODE);
        memset(at_rsp,0,1536);
        memcpy(at_rsp,cmd,strlen(cmd));
        rui_at_response(true, at_rsp, RAK_OK);
        return;
    }
    //at+scan=cellular
    if(strstr(cmd,"at+scan=cellular")!= NULL)
    {
        memset(gsm_rsp,0,256);
        rui_cellular_send("AT+COPS=?");
        rui_cellular_response(gsm_rsp, 256, 500 * 60);
        return;
    }
    //at+set_config=device:cellular:()
    if(strstr(cmd,"cellular:(")!= NULL)
    {
		ptr = NULL;
        int mqtt_flag = 0;
		memset(gsm_cmd,0,100);
        memset(gsm_rsp,0,256);
        index = 0;
        uint16_t len = 0;
        uint8_t tmp[10] = {0};
   		ptr = strstr(cmd,"(");
		for(index;index<1;index++)
        {
			ptr++;
        }
        index = 0;
		for(ptr;*ptr !=')';ptr++)
        {
			gsm_cmd[index++] = *ptr;
        }
        
        RUI_LOG_PRINTF("gsm_cmd: %s",gsm_cmd);
        if (strstr(gsm_cmd,"AT+QIOPEN")!= NULL)//open
        {
            rui_cellular_send(gsm_cmd);
            rui_cellular_response(gsm_rsp, 256, 500 * 2);
            memset(gsm_rsp,0,256);
            rui_cellular_response(gsm_rsp, 256, 500 * 10);

        }
        else if (strstr(gsm_cmd,"AT+QISEND")!= NULL)//send (AT+QISEND=0,9)
        {
            ptr = strstr(cmd,"QISEND=");
            for(index=0;index<9;index++)
            {
                ptr++;
            }
            index = 0;
            for(ptr;*ptr !=')';ptr++)
            {
                tmp[index++] = *ptr;
            }
            len = atoi(tmp) + 2;
            memset(gsm_cmd,0,100);
            sprintf(gsm_cmd,"AT+QISEND=0,%d",len);
            rui_cellular_send(gsm_cmd);
            rui_delay_ms(1000);
			memset(gsm_rsp,0,256);
            rui_cellular_response(gsm_rsp, 256, 500 * 2);
        }
        else if (strstr(gsm_cmd,"AT+QPING")!= NULL)
        {
            rui_cellular_send(gsm_cmd);
            rui_cellular_response(gsm_rsp, 256, 500 * 2);
            memset(gsm_rsp,0,256);
            rui_cellular_response(gsm_rsp, 256, 500 * 25);
        }
        else if (strstr(gsm_cmd,"AT+QMTOPEN")!= NULL && strstr(gsm_cmd,",")!= NULL)
        {
            rui_cellular_send(gsm_cmd);
            rui_cellular_response(gsm_rsp, 256, 500 * 2);
            memset(gsm_rsp,0,256);
            rui_cellular_response(gsm_rsp, 256, 500 * 15);
        }
        else if (strstr(gsm_cmd,"AT+QMTCONN=")!= NULL && strstr(gsm_cmd,",")!= NULL)
        {
            rui_cellular_send(gsm_cmd);
            rui_cellular_response(gsm_rsp, 256, 500 * 2);
            memset(gsm_rsp,0,256);
            rui_cellular_response(gsm_rsp, 256, 500 * 15);
        }
        else if (strstr(gsm_cmd,"AT+QMTSUB")!= NULL && strstr(gsm_cmd,",")!= NULL)
        {
            rui_cellular_send(gsm_cmd);
            rui_cellular_response(gsm_rsp, 256, 500 * 2);
            memset(gsm_rsp,0,256);
            rui_cellular_response(gsm_rsp, 256, 500 * 15);
        }        
        else if (strstr(gsm_cmd,"AT+QMTUNS")!= NULL && strstr(gsm_cmd,",")!= NULL)
        {
            rui_cellular_send(gsm_cmd);
            rui_cellular_response(gsm_rsp, 256, 500 * 2);
            memset(gsm_rsp,0,256);
            rui_cellular_response(gsm_rsp, 256, 500 * 15);
        } 
        else if (strstr(gsm_cmd,"AT+QMTPUB")!= NULL && strstr(gsm_cmd,",")!= NULL)
        {
            rui_cellular_send(gsm_cmd);
            rui_cellular_response(gsm_rsp, 256, 500 * 5);
            mqtt_flag =1;
        }         
        else if (mqtt_flag==1)
        {
            mqtt_flag=0;
            rui_cellular_send(gsm_cmd);
            rui_cellular_response(gsm_rsp, 256, 500 * 2);
            memset(gsm_rsp,0,256);
            rui_cellular_response(gsm_rsp, 256, 500 * 15);
        }
        else 
        {
            rui_cellular_send(gsm_cmd);
            rui_cellular_response(gsm_rsp, 256, 500 * 60);
        }

        return;
    }
    //at+send=cellular:XXX
    if(strstr(cmd,"at+send=cellular:")!= NULL)
    {

         ptr = NULL;
         memset(gsm_cmd,0,100);
         memset(gsm_rsp,0,256);
         memset(send_data,0,256);         
         index = 0;
         //get ip
         ptr = strstr(cmd,":");
         for(index;index<1;index++)
         {
            ptr++;
         }
         index = 0;
         for(ptr;*ptr !='\0';ptr++)
         {
            send_data[index++] = *ptr;
         }
         //open tcp client with remote server
		 sprintf(gsm_cmd,"AT+QIOPEN=1,0,\"TCP\",\"%s\",%s,0,1",g_rui_cfg_t.g_cellular_cfg_t.server_ip,g_rui_cfg_t.g_cellular_cfg_t.server_port);
         RUI_LOG_PRINTF("gsm_cmd: %s",gsm_cmd);
         rui_cellular_send(gsm_cmd);
         rui_cellular_response(gsm_rsp, 256, 500 * 60);
         memset(gsm_rsp,0,256);
         rui_cellular_response(gsm_rsp, 256, 500 * 20);         
		 //send data
		 memset(gsm_cmd,0,100);
         sprintf(gsm_cmd,"AT+QISEND=0,%d",strlen(send_data));
         RUI_LOG_PRINTF("gsm_cmd: %s",gsm_cmd);
         rui_cellular_send(gsm_cmd);
		 rui_delay_ms(1000);
         rui_cellular_send(send_data);
         memset(gsm_rsp,0,256);
         rui_cellular_response(gsm_rsp, 256, 500 * 20);
         //close socket
         rui_cellular_send("AT+QICLOSE=0");
         memset(gsm_rsp,0,256);
         rui_cellular_response(gsm_rsp, 256, 500 * 30);
         //rui_at_response(true, NULL, RAK_OK);
        return;
     }
     //at+set_config=cellular:ip:port:CHINA MOBILE:CMCC:CMNET:net service
     if(strstr(cmd,"at+set_config=cellular:")!= NULL && strstr(cmd,"cellular:(")== NULL)
     {
         ptr = NULL;
         memset(ip_data,0,20);
         memset(port_data,0,20);  
         memset(operator_long_data,0,20);
         memset(operator_short_data,0,20);          
         memset(operator_apn_data,0,20);
         memset(operator_net_data,0,20);        
         index = 0;
         //get ip
         ptr = strstr(cmd,"cellular:");
         for(index;index<9;index++)
         {
            ptr++;
         }
         index = 0;
         for(ptr;*ptr !=':';ptr++)
         {
            ip_data[index++] = *ptr;
         }
         //get port
         ptr++;
         index = 0;
         for(ptr;*ptr !=':';ptr++)
         {
            port_data[index++] = *ptr;
         }
         ptr++;
         index = 0; 
         for(ptr;*ptr !=':';ptr++)
         {
            operator_long_data[index++] = *ptr;
         }  
         ptr++;
         index = 0; 
         for(ptr;*ptr !=':';ptr++)
         {
            operator_short_data[index++] = *ptr;
         }      
         ptr++;
         index = 0; 
         for(ptr;*ptr !=':';ptr++)
         {
            operator_apn_data[index++] = *ptr;
         }    
         ptr++;
         index = 0; 
         for(ptr;*ptr !='\0';ptr++)
         {
            operator_net_data[index++] = *ptr;
         }      
         memset(&(g_rui_cfg_t.g_cellular_cfg_t.server_ip[0]),0,20);
		 memcpy(&(g_rui_cfg_t.g_cellular_cfg_t.server_ip[0]),ip_data,20);
         memset(&(g_rui_cfg_t.g_cellular_cfg_t.server_port[0]),0,20);
		 memcpy(&(g_rui_cfg_t.g_cellular_cfg_t.server_port[0]),port_data,20);   
         memset(&(g_rui_cfg_t.g_cellular_cfg_t.operator_long_data[0]),0,20);
         memcpy(&(g_rui_cfg_t.g_cellular_cfg_t.operator_long_data[0]),operator_long_data,20);
         memset(&(g_rui_cfg_t.g_cellular_cfg_t.operator_short_data[0]),0,20);
         memcpy(&(g_rui_cfg_t.g_cellular_cfg_t.operator_short_data[0]),operator_short_data,20);  
         memset(&(g_rui_cfg_t.g_cellular_cfg_t.operator_apn_data[0]),0,20);
         memcpy(&(g_rui_cfg_t.g_cellular_cfg_t.operator_apn_data[0]),operator_apn_data,20);   
         memset(&(g_rui_cfg_t.g_cellular_cfg_t.operator_net_data[0]),0,20);
         memcpy(&(g_rui_cfg_t.g_cellular_cfg_t.operator_net_data[0]),operator_net_data,20);                       
         err_code = rui_flash_write(RUI_FLASH_ORIGIN,NULL,0);                    
        memset(at_rsp,0,1536);
        if (err_code != 0)
        {
            memcpy(at_rsp,cmd,strlen(cmd));
            memcpy(at_rsp+strlen(at_rsp),"ERROR:RUI_AT_RW_FLASH_ERROR",strlen("ERROR:RUI_AT_RW_FLASH_ERROR"));
            rui_at_response(false, at_rsp, RUI_AT_RW_FLASH_ERROR);
        }
        else
        {
            sprintf(at_rsp,"%s",cmd);
            rui_at_response(true, at_rsp, RAK_OK);   
        }
        return;
     }
     //at+set_config=hologram:
     if(strstr(cmd,"at+set_config=hologram:")!= NULL)
     {
         ptr = NULL;      
         index = 0;
         ptr = strstr(cmd,"hologram:");
         for(index;index<9;index++)
         {
            ptr++;
         }
         index = 0;
         memset(g_rui_cfg_t.g_cellular_cfg_t.hologram_card_num,0,20);
         for(ptr;*ptr !='\0';ptr++)
         {
            g_rui_cfg_t.g_cellular_cfg_t.hologram_card_num[index++] = *ptr;
         };
         err_code = rui_flash_write(RUI_FLASH_ORIGIN,NULL,0);    
        memset(at_rsp,0,1536);
        if (err_code != 0)
        {
            memcpy(at_rsp,cmd,strlen(cmd));
            memcpy(at_rsp+strlen(at_rsp),"ERROR:RUI_AT_RW_FLASH_ERROR",strlen("ERROR:RUI_AT_RW_FLASH_ERROR"));
            rui_at_response(false, at_rsp, RUI_AT_RW_FLASH_ERROR);
        }
        else
        {
            sprintf(at_rsp,"%s",cmd);
            rui_at_response(true, at_rsp, RAK_OK);   
        }
        return;
     }
     //at+send=hologram:sensor
     if(strstr(cmd,"at+send=hologram:sensor")!= NULL)
     {
        float _x = 0;
        float _y = 0;
        float _z = 0;
        lis3dh_twi_init();
        get_lis3dh_data(&x,&y,&z);
    _x =x * 4000/65536;
    _y =y * 4000/65536;
    _z =z * 4000/65536;   

        memset(lat_data,0,20);        
        rui_gps_get(&g_gps_data);
        sprintf(lat_data,"%lf",g_gps_data.Latitude);
        memset(lon_data,0,20);
        sprintf(lon_data,"%lf",g_gps_data.Longitude);
         memset(gsm_cmd,0,100);
         memset(gsm_rsp,0,256);
         memset(send_data,0,256);   
         memset(hologram_cmd,0,256); 

    sprintf(send_data,"Acc:%.2f,%.2f,%.2f; ",_x,_y,_z);

    sprintf(send_data+strlen(send_data),"Lat(0-N,1-S):%d,%s,Lon(0-E,1-W):%d,%s; ",g_gps_data.LatitudeNS,lat_data,g_gps_data.LongitudaEW,lon_data); 


         hologram_cmd_packet(g_rui_cfg_t.g_cellular_cfg_t.hologram_card_num,send_data);
         RUI_LOG_PRINTF("send_data: %s",send_data);
         RUI_LOG_PRINTF("hologram_cmd: %s",hologram_cmd); 
         //open tcp client with remote server
         rui_cellular_send("AT+QIOPEN=1,0,\"TCP\",\"cloudsocket.hologram.io\",9999,0,1");
         rui_cellular_response(gsm_rsp, 256, 500 * 2);
         memset(gsm_rsp,0,256);
         rui_cellular_response(gsm_rsp, 256, 500 * 10);   
         //send data
         memset(gsm_cmd,0,100);
         sprintf(gsm_cmd,"AT+QISEND=0,%d",strlen(hologram_cmd));
         RUI_LOG_PRINTF("gsm_cmd: %s",gsm_cmd);
         rui_cellular_send(gsm_cmd);
         rui_delay_ms(10);
         rui_cellular_send(hologram_cmd);
         memset(gsm_rsp,0,256);
         rui_cellular_response(gsm_rsp, 256, 500 * 30);
         memset(gsm_rsp,0,256);
         rui_cellular_response(gsm_rsp, 256, 500 * 30);
         //close socket
         rui_cellular_send("AT+QICLOSE=0,30000");
         memset(gsm_rsp,0,256);
         rui_cellular_response(gsm_rsp, 256, 500 * 60);
         memset(at_rsp,0,1536);
         sprintf(at_rsp,"%s",cmd);
         rui_at_response(true, at_rsp, RAK_OK);   
        return;
     }
     //at+send=hologram:user:
     if(strstr(cmd,"at+send=hologram:user")!= NULL)
     {
         ptr = NULL;
         memset(gsm_cmd,0,100);
         memset(gsm_rsp,0,256);
         memset(send_data,0,256);   
         memset(hologram_cmd,0,256);      
         index = 0;
         //get ip
         ptr = strstr(cmd,"user");
         for(index;index<5;index++)
         {
            ptr++;
         }
         index = 0;
         for(ptr;*ptr !='\r';ptr++)
         {
            send_data[index++] = *ptr;
         }
         hologram_cmd_packet(g_rui_cfg_t.g_cellular_cfg_t.hologram_card_num,send_data);
         //open tcp client with remote server
         rui_cellular_send("AT+QIOPEN=1,0,\"TCP\",\"cloudsocket.hologram.io\",9999,0,1");
         rui_cellular_response(gsm_rsp, 256, 500 * 2);
         memset(gsm_rsp,0,256);
         rui_cellular_response(gsm_rsp, 256, 500 * 10);       
         //send data
         memset(gsm_cmd,0,100);
         sprintf(gsm_cmd,"AT+QISEND=0,%d",strlen(hologram_cmd));
         rui_cellular_send(gsm_cmd);
         rui_delay_ms(10);
         rui_cellular_send(hologram_cmd);
         memset(gsm_rsp,0,256);
         rui_cellular_response(gsm_rsp, 256, 500 * 30);
         memset(gsm_rsp,0,256);
         rui_cellular_response(gsm_rsp, 256, 500 * 30);
         //close socket
         rui_cellular_send("AT+QICLOSE=0,30000");
         memset(gsm_rsp,0,256);
         rui_cellular_response(gsm_rsp, 256, 500 * 60);
         memset(at_rsp,0,1536);
         sprintf(at_rsp,"%s",cmd);
         rui_at_response(true, at_rsp, RAK_OK);
        return;
     }

    // at+set_config=ble:work_mode:X:Y
    if (strstr(cmd, "ble:work_mode:") != NULL)
    {
        uint8_t work_mode;
        bool long_range_enable;
     	ptr = NULL;
        ptr = strstr(cmd,"work_mode:");
        ptr += 10;

        if (*ptr == '0')
            { work_mode = BLE_MODE_PERIPHERAL; }
        else if (*ptr == '1')
            { work_mode = BLE_MODE_CENTRAL; }
        else if (*ptr == '2')
            { work_mode = BLE_MODE_OBSERVER; }
        else
        {
            memset(at_rsp,0,1536);
            memcpy(at_rsp,cmd,strlen(cmd));
            memcpy(at_rsp+strlen(at_rsp),"ERROR:RUI_AT_PARAMETER_INVALID",strlen("ERROR:RUI_AT_PARAMETER_INVALID"));
            rui_at_response(false, at_rsp, RUI_AT_PARAMETER_INVALID);
            return ;
        }

        #ifdef S140
        ptr += 2;
        if (*ptr == '1')
            { long_range_enable = 1; }
        else if (*ptr == '0')
            { long_range_enable = 0; }
        else
        {
            memset(at_rsp,0,1536);
            memcpy(at_rsp,cmd,strlen(cmd));
            memcpy(at_rsp+strlen(at_rsp),"ERROR:RUI_AT_PARAMETER_INVALID",strlen("ERROR:RUI_AT_PARAMETER_INVALID"));
            rui_at_response(false, at_rsp, RUI_AT_PARAMETER_INVALID);
            return ;
        }
        
        #endif

        #ifdef S132
        ptr += 2;
        if (*ptr == '0')
            long_range_enable = 0;
        else
        {
            memset(at_rsp,0,1536);
            memcpy(at_rsp,cmd,strlen(cmd));
            memcpy(at_rsp+strlen(at_rsp),"ERROR:RUI_AT_PARAMETER_INVALID",strlen("ERROR:RUI_AT_PARAMETER_INVALID"));
            rui_at_response(false, at_rsp, RUI_AT_PARAMETER_INVALID);
            return ;
        }
        #endif
        
        RUI_LOG_PRINTF("g_rui_cfg_t.g_ble_cfg_t.work_mode = %d", work_mode); 
        RUI_LOG_PRINTF("g_rui_cfg_t.g_ble_cfg_t.long_range_enable = %d", long_range_enable); 
        memset(at_rsp,0,1536);
        err_code = rui_ble_set_work_mode(work_mode, long_range_enable);
        if (err_code != 0)
        {
            memcpy(at_rsp,cmd,strlen(cmd));
            memcpy(at_rsp+strlen(at_rsp),"ERROR:RUI_AT_RW_FLASH_ERROR",strlen("ERROR:RUI_AT_RW_FLASH_ERROR"));
            rui_at_response(false, at_rsp, RUI_AT_RW_FLASH_ERROR);
        }
        else
        {
            sprintf(at_rsp,"%s",cmd);
            rui_at_response(true, at_rsp, RAK_OK);   
        }
		return;
    }

    // at+help
    if (strstr(cmd, "at+help") != NULL)
    {
        memset(at_rsp,0,1536);
        memcpy(at_rsp,"at+help\r\n",strlen("at+help\r\n"));
        sprintf(at_rsp+strlen(at_rsp),"%s\r\n",AT_HELP);
        rui_at_response(true, at_rsp, RAK_OK);
		return;
	}
	memset(at_rsp,0,1536);
    //RUI_LOG_PRINTF("Invalid at command!!");
    memcpy(at_rsp,cmd,strlen(cmd));
    memcpy(at_rsp+strlen(at_rsp),"ERROR:RUI_AT_UNSUPPORT",strlen("ERROR:RUI_AT_UNSUPPORT"));
    rui_at_response(false, at_rsp, RUI_AT_UNSUPPORT);
}
