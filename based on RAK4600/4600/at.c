#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
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

#ifndef __RUI_LOG_PRINT_MERGE
#define __RUI_LOG_PRINT_MERGE
#define RUI_LOG_PRINTF_MERGE(fmt, args...);  {uart_log_printf(fmt, ##args);RUI_LOG_PRINTF(fmt, ##args);}
#endif
rui_cfg_t g_rui_cfg_t = {0};

extern RUI_LORA_STATUS_T lora_status;
extern bool IsJoiningflag;  //flag whether joining or not status
extern RUI_RETURN_STATUS rui_return_status;
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

void HexToStr(char *pbDest, char *pbSrc, int nLen)
{
    char ddl,ddh;
    int i;
    
    for (i=0; i<nLen; i++)
    {
        ddh = 48 + pbSrc[i] / 16;
        ddl = 48 + pbSrc[i] % 16;
        if (ddh > 57) ddh = ddh + 7;
        if (ddl > 57) ddl = ddl + 7;
        pbDest[i*2] = ddh;
        pbDest[i*2+1] = ddl;
    }
    
    pbDest[nLen*2] = '\0';
}

void at_response_string(uint8_t *msg)
{
    rui_uart_send(RUI_UART1, msg, strlen(msg));
    RUI_LOG_PRINTF("%s", msg);
}

void at_response_param_invalid(uint8_t *at_rsp)
{
    strcat(at_rsp, "ERROR:RUI_AT_PARAMETER_INVALID");
    rui_at_response(false, at_rsp, RUI_AT_PARAMETER_INVALID); 
}

bool at_check_param_length(uint8_t *p_data, uint16_t len)
{
    uint16_t param_len = strlen(p_data);

    if (param_len < len){
        return false;
    }
    else{
        if ((p_data[len] != '\0') && (p_data[len] != '\r'))
        {
            return false;
        }
    }

    return true;

}

void uart_log_printf(const char *fmt, ...)
{
    char print_buf[512];
    va_list aptr;
    int ret;
 
    va_start (aptr, fmt);
    ret = vsprintf (print_buf, fmt, aptr);
    va_end (aptr);
 
    rui_uart_send(RUI_UART1, print_buf, strlen(print_buf));
}

