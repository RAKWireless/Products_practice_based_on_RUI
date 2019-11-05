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

extern RUI_LORA_STATUS_T lora_status;
int at_flag = 0;
int power_flag = 0;

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

void uart_put_string(uint8_t *msg)
{
    rui_uart_send(RUI_UART1, msg, strlen(msg));
}

void at_parse(char *cmd)
{
    char  *ptr = NULL;
    uint8_t send_data[256] = {0};
    uint8_t lora_port[5] = {0};
    uint8_t sleep_data[10] = {0};
    uint8_t index = 0;
    uint32_t err_code = 0;
    uint8_t lora_config_data[10] = {0};
    uint8_t msg_flash_success[]="Config success.\r\n";
    uint8_t msg_flash_failed[]="Config failed.\r\n";
    RUI_LOG_PRINTF("at_parse: %s",cmd);

    if(cmd[0] == 0)
    {
        return;
    }

    // at+version
    if(strstr(cmd,"at+version")!= 0)
    {
        char ver[48]="Firmware Version: RUI v";
        strcat(ver, RUI_VERSION);
        RUI_LOG_PRINTF("%s", ver);
        rui_at_response(true, ver, RAK_OK);
        return;
    }

    // at+set_config=device:sleep:1
    if(strstr(cmd,"at+set_config=device:sleep:1")!= 0)
    {
        RUI_LOG_PRINTF("Device wil go to sleep.");
        rui_at_response(true, "Device will go to sleep.", RAK_OK);

        if(power_flag == 0)
        {
            rui_device_sleep(1);
            power_flag =1;
        }
        return;
    }

    // at+set_config=device:sleep:0
    if(strstr(cmd,"at+set_config=device:sleep:0")!= 0)
    {
        rui_device_sleep(0);
        power_flag = 0;
        RUI_LOG_PRINTF(true, NULL, RAK_OK);
        return;
    }

    // at+set_config=device:restart
    if(strstr(cmd,"at+set_config=device:restart")!= NULL)
    {
        uint8_t msg[64];

        sprintf(msg, "%s", "Device will Reset after 3s...\r\n");
        RUI_LOG_PRINTF("%s", msg);
        rui_uart_send(RUI_UART1, msg, strlen(msg));
        delay_ms(1000);

        sprintf(msg, "Device will Reset after 2s...\r\n");
        RUI_LOG_PRINTF("%s", msg);
        rui_uart_send(RUI_UART1, msg, strlen(msg));
        delay_ms(1000);

        sprintf(msg, "Device will Reset after 1s...\r\n");
        RUI_LOG_PRINTF("%s", msg);
        rui_uart_send(RUI_UART1, msg, strlen(msg));
        delay_ms(1000);

        rui_device_reset();
        return;
    }

    // at+get_config=device:status
    if(strstr(cmd,"at+get_config=device:status")!= NULL)
    {
        memset(send_data,0,256);
        rui_at_response(true, send_data, RAK_OK);
        return;
    }

    // at+set_config=lora:dev_eui:XXXX
    if(strstr(cmd,"at+set_config=lora:dev_eui")!= NULL)
    {
        uint8_t dev_eui[8];
        ptr = NULL;
        index = 0;
        ptr = strstr(cmd,"dev_eui");
        for(index; index<8; index++)
        {
            ptr++;
        }
        index = 0;
        memset(g_rui_cfg_t.g_lora_cfg_t.dev_eui,0,8);
        for(ptr; *ptr !='\0'; ptr++)
        {
            memset(lora_config_data,0,10);
            lora_config_data[0] = *ptr;
            ptr++;
            lora_config_data[1] = *ptr;
            StrToHex(&(dev_eui[index]),lora_config_data,2);
            index++;
        }
        err_code = rui_lora_set_dev_eui(dev_eui);
        if (err_code != RUI_STATUS_OK) { rui_at_response(false, msg_flash_failed, WRITE_FLASH_FAIL); }
        else { rui_at_response(true, msg_flash_success, RAK_OK); }
        return;
    }

    // at+set_config=lora:app_eui:XXXX
    if(strstr(cmd,"at+set_config=lora:app_eui")!= NULL)
    {
        uint8_t app_eui[8];
        ptr = NULL;
        index = 0;
        ptr = strstr(cmd,"app_eui");
        for(index; index<8; index++)
        {
            ptr++;
        }
        index = 0;
        memset(g_rui_cfg_t.g_lora_cfg_t.app_eui,0,8);
        for(ptr; *ptr !='\0'; ptr++)
        {
            memset(lora_config_data,0,10);
            lora_config_data[0] = *ptr;
            ptr++;
            lora_config_data[1] = *ptr;
            StrToHex(&(app_eui[index]),lora_config_data,2);
            index++;
        }
        err_code = rui_lora_set_app_eui(app_eui);
        if (err_code != RUI_STATUS_OK) { rui_at_response(false, msg_flash_failed, WRITE_FLASH_FAIL); }
        else { rui_at_response(true, msg_flash_success, RAK_OK); }
        return;
    }

    // at+set_config=lora:app_key:XXXX
    if(strstr(cmd,"at+set_config=lora:app_key")!= NULL)
    {
        uint8_t app_key[16];
        ptr = NULL;
        index = 0;
        ptr = strstr(cmd,"app_key");
        for(index; index<8; index++)
        {
            ptr++;
        }
        index = 0;
        memset(g_rui_cfg_t.g_lora_cfg_t.app_key,0,16);
        for(ptr; *ptr !='\0'; ptr++)
        {
            memset(lora_config_data,0,10);
            lora_config_data[0] = *ptr;
            ptr++;
            lora_config_data[1] = *ptr;
            StrToHex(&(app_key[index]),lora_config_data,2);
            index++;
        }
        err_code = rui_lora_set_app_key(app_key);
        if (err_code != RUI_STATUS_OK) { rui_at_response(false, msg_flash_failed, WRITE_FLASH_FAIL); }
        else { rui_at_response(true, msg_flash_success, RAK_OK); }
        return;
    }

    // at+set_config=lora:dev_addr:XXXX
    if(strstr(cmd,"at+set_config=lora:dev_addr")!= NULL)
    {
        ptr = NULL;
        uint8_t i = 0;
        index = 0;
        memset(lora_config_data,0,10);
        ptr = strstr(cmd,"dev_addr");
        for(index; index<9; index++)
        {
            ptr++;
        }
        index = 0;
        g_rui_cfg_t.g_lora_cfg_t.dev_addr = 0;
        for(ptr; *ptr !='\0'; ptr++)
        {
            lora_config_data[i++] = *ptr;

        }
        err_code = rui_lora_set_dev_addr(lora_config_data);
        if (err_code != RUI_STATUS_OK) { rui_at_response(false, msg_flash_failed, WRITE_FLASH_FAIL); }
        else { rui_at_response(true, msg_flash_success, RAK_OK); }
        return;
    }
    
    // at+set_config=lora:apps_key:XXXX
    if(strstr(cmd,"at+set_config=lora:apps_key")!= NULL)
    {
        uint8_t appskey[16];
        ptr = NULL;
        index = 0;
        ptr = strstr(cmd,"apps_key");
        for(index; index<9; index++)
        {
            ptr++;
        }
        index = 0;
        memset(g_rui_cfg_t.g_lora_cfg_t.appskey,0,16);
        for(ptr; *ptr !='\0'; ptr++)
        {
            memset(lora_config_data,0,10);
            lora_config_data[0] = *ptr;
            ptr++;
            lora_config_data[1] = *ptr;
            StrToHex(&(appskey[index]),lora_config_data,2);
            index++;
        }
        err_code = rui_lora_set_apps_key(appskey);
        if (err_code != RUI_STATUS_OK) { rui_at_response(false, msg_flash_failed, WRITE_FLASH_FAIL); }
        else { rui_at_response(true, msg_flash_success, RAK_OK); }
        return;
    }
    
    // at+set_config=lora:nwkskey:XXXX
    if(strstr(cmd,"at+set_config=lora:nwks_key")!= NULL)
    {
        uint8_t nwkskey[16];
        ptr = NULL;
        index = 0;
        ptr = strstr(cmd,"nwks_key");
        for(index; index<9; index++)
        {
            ptr++;
        }
        index = 0;
        memset(g_rui_cfg_t.g_lora_cfg_t.nwkskey,0,16);
        for(ptr; *ptr !='\0'; ptr++)
        {
            memset(lora_config_data,0,10);
            lora_config_data[0] = *ptr;
            ptr++;
            lora_config_data[1] = *ptr;
            StrToHex(&(nwkskey[index]),lora_config_data,2);
            index++;
        }
        err_code = rui_lora_set_nwks_key(nwkskey);
        if (err_code != RUI_STATUS_OK) { rui_at_response(false, msg_flash_failed, WRITE_FLASH_FAIL); }
        else { rui_at_response(true, msg_flash_success, RAK_OK); }
        return;
    }
    
    // at+set_config=lora:region:XXX
    if(strstr(cmd,"at+set_config=lora:region")!= NULL)
    {
        LORA_REGION region;
        ptr = NULL;
        ptr = strstr(cmd,"region:");
        ptr += 7;

        region = rui_lora_region_convert(ptr);
        err_code = rui_lora_set_region(region);
        if (err_code != RUI_STATUS_OK) { rui_at_response(false, msg_flash_failed, WRITE_FLASH_FAIL); }
        else { rui_at_response(true, msg_flash_success, RAK_OK); }
        return;
    }
    
    // at+set_config=lora:join_mode:XXX
    if(strstr(cmd,"at+set_config=lora:join_mode")!= NULL)
    {
        uint8_t join_mode = RUI_OTAA;
        ptr = NULL;
        index = 0;
        if(strstr(cmd,"join_mode:0")!=NULL)
        {
            join_mode = RUI_OTAA;
        }
        else
        {
            join_mode = RUI_ABP;
        }

        err_code = rui_lora_set_join_mode(join_mode);
        if (err_code != RUI_STATUS_OK) { rui_at_response(false, msg_flash_failed, WRITE_FLASH_FAIL); }
        else { rui_at_response(true, msg_flash_success, RAK_OK); }
        return;
    }
    
    // at+join
    if(strstr(cmd,"at+join")!= NULL)
    {
        rui_lora_join();
        rui_at_response(true, NULL, RAK_OK);
        return;
    }
    
    // at+send=lora:X:YYY
    if(strstr(cmd,"at+send=lora")!= NULL)
    {
        ptr = NULL;
        index = 0;
        memset(lora_port,0,5);
        memset(send_data,0,256);
        ptr = strstr(cmd,"lora:");
        for(index; index<5; index++)
        {
            ptr++;
        }
        index = 0;
        for(ptr; *ptr !=':'; ptr++)
        {
            lora_port[index++] = *ptr;
        }
        ptr++;
        index= 0;
        for(ptr; *ptr !='\0'; ptr++)
        {
            send_data[index++] = *ptr;
        }
        rui_lora_send(atoi(lora_port),send_data,strlen(send_data));
        rui_at_response(true, NULL, RAK_OK);
        return;
    }
    
    // at+set_config=lora:work_mode:X
    if(strstr(cmd,"at+set_config=lora:work_mode")!= NULL)
    {
        uint8_t work_mode = 0;
        ptr = NULL;
        index = 0;
        if(strstr(cmd,"work_mode:0")!=NULL)
        {
            work_mode =0;
        }
        else
        {
            uart_put_string("Parameter is invalid.\r\n");
            return;
        }

        err_code = rui_lora_set_work_mode(work_mode);
        if (err_code != RUI_STATUS_OK) { rui_at_response(false, msg_flash_failed, WRITE_FLASH_FAIL); }
        else { rui_at_response(true, msg_flash_success, RAK_OK); }
        return;
    }
    
    // at+set_config=lora:class:X
    if(strstr(cmd,"at+set_config=lora:class")!= NULL)
    {
        uint8_t class;
        ptr = NULL;
        index = 0;
        ptr = strstr(cmd,"class:");
        for(index; index<6; index++)
        {
            ptr++;
        }
        if(*ptr == '0')
        {
            class = 0;
        }
        else if(*ptr == '1')
        {
            class = 1;
        }
        else if(*ptr == '2')
        {
            class = 2;
        }
        else
        {
            class = 0;
        }
        err_code = rui_lora_set_class(class);
        if (err_code != RUI_STATUS_OK) { rui_at_response(false, msg_flash_failed, WRITE_FLASH_FAIL); }
        else { rui_at_response(true, msg_flash_success, RAK_OK); }
        return;
    }
    
    // at+set_config=lora:confirm:X
    if(strstr(cmd,"at+set_config=lora:confirm")!= NULL)
    {
        bool confirm;
        ptr = NULL;
        index = 0;
        ptr = strstr(cmd,"confirm:");
        for(index; index<8; index++)
        {
            ptr++;
        }
        if(*ptr == '0')
        {
            confirm = false;
        }
        else if(*ptr == '1')
        {
            confirm = true;
        }
        else
        {}
        err_code = rui_lora_set_confirm(confirm);
        if (err_code != RUI_STATUS_OK) { rui_at_response(false, msg_flash_failed, WRITE_FLASH_FAIL); }
        else { rui_at_response(true, msg_flash_success, RAK_OK); }
        return;
    }
    
    // at+set_config=lora:send_interval:X
    if(strstr(cmd,"at+set_config=lora:send_interval")!= NULL)
    {
        RUI_LORA_AUTO_SEND_MODE mode = RUI_AUTO_DISABLE;
        uint32_t sleep_period;
        ptr = NULL;
        index = 0;
        memset(sleep_data,0,10);
        ptr = strstr(cmd,"interval:");
        for(index; index<9; index++)
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
        for(ptr; *ptr !='\0'; ptr++)
        {
            sleep_data[index++] = *ptr;
        }
        if(atoi(sleep_data)<30)
        {
            RUI_LOG_PRINTF("send interval should not be less than 30 s !!!!");
            rui_at_response(false, "send interval should not be less than 30 s.\r\n", RAK_PARAM_ERROR);
            return;
        }
        sleep_period = atoi(sleep_data) * 1000;
        RUI_LOG_PRINTF("g_rui_cfg_t.sleep_enable = %d",mode);
        RUI_LOG_PRINTF("g_rui_cfg_t.sleep_period = %d",sleep_period);
        err_code = rui_lora_set_send_interval(mode, sleep_period);
        if (err_code != RUI_STATUS_OK) { rui_at_response(false, msg_flash_failed, WRITE_FLASH_FAIL); }
        else { rui_at_response(true, msg_flash_success, RAK_OK); }
        return;
    }
    
    // at+get_config=lora:status
    if(strstr(cmd,"at+get_config=lora:status")!= NULL)
    {
        rui_lora_get_status(true, &lora_status);
        rui_at_response(true, NULL, RAK_OK);
        return;
    }
    
    // at+get_config=lora:channel
    if(strstr(cmd,"at+get_config=lora:channel")!= NULL)
    {
        rui_get_channel_list();  // print lora channel list via uart
        rui_at_response(true, NULL, RAK_OK);
        return;
    }

    // at+set_config=lora:ch_mask:X:Y
    if(strstr(cmd,"at+set_config=lora:ch_mask")!= NULL)
    {
        uint8_t channel_num[4] = {0};
        uint8_t status = 0;
        ptr = NULL;
        index = 0;
        memset(channel_num,0,4);
        ptr = strstr(cmd,"ch_mask:");
        for(index; index<8; index++)
        {
            ptr++;
        }
        index = 0;
        for(ptr; *ptr !=':'; ptr++)
        {
            channel_num[index++] = *ptr;
        }
        ptr++;
        if(*ptr == '1')
        {
            status = 1;
        }
        else
        {
            status = 0;
        }

        rui_lora_set_channel_mask(atoi(channel_num),status);
        rui_at_response(true, NULL, RAK_OK);
        return;
    }

    // at+set_config=device:uart_mode:X:Y
    if (strstr(cmd, "at+set_config=device:uart_mode:")!=NULL)
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

    // at+set_config=ble:work_mode:X:Y
    if (strstr(cmd, "at+set_config=ble:work_mode:") != NULL)
    {
        uint8_t work_mode;
        bool long_range_enable;
        ptr = NULL;
        ptr = strstr(cmd,"work_mode:");
        ptr += 10;

        if (*ptr == '0')
        {
            work_mode = BLE_MODE_PERIPHERAL;
        }
        else if (*ptr == '1')
        {
            work_mode = BLE_MODE_CENTRAL;
        }
        else
        {
            work_mode = BLE_MODE_OBSERVER;
        }

        #ifdef S140
            ptr += 2;
            if (*ptr == '1') { long_range_enable = 1; }
            else { long_range_enable = 0; }
        #endif

        #ifdef S132
            long_range_enable = 0;  // nrf52832 does not support long range
        #endif

        RUI_LOG_PRINTF("g_rui_cfg_t.g_ble_cfg_t.work_mode = %d", work_mode);
        RUI_LOG_PRINTF("g_rui_cfg_t.g_ble_cfg_t.long_range_enable = %d", long_range_enable);
        err_code = rui_ble_set_work_mode(work_mode, long_range_enable);
        if (err_code != RUI_STATUS_OK) { rui_at_response(false, msg_flash_failed, WRITE_FLASH_FAIL); }
        else { rui_at_response(true, msg_flash_success, RAK_OK); }
        return;
    }

    // at+help
    if (strstr(cmd, "at+help") != NULL)
    {
        rui_at_response(true, AT_HELP, RAK_OK);
        RUI_LOG_PRINTF(AT_HELP);
        return;
    }

    rui_at_response(false, "Invalid at command!!\r\n", RAK_ERROR);
    RUI_LOG_PRINTF("Invalid at command!!");
    return;
}

