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
       strstr(cmd,"at+send=hologram:sensor")==NULL && strstr(cmd,"at+send=hologram:user:")==NULL && strstr(cmd, "at+set_config=uart:work_mode:")==NULL && 
	   strstr(cmd, "at+set_config=device:uart_mode:X:Y")==NULL && strstr(cmd,"at+help")==NULL)
    {
        RUI_LOG_PRINTF("Invalid at command!!");
        rui_at_response(false, "Invalid at command.\r\n", RAK_ERROR);
        return;
    }  
    if(strstr(cmd,"version")!= 0)
    {
        char ver[48]="Firmware Version: RUI v";
        strcat(ver, RUI_VERSION);
		RUI_LOG_PRINTF("%s", ver);
        rui_at_response(true, ver, RAK_OK);
        return;
    }
    if(strstr(cmd,"device:sleep:1")!= 0)
    {
		RUI_LOG_PRINTF("Device has been sleep!");
        rui_at_response(true, NULL, RAK_OK);

		if(power_flag == 0)
		{
			rui_device_sleep(1);
			power_flag =1;
		}
        return;
    }
    if(strstr(cmd,"device:sleep:0")!= 0)
    {
		rui_device_sleep(0);
		power_flag = 0;
        RUI_LOG_PRINTF(true, NULL, RAK_OK);
        return;
    }
    if(strstr(cmd,"device:gps:0")!= 0)
    {
        //bg96 inner gps will automatically off when not use
        rui_at_response(true, NULL, RAK_OK);
        return;
    }
    if(strstr(cmd,"device:gps:1")!= 0)
    {
        //bg96 inner gps will automatically off when not use
        rui_at_response(true, NULL, RAK_OK);
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
			RUI_LOG_PRINTF("send interval should not be less than 150000 !!!!"); 
			return;
		}
        g_rui_cfg_t.sleep_period = atoi(sleep_data);

        RUI_LOG_PRINTF("g_rui_cfg_t.sleep_enable = %d",g_rui_cfg_t.sleep_enable); 
        RUI_LOG_PRINTF("g_rui_cfg_t.sleep_period = %d",g_rui_cfg_t.sleep_period);
        err_code = rui_flash_write(RUI_FLASH_ORIGIN,NULL,0);
        if (err_code != 0){rui_at_response(false, NULL, WRITE_FLASH_FAIL);}
        else{rui_at_response(true, NULL, RAK_OK);}
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
    	delay_ms(1000);
        
    	sprintf(msg, "Device will Reset after 2s...\r\n");
        RUI_LOG_PRINTF("%s", msg);
        #ifdef USER_UART
        rui_uart_send(RUI_UART1, msg, strlen(msg));
        #endif
   	 	delay_ms(1000);    

        sprintf(msg, "Device will Reset after 1s...\r\n");
        RUI_LOG_PRINTF("%s", msg);
        #ifdef USER_UART
        rui_uart_send(RUI_UART1, msg, strlen(msg));
        #endif
    	delay_ms(1000);
        
		rui_device_reset();
        return;
    }
    if(strstr(cmd,"device:status")!= NULL)
    {
      
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
		
    memset(send_data,0,256); 
    sprintf(send_data,"Acc:%.2f,%.2f,%.2f; ",x,y,z);
    sprintf(send_data+strlen(send_data),"Lat(0-N,1-S):%d,%s,Lon(0-E,1-W):%d,%s; ",g_gps_data.LatitudeNS,lat_data,g_gps_data.LongitudaEW,lon_data); 


        rui_at_response(true, send_data, RAK_OK);
        return;
    }
    //at+set_config=device:cellular:0
    if(strstr(cmd,"device:cellular:0")!= NULL)
    {
        rui_cellular_mode(0);
        rui_at_response(true, NULL, RAK_OK);
        return;
	}
    //at+set_config=device:cellular:1
    if(strstr(cmd,"device:cellular:1")!= NULL)
    {
        rui_cellular_mode(1);
        rui_at_response(true, NULL, RAK_OK);
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
		memset(gsm_cmd,0,100);
        memset(gsm_rsp,0,256);
        index = 0;
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
        rui_cellular_send(gsm_cmd);
        rui_cellular_response(gsm_rsp, 256, 500 * 60);
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
		 delay_ms(1000);
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
        if (err_code != 0){rui_at_response(false, NULL, WRITE_FLASH_FAIL);}
        else{rui_at_response(true, NULL, RAK_OK);}
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
        if (err_code != 0){rui_at_response(false, NULL, WRITE_FLASH_FAIL);}
        else{rui_at_response(true, NULL, RAK_OK);}
        return;
     }
     //at+send=hologram:sensor
     if(strstr(cmd,"at+send=hologram:sensor")!= NULL)
     {
        get_lis3dh_data(&x,&y,&z);
        x =x * 4000/65536;
        y =y * 4000/65536;
        z =z * 4000/65536;   

        memset(lat_data,0,20);        
        rui_gps_get(&g_gps_data);
        sprintf(lat_data,"%lf",g_gps_data.Latitude);
        memset(lon_data,0,20);
        sprintf(lon_data,"%lf",g_gps_data.Longitude);
         memset(gsm_cmd,0,100);
         memset(gsm_rsp,0,256);
         memset(send_data,0,256);   
         memset(hologram_cmd,0,256); 

    sprintf(send_data,"Acc:%.2f,%.2f,%.2f; ",x,y,z);

    sprintf(send_data+strlen(send_data),"Lat(0-N,1-S):%d,%s,Lon(0-E,1-W):%d,%s; ",g_gps_data.LatitudeNS,lat_data,g_gps_data.LongitudaEW,lon_data); 


         hologram_cmd_packet(g_rui_cfg_t.g_cellular_cfg_t.hologram_card_num,send_data);
         //open tcp client with remote server
         rui_cellular_send("AT+QIOPEN=1,0,\"TCP\",\"cloudsocket.hologram.io\",9999,0,1");
         rui_cellular_response(gsm_rsp, 256, 500 * 60);
         memset(gsm_rsp,0,256);
         rui_cellular_response(gsm_rsp, 256, 500 * 20);         
         //send data
         memset(gsm_cmd,0,100);
         sprintf(gsm_cmd,"AT+QISEND=0,%d",strlen(hologram_cmd));
         RUI_LOG_PRINTF("gsm_cmd: %s",gsm_cmd);
         rui_cellular_send(gsm_cmd);
         delay_ms(1000);
         rui_cellular_send(hologram_cmd);
         memset(gsm_rsp,0,256);
         rui_cellular_response(gsm_rsp, 256, 500 * 60);
         memset(gsm_rsp,0,256);
         rui_cellular_response(gsm_rsp, 256, 500 * 80);
         //close socket
         rui_cellular_send("AT+QICLOSE=0,30000");
         memset(gsm_rsp,0,256);
         rui_cellular_response(gsm_rsp, 256, 500 * 60);
         rui_at_response(true, NULL, RAK_OK);
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
         for(ptr;*ptr !='\0';ptr++)
         {
            send_data[index++] = *ptr;
         }
         hologram_cmd_packet(g_rui_cfg_t.g_cellular_cfg_t.hologram_card_num,send_data);
         //open tcp client with remote server
         rui_cellular_send("AT+QIOPEN=1,0,\"TCP\",\"cloudsocket.hologram.io\",9999,0,1");
         rui_cellular_response(gsm_rsp, 256, 500 * 60);
         memset(gsm_rsp,0,256);
         rui_cellular_response(gsm_rsp, 256, 500 * 20);         
         //send data
         memset(gsm_cmd,0,100);
         sprintf(gsm_cmd,"AT+QISEND=0,%d",strlen(hologram_cmd));
         RUI_LOG_PRINTF("gsm_cmd: %s",gsm_cmd);
         rui_cellular_send(gsm_cmd);
         delay_ms(1000);
         rui_cellular_send(hologram_cmd);
         memset(gsm_rsp,0,256);
         rui_cellular_response(gsm_rsp, 256, 500 * 60);
         memset(gsm_rsp,0,256);
         rui_cellular_response(gsm_rsp, 256, 500 * 80);
         //close socket
         rui_cellular_send("AT+QICLOSE=0,30000");
         memset(gsm_rsp,0,256);
         rui_cellular_response(gsm_rsp, 256, 500 * 60);
         rui_at_response(true, NULL, RAK_OK);
        return;
     }

    
     //at+set_config=lora:dev_eui:XXXX
     if(strstr(cmd,"lora:dev_eui")!= NULL)
     {
         uint8_t dev_eui[8];
		 ptr = NULL;      
         index = 0;
         ptr = strstr(cmd,"dev_eui");
         for(index;index<8;index++)
         {
            ptr++;
         }
         index = 0;
         memset(g_rui_cfg_t.g_lora_cfg_t.dev_eui,0,8);
         for(ptr;*ptr !='\0';ptr++)
         {
            memset(lora_config_data,0,10);
			lora_config_data[0] = *ptr;
            ptr++;
            lora_config_data[1] = *ptr;
            StrToHex(&(dev_eui[index]),lora_config_data,2); 
            index++;
         }
         err_code = rui_lora_set_dev_eui(dev_eui);
         if (err_code != 0){rui_at_response(false, msg_flash_failed, WRITE_FLASH_FAIL);}
         else{rui_at_response(true, msg_flash_success, RAK_OK);}
         return;
     }
     //at+set_config=lora:app_eui:XXXX
     if(strstr(cmd,"lora:app_eui")!= NULL)
     {
        uint8_t app_eui[8];
		 ptr = NULL;      
         index = 0;
         ptr = strstr(cmd,"app_eui");
         for(index;index<8;index++)
         {
            ptr++;
         }
         index = 0;
         memset(g_rui_cfg_t.g_lora_cfg_t.app_eui,0,8);
         for(ptr;*ptr !='\0';ptr++)
         {
             memset(lora_config_data,0,10);
             lora_config_data[0] = *ptr;
             ptr++;
             lora_config_data[1] = *ptr;
             StrToHex(&(app_eui[index]),lora_config_data,2); 
             index++;
         }
        err_code = rui_lora_set_app_eui(app_eui);
        if (err_code != 0){rui_at_response(false, msg_flash_failed, WRITE_FLASH_FAIL);}
        else{rui_at_response(true, msg_flash_success, RAK_OK);}
        return;
     }
     //at+set_config=lora:app_key:XXXX
     if(strstr(cmd,"lora:app_key")!= NULL)
     {
        uint8_t app_key[16];
		 ptr = NULL;      
         index = 0;
         ptr = strstr(cmd,"app_key");
         for(index;index<8;index++)
         {
            ptr++;
         }
         index = 0;
         memset(g_rui_cfg_t.g_lora_cfg_t.app_key,0,16);
         for(ptr;*ptr !='\0';ptr++)
         {
             memset(lora_config_data,0,10);
             lora_config_data[0] = *ptr;
             ptr++;
             lora_config_data[1] = *ptr;
             StrToHex(&(app_key[index]),lora_config_data,2); 
             index++;
         }
         err_code = rui_lora_set_app_key(app_key);
         if (err_code != 0){rui_at_response(false, msg_flash_failed, WRITE_FLASH_FAIL);}
         else{rui_at_response(true, msg_flash_success, RAK_OK);}
         return;
     }
     //at+set_config=lora:dev_addr:XXXX
     if(strstr(cmd,"lora:dev_addr")!= NULL)
     {
		 ptr = NULL;
         uint8_t i = 0;
         index = 0;
         memset(lora_config_data,0,10);
         ptr = strstr(cmd,"dev_addr");
         for(index;index<9;index++)
         {
            ptr++;
         }
         index = 0;
         g_rui_cfg_t.g_lora_cfg_t.dev_addr = 0;
         for(ptr;*ptr !='\0';ptr++)
         {
             lora_config_data[i++] = *ptr;

         }
         err_code = rui_lora_set_dev_addr(lora_config_data);
         if (err_code != 0){rui_at_response(false, msg_flash_failed, WRITE_FLASH_FAIL);}
         else{rui_at_response(true, msg_flash_success, RAK_OK);}
         return;
     }
     //at+set_config=lora:apps_key:XXXX
     if(strstr(cmd,"lora:apps_key")!= NULL)
     {
        uint8_t appskey[16];
		 ptr = NULL;      
         index = 0;
         ptr = strstr(cmd,"apps_key");
         for(index;index<9;index++)
         {
            ptr++;
         }
         index = 0;
         memset(g_rui_cfg_t.g_lora_cfg_t.appskey,0,16);
         for(ptr;*ptr !='\0';ptr++)
         {
             memset(lora_config_data,0,10);
             lora_config_data[0] = *ptr;
             ptr++;
             lora_config_data[1] = *ptr;
             StrToHex(&(appskey[index]),lora_config_data,2); 
             index++;
         }
         err_code = rui_lora_set_apps_key(appskey);
         if (err_code != 0){rui_at_response(false, msg_flash_failed, WRITE_FLASH_FAIL);}
         else{rui_at_response(true, msg_flash_success, RAK_OK);}
         return;
     }
     //at+set_config=lora:nwkskey:XXXX
     if(strstr(cmd,"lora:nwks_key")!= NULL)
     {
         uint8_t nwkskey[16];
		 ptr = NULL;      
         index = 0;
         ptr = strstr(cmd,"nwks_key");
         for(index;index<9;index++)
         {
            ptr++;
         }
         index = 0;
         memset(g_rui_cfg_t.g_lora_cfg_t.nwkskey,0,16);
         for(ptr;*ptr !='\0';ptr++)
         {
             memset(lora_config_data,0,10);
             lora_config_data[0] = *ptr;
             ptr++;
             lora_config_data[1] = *ptr;
             StrToHex(&(nwkskey[index]),lora_config_data,2); 
             index++;
         }
         err_code = rui_lora_set_nwks_key(nwkskey);
         if (err_code != 0){rui_at_response(false, msg_flash_failed, WRITE_FLASH_FAIL);}
         else{rui_at_response(true, msg_flash_success, RAK_OK);}
         return;
     }
	 //at+set_config=lora:region:XXX
     if(strstr(cmd,"lora:region")!= NULL)
     {
         LORA_REGION region;
		 ptr = NULL;      
         ptr = strstr(cmd,"region:");
         ptr += 7;

         region = rui_lora_region_convert(ptr);
         err_code = rui_lora_set_region(region);
         if (err_code != 0){rui_at_response(false, msg_flash_failed, WRITE_FLASH_FAIL);}
         else{rui_at_response(true, msg_flash_success, RAK_OK);}
         return;
     }
     //at+set_config=lora:join_mode:XXX
     if(strstr(cmd,"lora:join_mode")!= NULL)
     {
         uint8_t join_mode = RUI_OTAA;
		 ptr = NULL;      
         index = 0;
         if(strstr(cmd,"join_mode:0")!=NULL)
            { join_mode = RUI_OTAA;}
         else
            { join_mode = RUI_ABP;}

         err_code = rui_lora_set_join_mode(join_mode);
         if (err_code != 0){rui_at_response(false, msg_flash_failed, WRITE_FLASH_FAIL);}
         else{rui_at_response(true, msg_flash_success, RAK_OK);}
         return;
     }
     //at+join
     if(strstr(cmd,"at+join")!= NULL)
     {
         rui_lora_join();
         rui_at_response(true, NULL, RAK_OK);
         return;
     }	
     //at+send=lora:X:YYY
     if(strstr(cmd,"at+send=lora")!= NULL)
     {
         ptr = NULL;      
         index = 0;
         memset(lora_port,0,5);
         memset(send_data,0,256);
         ptr = strstr(cmd,"lora:");
         for(index;index<5;index++)
         {
            ptr++;
         }
         index = 0;
         for(ptr;*ptr !=':';ptr++)
         {
			lora_port[index++] = *ptr;
         }
         ptr++;
         index= 0;
         for(ptr;*ptr !='\0';ptr++)
         {
			send_data[index++] = *ptr;
         }
         rui_lora_send(atoi(lora_port),send_data,strlen(send_data));
         rui_at_response(true, NULL, RAK_OK);
         return;
     }
     //at+set_config=lora:work_mode:X
     if(strstr(cmd,"lora:work_mode")!= NULL)
     {
         uint8_t work_mode = 0;
         ptr = NULL;      
         index = 0;
         if(strstr(cmd,"work_mode:0")!=NULL)
            { work_mode =0;}
         else if(strstr(cmd,"work_mode:1")!=NULL)
            { work_mode =1;}
         else if(strstr(cmd,"work_mode:2")!=NULL)
            { work_mode =2;}
         else
            {}

         err_code = rui_lora_set_work_mode(work_mode);
         if (err_code != 0){rui_at_response(false, msg_flash_failed, WRITE_FLASH_FAIL);}
         else{rui_at_response(true, msg_flash_success, RAK_OK);}
         return;
     }	
     //at+set_config=lora:class:X
     if(strstr(cmd,"lora:class")!= NULL)
     {
        uint8_t class;
     	ptr = NULL;
        index = 0;
        ptr = strstr(cmd,"class:");
        for(index;index<6;index++)
        {
           ptr++;
        }
        if(*ptr == '0')
            {class = 0;}
        else if(*ptr == '1')
            {class = 1;}
        else if(*ptr == '2')
            {class = 2;}
        else
            {class = 0;}
        err_code = rui_lora_set_class(class);
        if (err_code != 0){rui_at_response(false, msg_flash_failed, WRITE_FLASH_FAIL);}
        else{rui_at_response(true, msg_flash_success, RAK_OK);}
        return;
     }	
     //at+set_config=lora:confirm:
     if(strstr(cmd,"lora:confirm")!= NULL)
     {
        bool confirm;
        ptr = NULL;
        index = 0;
        ptr = strstr(cmd,"confirm:");
        for(index;index<8;index++)
        {
           ptr++;
        }
        if(*ptr == '0')
            {confirm = false;}
        else if(*ptr == '1')
            {confirm = true;}
        else
            {}
        err_code = rui_lora_set_confirm(confirm);
        if (err_code != 0){rui_at_response(false, msg_flash_failed, WRITE_FLASH_FAIL);}
        else{rui_at_response(true, msg_flash_success, RAK_OK);}
        return;
     }
     //at+set_config=lora:send_interval:X
     if(strstr(cmd,"lora:send_interval")!= NULL)
     {
        RUI_LORA_AUTO_SEND_MODE mode = RUI_AUTO_DISABLE;
        uint32_t sleep_period;
        ptr = NULL;
        index = 0;
        memset(sleep_data,0,10);
        ptr = strstr(cmd,"interval:");
        for(index;index<9;index++)
        {
           ptr++;
        }
        if (*ptr == '0')
        {
            mode = RUI_AUTO_DISABLE;
        }
        if (*ptr == '1')
        {
            mode = RUI_AUTO_ENABLE_SLEEP;
        }
        if (*ptr == '2')
        {
            mode = RUI_AUTO_ENABLE_SLEEP;
        }
        index = 0;
        ptr++;
        ptr++;
        for(ptr;*ptr !='\0';ptr++)
        {
            sleep_data[index++] = *ptr;
        }
		if(atoi(sleep_data)<30)
		{
			RUI_LOG_PRINTF("send interval should not be less than 30 s !!!!"); 
			return;
		}
        sleep_period = atoi(sleep_data) * 1000;
        RUI_LOG_PRINTF("g_rui_cfg_t.sleep_enable = %d",mode); 
        RUI_LOG_PRINTF("g_rui_cfg_t.sleep_period = %d",sleep_period);
        err_code = rui_lora_set_send_interval(mode, sleep_period);
        if (err_code != 0){rui_at_response(false, msg_flash_failed, WRITE_FLASH_FAIL);}
        else{rui_at_response(true, msg_flash_success, RAK_OK);}
        return;
     }
     //at+get_config=lora:status
     if(strstr(cmd,"lora:status")!= NULL)
     {
         rui_lora_get_status(&lora_status);
         rui_at_response(true, NULL, RAK_OK);
         return;
     }
     //at+get_config=lora:channel
     if(strstr(cmd,"lora:channel")!= NULL)
     {
     	 rui_get_channel_list();  // print lora channel list via uart
         rui_at_response(true, NULL, RAK_OK);
         return;
     }

     //at+set_config=lora:ch_mask:X:Y
     if(strstr(cmd,"lora:ch_mask")!= NULL)
     {	
     	uint8_t channel_num[4] = {0};
        uint8_t status = 0;
     	ptr = NULL;
        index = 0;
        memset(channel_num,0,4);
        ptr = strstr(cmd,"ch_mask:");
        for(index;index<8;index++)
        {
           ptr++;
        }
        index = 0;
        for(ptr;*ptr !=':';ptr++)
        {
            channel_num[index++] = *ptr;
        }
        ptr++;
        if(*ptr == '1')
            {status = 1;}
        else
            {status = 0;}
        
     	rui_lora_set_channel_mask(atoi(channel_num),status);
        rui_at_response(true, NULL, RAK_OK);
        return;
     }

     // at+set_config=uart:work_mode:x
    if (strstr(cmd, "uart:work_mode:")!=NULL)
    {
        ptr = NULL;
        ptr = strstr(cmd, "work_mode:");
        ptr += 10;

        
        if (at_flag & AT_BLE_MASK)
        {
            if (*ptr == '1')
                {rui_uart_pin_mode_change(true);}
            else
                {rui_uart_pin_mode_change(false);}
            rui_at_response(true, "Uart mode set succes.", RAK_OK);
        }
        else
        {
            rui_at_response(false, "This command only supports BLE.\r\n", RAK_ERROR);
        }

        return ;
    }
    // at+set_config=device:uart_mode:X:Y
    if (strstr(cmd, "device:uart_mode:")!=NULL)
    {
        ptr = NULL;
        ptr = strstr(cmd, "uart_mode:");
        ptr += 10;

        if (*ptr == '1')
        {
            ptr += 2;
            if (*ptr == '1')
            {
                rui_uart_mode_config(RUI_UART1, RUI_UART_UNVARNISHED);
            }
        }

        rui_at_response(true, "Uart config success.\r\n", RAK_OK);
        return ;
    }