void at_parse(char *cmd)
{
    char  *ptr = NULL;
    uint8_t send_data[256] = {0};
    uint8_t at_rsp[1536] = {0};
    uint8_t lora_port[5] = {0};
    uint8_t sleep_data[10] = {0};
    uint8_t index = 0;
    uint32_t param_len = 0;
    uint32_t err_code = 0;
    uint8_t lora_config_data[10] = {0};

    if((cmd[0] == 0) || (strlen(cmd)>128))
    {
        return;
    }

    sprintf(at_rsp,"\r\n%s",cmd);

    // at+version
    if(strstr(cmd,"at+version")!= 0)
    {
        char ver[48]="Firmware Version: RUI v";
        strcat(ver, RUI_VERSION);
        sprintf(at_rsp+strlen(at_rsp), "%s\r\n", ver);
        rui_at_response(true, at_rsp, RUI_AT_OK);

        return;
    }

    // at+set_config=device:sleep:1
    if(strstr(cmd,"at+set_config=device:sleep:1")!= 0)
    {
        strcat(at_rsp, "Go to sleep\r\n");
        rui_at_response(true, at_rsp, RUI_AT_OK);
        rui_delay_ms(100);

        rui_device_sleep(1);
        return;
    }

    // at+set_config=device:sleep:0
    if(strstr(cmd,"at+set_config=device:sleep:0")!= 0)
    {
        rui_device_sleep(0);
        power_flag = 0;

        
        strcat(at_rsp, "Wake up.\r\n");
        rui_at_response(true, at_rsp, RUI_AT_OK);
        return;
    }

    // at+set_config=device:restart
    if(strstr(cmd,"at+set_config=device:restart")!= NULL)
    {
        rui_at_response(true, at_rsp, RUI_AT_OK);

        rui_delay_ms(50);
		rui_device_reset();
        return;
    }

    // at+set_config=lora:dev_eui:XXXX
    if(strstr(cmd,"at+set_config=lora:dev_eui:")!= NULL)
    {
        uint8_t dev_eui[8];
        ptr = strstr(cmd,"dev_eui:");
        ptr += 8;

        if (false == at_check_param_length(ptr, 16))
        {
            at_response_param_invalid(at_rsp);
            return ;
        }

        StrToHex(dev_eui, ptr, 16);
        err_code = rui_lora_set_dev_eui(dev_eui);
        if (err_code != RUI_STATUS_OK) {
            strcat(at_rsp, "ERROR:RUI_AT_RW_FLASH_ERROR");
            rui_at_response(false, at_rsp, RUI_AT_RW_FLASH_ERROR); 
        }
        else {
            strcat(at_rsp, "LoRa dev_eui configure success\r\n");
            rui_at_response(true, at_rsp, RUI_AT_OK); 
        }
        return;
    }

    // at+set_config=lora:app_eui:XXXX
    if(strstr(cmd,"at+set_config=lora:app_eui:")!= NULL)
    {
        uint8_t app_eui[8];
        ptr = strstr(cmd,"app_eui:");
        ptr+=8;

        if (false == at_check_param_length(ptr, 16))
        {
            at_response_param_invalid(at_rsp);
            return ;
        }

        StrToHex(app_eui, ptr, 16);
        err_code = rui_lora_set_app_eui(app_eui);
        if (err_code != RUI_STATUS_OK) {
            strcat(at_rsp, "ERROR:RUI_AT_RW_FLASH_ERROR");
            rui_at_response(false, at_rsp, RUI_AT_RW_FLASH_ERROR); 
        }
        else {
            strcat(at_rsp, "LoRa app_eui configure success\r\n");
            rui_at_response(true, at_rsp, RUI_AT_OK); 
        }
        return;
    }

    // at+set_config=lora:app_key:XXXX
    if(strstr(cmd,"at+set_config=lora:app_key:")!= NULL)
    {
        uint8_t app_key[16];
        ptr = strstr(cmd,"app_key:");
        ptr += 8;

        if (false == at_check_param_length(ptr, 32))
        {
            at_response_param_invalid(at_rsp);
            return ;
        }

        StrToHex(app_key, ptr, 32);
        err_code = rui_lora_set_app_key(app_key);
        if (err_code != RUI_STATUS_OK) {
            strcat(at_rsp, "ERROR:RUI_AT_RW_FLASH_ERROR");
            rui_at_response(false, at_rsp, RUI_AT_RW_FLASH_ERROR); 
        }
        else {
            strcat(at_rsp, "LoRa app_key configure success\r\n");
            rui_at_response(true, at_rsp, RUI_AT_OK); 
        }
        return;
    }

    // at+set_config=lora:dev_addr:XXXX
    if(strstr(cmd,"at+set_config=lora:dev_addr:")!= NULL)
    {
        ptr = strstr(cmd,"dev_addr:");
        ptr += 9;

        if (false == at_check_param_length(ptr, 8))
        {
            at_response_param_invalid(at_rsp);
            return ;
        }

        for(index=0; index<8; index++)
        {
            lora_config_data[index] = *ptr;
            ptr++;

        }

        err_code = rui_lora_set_dev_addr(lora_config_data);
        if (err_code != RUI_STATUS_OK) {
            strcat(at_rsp, "ERROR:RUI_AT_RW_FLASH_ERROR");
            rui_at_response(false, at_rsp, RUI_AT_RW_FLASH_ERROR); 
        }
        else { 
            strcat(at_rsp, "LoRa dev_addr configure success\r\n");
            rui_at_response(true, at_rsp, RUI_AT_OK); 
        }
        return;
    }
    
    // at+set_config=lora:apps_key:XXXX
    if(strstr(cmd,"at+set_config=lora:apps_key:")!= NULL)
    {
        uint8_t appskey[16];
        ptr = strstr(cmd,"apps_key:");
        ptr+=9;

        if (false == at_check_param_length(ptr, 32))
        {
            at_response_param_invalid(at_rsp);
            return ;
        }

        StrToHex(appskey, ptr, 32);
        err_code = rui_lora_set_apps_key(appskey);
        if (err_code != RUI_STATUS_OK) {
            strcat(at_rsp, "ERROR:RUI_AT_RW_FLASH_ERROR");
            rui_at_response(false, at_rsp, RUI_AT_RW_FLASH_ERROR); 
        }
        else { 
            strcat(at_rsp, "LoRa apps_key configure success\r\n");
            rui_at_response(true, at_rsp, RUI_AT_OK); 
        }
        return;
    }
    
    // at+set_config=lora:nwkskey:XXXX
    if(strstr(cmd,"at+set_config=lora:nwks_key:")!= NULL)
    {
        uint8_t nwkskey[16];
        ptr = strstr(cmd,"nwks_key:");
        ptr += 9;

        if (false == at_check_param_length(ptr, 32))
        {
            at_response_param_invalid(at_rsp);
            return ;
        }

        StrToHex(nwkskey, ptr, 32);
        err_code = rui_lora_set_nwks_key(nwkskey);
        if (err_code != RUI_STATUS_OK) {
            strcat(at_rsp, "ERROR:RUI_AT_RW_FLASH_ERROR");
            rui_at_response(false, at_rsp, RUI_AT_RW_FLASH_ERROR); 
        }
        else { 
            strcat(at_rsp, "LoRa nwks_key configure success\r\n");
            rui_at_response(true, at_rsp, RUI_AT_OK); 
        }
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
        if (region == 0xFF)
        {
            at_response_param_invalid(at_rsp);
            return ;
        }
        err_code = rui_lora_set_region(region);
        if (err_code != RUI_STATUS_OK) {
            strcat(at_rsp, "ERROR:RUI_LORA_STATUS_REGION_NOT_SUPPORTED");
            rui_at_response(false, at_rsp, RUI_LORA_STATUS_REGION_NOT_SUPPORTED); 
        }
        else { 
            strcat(at_rsp, "Selected LoRaWAN 1.0.2 Region:");
            strcat(at_rsp, ptr);
            rui_at_response(true, at_rsp, RUI_AT_OK); 
        }
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
        else if (strstr(cmd,"join_mode:1")!=NULL)
        {
            join_mode = RUI_ABP;
        }
        else
        {
            at_response_param_invalid(at_rsp);
            return ;
        }

        err_code = rui_lora_set_join_mode(join_mode);
        if (err_code != RUI_STATUS_OK) {
            strcat(at_rsp, "ERROR:RUI_AT_RW_FLASH_ERROR");
            rui_at_response(false, at_rsp, RUI_AT_RW_FLASH_ERROR); 
        }
        else { 
            strcat(at_rsp, "LoRa join mode configure success\r\n");
            rui_at_response(true, at_rsp, RUI_AT_OK); 
        }
        return;
    }
    
    // at+join
    if(strstr(cmd,"at+join")!= NULL)
    {
        if (RUI_OTAA == g_rui_cfg_t.g_lora_cfg_t.join_mode)
        {
            strcat(at_rsp, "OTAA:");
            strcat(at_rsp, "\r\nDevEui:");
            HexToStr(at_rsp+strlen(at_rsp), g_rui_cfg_t.g_lora_cfg_t.dev_eui, 8);
            strcat(at_rsp, "\r\nAppEui:");
            HexToStr(at_rsp+strlen(at_rsp), g_rui_cfg_t.g_lora_cfg_t.app_eui, 8);
            strcat(at_rsp, "\r\nAppKey:");
            HexToStr(at_rsp+strlen(at_rsp), g_rui_cfg_t.g_lora_cfg_t.app_key, 16);
            strcat(at_rsp, "\r\nOTAA Join Start...\r\n");
        }
        else
        {
            strcat(at_rsp, "ABP:");
            strcat(at_rsp, "\r\nDevAddr:");
            sprintf(at_rsp+strlen(at_rsp), "%08X", g_rui_cfg_t.g_lora_cfg_t.dev_addr);
            strcat(at_rsp, "\r\nAppsKey:");
            HexToStr(at_rsp+strlen(at_rsp), g_rui_cfg_t.g_lora_cfg_t.appskey, 16);
            strcat(at_rsp, "\r\nNwksKey:");
            HexToStr(at_rsp+strlen(at_rsp), g_rui_cfg_t.g_lora_cfg_t.nwkskey, 16);
            strcat(at_rsp, "\r\n");
        }
        at_response_string(at_rsp);
        IsJoiningflag = true;
        rui_return_status = rui_lora_join();
        switch(rui_return_status)
        {
            case RUI_STATUS_OK: if (RUI_ABP == g_rui_cfg_t.g_lora_cfg_t.join_mode) LoRaWANJoined_callback(1);
                break;
            case RUI_STATUS_PARAMETER_INVALID:RUI_LOG_PRINTF_MERGE("ERROR: RUI_AT_PARAMETER_INVALID %d\r\n",RUI_AT_PARAMETER_INVALID);
                break;
            default: RUI_LOG_PRINTF_MERGE("ERROR: RUI_AT_UNSUPPORT %d\r\n",RUI_AT_UNSUPPORT);
                break;
        }

        return;
    }
    
    // at+send=lora:X:YYY
    if(strstr(cmd,"at+send=lora")!= NULL)
    {
        ptr = NULL;
        index = 0;
        memset(lora_port, 0, 5);
        memset(send_data, 0, 256);
        ptr = strstr(cmd,"lora:");
        ptr += 5;

        index = 0;
        for(ptr; *ptr !=':'; ptr++)
        {
            lora_port[index++] = *ptr;
        }
        ptr++;

        index = strlen(ptr);
        if ((index%2) != 0)
        {
            at_response_param_invalid(at_rsp);
            return ;
        }
        else
            StrToHex(send_data, ptr, index-2);

        rui_lora_send(atoi(lora_port),send_data, (index-2)/2);
        at_response_string(at_rsp);
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
            at_response_param_invalid(at_rsp);
            return ;
        }

        err_code = rui_lora_set_work_mode(work_mode);
        if (err_code != RUI_STATUS_OK) {
            strcat(at_rsp, "ERROR:RUI_AT_RW_FLASH_ERROR");
            rui_at_response(false, at_rsp, RUI_AT_RW_FLASH_ERROR); 
        }
        else {
            strcat(at_rsp, "LoRa work mode configure success\r\n");
            rui_at_response(true, at_rsp, RUI_AT_OK); 
        }
        return;
    }
    
    // at+set_config=lora:class:X
    if(strstr(cmd,"at+set_config=lora:class")!= NULL)
    {
        uint8_t class;
        ptr = NULL;
        index = 0;
        ptr = strstr(cmd,"class:");
        ptr += 6;

        if(*ptr == '0')
        {
            class = 0;
        }
        else if(*ptr == '2')
        {
            class = 2;
        }
        else
        {
            at_response_param_invalid(at_rsp);
            return ;
        }
        err_code = rui_lora_set_class(class);
        if (err_code != RUI_STATUS_OK) {
            strcat(at_rsp, "ERROR:RUI_AT_RW_FLASH_ERROR");
            rui_at_response(false, at_rsp, RUI_AT_RW_FLASH_ERROR); 
        }
        else {
            strcat(at_rsp, "LoRa class configure success\r\n");
            rui_at_response(true, at_rsp, RUI_AT_OK); 
        }
        return;
    }
    
    // at+set_config=lora:confirm:X
    if(strstr(cmd,"at+set_config=lora:confirm")!= NULL)
    {
        bool confirm;
        ptr = NULL;
        index = 0;
        ptr = strstr(cmd,"confirm:");
        ptr += 8;

        if(*ptr == '0')
        {
            confirm = false;
        }
        else if(*ptr == '1')
        {
            confirm = true;
        }
        else
        {
            at_response_param_invalid(at_rsp);
            return ;
        }
        err_code = rui_lora_set_confirm(confirm);
        if (err_code != RUI_STATUS_OK) {
            strcat(at_rsp, "ERROR:RUI_AT_RW_FLASH_ERROR");
            rui_at_response(false, at_rsp, RUI_AT_RW_FLASH_ERROR); 
        }
        else {
            if (confirm) strcat(at_rsp, "LoRa configure confirm success\r\n");
            else strcat(at_rsp, "LoRa configure unconfirm success\r\n");
            rui_at_response(true, at_rsp, RUI_AT_OK); 
        }
        return;
    }

    // at+set_config=lora:adr:X
    if(strstr(cmd,"at+set_config=lora:adr")!= NULL)
    {
        bool adr;
        ptr = NULL;
        ptr = strstr(cmd,"adr");
        ptr += 4;

        if(*ptr == '0')
            adr = false;
        else if(*ptr == '1')
            adr = true;
        else
        {
            at_response_param_invalid(at_rsp);
            return ;
        }

        err_code = rui_lora_set_adr(adr);
        if (err_code != RUI_STATUS_OK) {
            strcat(at_rsp, "ERROR:RUI_STATUS_PARAMETER_INVALID");
            rui_at_response(false, at_rsp, RUI_STATUS_PARAMETER_INVALID);
        }
        else {
            if (adr) strcat(at_rsp, "LoRa configure adr enable success\r\n");
            else strcat(at_rsp, "LoRa configure adr disable success\r\n");
            rui_at_response(true, at_rsp, RUI_AT_OK);
        }

        return;
    }

    // at+set_config=lora:dr:X
    if(strstr(cmd,"at+set_config=lora:dr")!= NULL)
    {
        uint8_t dr;
        uint8_t dr_str[10]={0};
        ptr = NULL;
        index = 0;
        ptr = strstr(cmd,"dr");
        ptr += 3;

        for(ptr; *ptr !='\r'; ptr++)
        {
            dr_str[index++] = *ptr;
            if (index >= 10) break;
        }
        dr = atoi(dr_str);

        err_code = rui_lora_set_dr(dr);
        if (err_code != RUI_STATUS_OK) {
            strcat(at_rsp, "ERROR:RUI_LORA_STATUS_DATARATE_INVALID");
            rui_at_response(false, at_rsp, RUI_LORA_STATUS_DATARATE_INVALID); 
        }
        else {
            sprintf(at_rsp+strlen(at_rsp), "LoRa configure DR %d success\r\n", dr);
            rui_at_response(true, at_rsp, RUI_AT_OK); 
        }

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
        ptr += 9;

        if (*ptr == '0')
        {
            mode = RUI_AUTO_DISABLE;
        }
        else if (*ptr == '1')
        {
            mode = RUI_AUTO_ENABLE_SLEEP;
        }
        else
        {
            at_response_param_invalid(at_rsp);
            return ;
        }
        index = 0;
        ptr++;
        ptr++;
        for(ptr; *ptr !='\0'; ptr++)
        {
            sleep_data[index++] = *ptr;
        }
        if(atoi(sleep_data)<1)
        {
            RUI_LOG_PRINTF("Send interval should not be less than 1s !!!!");
            strcat(at_rsp, "Send interval should not be less than 1s.\r\n");
            at_response_param_invalid(at_rsp);
            return;
        }
        sleep_period = atoi(sleep_data);
        err_code = rui_lora_set_send_interval(mode, sleep_period);
        if (err_code != RUI_STATUS_OK) {
            strcat(at_rsp, "ERROR:RUI_AT_RW_FLASH_ERROR");
            rui_at_response(false, at_rsp, RUI_AT_RW_FLASH_ERROR); 
        }
        else {
            if (mode == RUI_AUTO_DISABLE) strcat(at_rsp, "LoRa configure send_interval disable success\r\n");
            else strcat(at_rsp, "LoRa configure send_interval sleep success\r\n");
            rui_at_response(true, at_rsp, RUI_AT_OK); 
        }
        rui_delay_ms(50);
        return;
    }
    
    // at+get_config=lora:status
    if(strstr(cmd,"at+get_config=lora:status")!= NULL)
    {
        rui_at_response(true, at_rsp, RUI_AT_OK);
        rui_lora_get_status(true, &lora_status);
        return;
    }
    
    // at+get_config=lora:channel
    if(strstr(cmd,"at+get_config=lora:channel")!= NULL)
    {
        rui_at_response(true, at_rsp, RUI_AT_OK);
        rui_get_channel_list();  // print lora channel list via uart
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
        ptr += 8;

        index = 0;
        for(ptr; *ptr !=':'; ptr++)
        {
            channel_num[index++] = *ptr;
        }
        if (atoi(channel_num) > 71)
        {
            at_response_param_invalid(at_rsp);
            return ;
        }

        ptr++;
        if(*ptr == '1') {status = 1;}
        else if (*ptr == '0') {status = 0;}
        else {
            at_response_param_invalid(at_rsp);
            return ;
        }
        rui_lora_set_channel_mask(atoi(channel_num),status);

        strcat(at_rsp,"LoRa channel mask configure success\r\n");
        rui_at_response(true, at_rsp, RUI_AT_OK);
        return;
    }

    // at+set_config=device:uart_mode:X:Y
    if (strstr(cmd, "at+set_config=device:uart_mode:")!=NULL)
    {
        ptr = NULL;
        ptr = strstr(cmd, "uart_mode:");
        ptr += 10;

        if ((*ptr == '1') && (*(ptr+2) == '1'))
        {
            rui_uart_mode_config(RUI_UART1, RUI_UART_UNVARNISHED);
        }
        else
        {
            at_response_param_invalid(at_rsp);
            return ;
        }

        strcat(at_rsp,"Uart transparent mode configure success\r\n");
        rui_at_response(true, at_rsp, RUI_AT_OK);
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
        else if (*ptr == '2')
        {
            work_mode = BLE_MODE_OBSERVER;
        }
        else
        {
            at_response_param_invalid(at_rsp);
            return ;
        }
        ptr += 2;

        #ifdef S140
            if (*ptr == '1') { long_range_enable = 1; }
            else if (*ptr == '0'){ long_range_enable = 0; }
            else
            {
                at_response_param_invalid(at_rsp);
                return ;
            }
        #endif

        #ifdef S132
            if (*ptr == '0')
                { long_range_enable = 0; }
            else
            {
                at_response_param_invalid(at_rsp);
                return ;
            }
        #endif

        err_code = rui_ble_set_work_mode(work_mode, long_range_enable);
        if (err_code != RUI_STATUS_OK) {
            strcat(at_rsp, "ERROR:RUI_AT_RW_FLASH_ERROR");
            rui_at_response(false, at_rsp, RUI_AT_RW_FLASH_ERROR); 
        }
        else {
            strcat(at_rsp, "BLE work mode configure success\r\n");
            rui_at_response(true, at_rsp, RUI_AT_OK); 
        }
        return;
    }

    // at+help
    if (strstr(cmd, "at+help") != NULL)
    {
        rui_at_response(true, at_rsp, RUI_AT_OK);
        at_response_string(AT_HELP);
        return;
    }

    strcat(at_rsp, "ERROR:RUI_AT_UNSUPPORT");
    rui_at_response(false, at_rsp, RUI_AT_UNSUPPORT);
    return;
}

