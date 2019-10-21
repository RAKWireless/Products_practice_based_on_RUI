#include <stdbool.h>
#include <string.h>
#include "inner.h"
#include "at.h"

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



void at_parse(char *cmd)
{
	char  *ptr = NULL;
    uint32_t err_code = 0;
    uint8_t msg_flash_success[]="Config success.\r\n";
    uint8_t msg_flash_failed[]="Config failed.\r\n";
    RUI_LOG_PRINTF("at_parse: %s",cmd);

	if(cmd[0] == 0)
    {
    	return;
    }    

    // at+version
    if(strstr(cmd,"version")!= 0)
    {
        char ver[48]="Firmware Version: RUI v";
        strcat(ver, RUI_VERSION);
		RUI_LOG_PRINTF("%s", ver);
        rui_at_response(true, ver, RAK_OK);
        return;
    }

    // at+device:sleep:1
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

    // at+device:sleep:0
    if(strstr(cmd,"device:sleep:0")!= 0)
    {
		rui_device_sleep(0);
		power_flag = 0;
        RUI_LOG_PRINTF(true, NULL, RAK_OK);
        return;
    }

    // at+set_config=device:restart
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
#endif 

    // at+help
    if (strstr(cmd, "at+help") != NULL)
    {
	    rui_at_response(true, AT_HELP, RAK_OK);
		RUI_LOG_PRINTF(AT_HELP);
		return;
	}

    rui_at_response(false, "Invalid at command!!\r\n", RAK_ERROR);
    RUI_LOG_PRINTF("Invalid at command!!");
}