#endif

#ifdef BLE_CENTRAL_SUPPORT
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
        else
            { work_mode = BLE_MODE_OBSERVER; }

        #ifdef S140
        ptr += 2;
        if (*ptr == '1')
            { long_range_enable = 1; }
        else
            { long_range_enable = 0; }
        #endif

        #ifdef S132
            long_range_enable = 0;
        #endif
        
        RUI_LOG_PRINTF("g_rui_cfg_t.g_ble_cfg_t.work_mode = %d", work_mode); 
        RUI_LOG_PRINTF("g_rui_cfg_t.g_ble_cfg_t.long_range_enable = %d", long_range_enable); 
        err_code = rui_ble_set_work_mode(work_mode, long_range_enable);
        if (err_code != 0){rui_at_response(false, msg_flash_failed, WRITE_FLASH_FAIL);}
        else{rui_at_response(true, msg_flash_success, RAK_OK);}
		return;
    }
#endif // BLE_CENTRAL_SUPPORT
    // at+help
    if (strstr(cmd, "at+help") != NULL)
    {
	    rui_at_response(true, AT_HELP, RAK_OK);
		RUI_LOG_PRINTF(AT_HELP);
		return;
	}
	rui_at_response(false, "Not Support Command!", RAK_ERROR);
	RUI_LOG_PRINTF("Not Support Command!!");
}
