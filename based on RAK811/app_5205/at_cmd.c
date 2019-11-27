#include "at_cmd.h"
#include "rui.h"

static RUI_RETURN_STATUS rui_return_status;
struct cli_cmd cli_cmds[] = 
{
    "version",        lora_version,         
    "join",           lora_join,
    "get_config",     lora_read_config,
    "set_config",     lora_write_config,
    "send",           lora_send, 
    "help",           atcmd_help,  
};

static int parse_args(char* str, char* argv[])
{
    int i = 0;
    char* ch = str;

    while(*ch != '\0') {
        i++;
        /*Check if length exceeds*/ 
        if (i > MAX_ARGV) {
            return 0;
        }

        argv[i-1] = ch;
        
        while(*ch != ',' && *ch != '\0' && *ch != '\r') {
            if (*ch == ':') {
                return i;  
            } 
            
            if(*ch == '=' && i == 1) {
                break;
            }
            else
                ch++;
        }
        if (*ch == '\r')
            break;
        if (*ch != '\0') {
            *ch = '\0';
            ch++;
            while(*ch == ':') {
                ch++;
            }
        }
    }
    return i;
}
char* str0;
int at_cmd_process(char *str)
{
    int i;
    int argc;    
    char* argv[MAX_ARGV]={NULL};

    str0=str;

    RUI_LOG_PRINTF("%s\r\n",str);
		
    if ((strncmp(str0, "at+", 3) != 0) || str0[3] == '\0') {
        RUI_LOG_PRINTF("ERROR:RUI_AT_UNSUPPORT %d\r\n",RUI_AT_UNSUPPORT);
        return FAIL;
    }
    str0 += 3;
    argc = parse_args(str0, argv);
    if (argc > 0) 
    {
        for (i = 0; i < sizeof(cli_cmds)/sizeof(struct cli_cmd); i++) {
            if (strcmp(argv[0], cli_cmds[i].name) == 0) {
                cli_cmds[i].function(argc, argv);
                break;
            }        
        }
        if (i == sizeof(cli_cmds)/sizeof(struct cli_cmd)) {
            RUI_LOG_PRINTF("ERROR:RUI_AT_UNSUPPORT %d\r\n",RUI_AT_UNSUPPORT);
        }
    }
    else 
    {
        RUI_LOG_PRINTF("ERROR:RUI_AT_UNSUPPORT %d\r\n",RUI_AT_UNSUPPORT);
        return FAIL;
    }

    return SUCCESS;
}

int check_hex_invaild(uint8_t *data, uint16_t len) 
{
    uint8_t check1 = 0xff, check2 = 0;
    
    for (int i = 0; i < len; i++) {
        check1 &= data[i];
        check2 |= data[i];
    }
    if (check1 == 0xff || check2 == 0) {
        return FAIL;
    }
    return SUCCESS;
}


static void out_hex_buf( char *buffer, uint16_t size)
{
     char hex_str[3] = {0};
  
    for (int i = 0; i < size; i++) {
        sprintf(hex_str, "%02x", buffer[i]);
        RUI_LOG_PRINTF("%s", hex_str); 
    }
}

void dump_hex2string(uint8_t *buf , uint8_t len)
{
  for(uint8_t i=0; i<len; i++) {
     RUI_LOG_PRINTF("%02X", buf[i]);
  }
   RUI_LOG_PRINTF("\r\n");
}

bool IsJoiningflag;  //Flag whether is joining LoRaWAN
static void lora_join(int argc, char *argv[])
{
    RUI_LORA_STATUS_T app_lora_status;
    rui_lora_get_status(false,&app_lora_status);     
    if(argv[1] == NULL)
    {
        if(app_lora_status.join_mode == RUI_OTAA)
        {
            RUI_LOG_PRINTF("OTAA:\r\n");
			RUI_LOG_PRINTF("DevEui:");
            dump_hex2string(app_lora_status.dev_eui, 8);
			RUI_LOG_PRINTF("AppEui:");
			dump_hex2string(app_lora_status.app_eui , 8);
			RUI_LOG_PRINTF("AppKey:");
			dump_hex2string(app_lora_status.app_key, 16);
            RUI_LOG_PRINTF("OTAA Join Start... \r\n"); 
            IsJoiningflag = true;            
            rui_return_status = rui_lora_join();
            switch(rui_return_status)
            {
                case RUI_STATUS_OK:break ;
                case RUI_LORA_STATUS_PARAMETER_INVALID:RUI_LOG_PRINTF("ERROR: RUI_AT_LORA_PARAMETER_INVALID %d\r\n",RUI_AT_LORA_PARAMETER_INVALID);
                    break;
                default: RUI_LOG_PRINTF("ERROR: RUI_RETURN_STATUS %d\r\n",rui_return_status);
                    break;
            } 
        }
        else if(app_lora_status.join_mode == RUI_ABP)
        {
            RUI_LOG_PRINTF("ABP: \r\n");
			RUI_LOG_PRINTF("DevAddr: %08X\r\n", app_lora_status.dev_addr);
			RUI_LOG_PRINTF("AppsKey: ");
			dump_hex2string(app_lora_status.apps_key , 16);    
			RUI_LOG_PRINTF("NwksKey: ");
			dump_hex2string(app_lora_status.nwks_key , 16);
            rui_return_status = rui_lora_join();
            switch(rui_return_status)
            {
                case RUI_STATUS_OK:LoRaWANJoined_callback(1);break ;
                case RUI_STATUS_PARAMETER_INVALID:RUI_LOG_PRINTF("ERROR: RUI_AT_PARAMETER_INVALID %d\r\n",RUI_AT_PARAMETER_INVALID);
                    break;
                default: RUI_LOG_PRINTF("ERROR: RUI_RETURN_STATUS %d\r\n",rui_return_status);
                    break;
            }           
        }else 
        {
            RUI_LOG_PRINTF("ERROR: RUI_AT_UNSUPPORT %d\r\n",RUI_AT_UNSUPPORT);
            return ;
        }
    }
    else
    {
        RUI_LOG_PRINTF("ERROR: RUI_AT_UNSUPPORT %d\r\n",RUI_AT_UNSUPPORT);
        return ;
    }    
    
}

static void lora_version(int argc, char *argv[])
{
    uint8_t version[20];
    rui_device_version(version);
    RUI_LOG_PRINTF("Firmware Version: RUI v%s\r\nOK\r\n", version);
}

LORA_REGION rw_String2Region(char* region)
{
  if ( 0==strcmp(region, "AS923")) {
     return AS923;
  } else if (0==strcmp(region, "AU915")) {
     return AU915;
  }else if (0==strcmp(region, "CN470")) {
     return CN470;
  }else if (0==strcmp(region, "CN779")) {
     return CN779;
  }else if (0==strcmp(region, "EU433")) {
     return EU433;
  }else if (0==strcmp(region, "EU868")) {
     return EU868;
  }else if (0==strcmp(region, "KR920")) {
     return KR920;
  }else if (0==strcmp(region, "IN865")) {
     return IN865;
  }else if (0==strcmp(region, "US915")) {
     return US915;
  }else if (0==strcmp(region, "US915_H")) {
     return US915_Hybrid;
  }else {
     return 100;
  }
}


static void lora_read_config(int argc, char *argv[])
{
    if (argc != 2) 
    {
        RUI_LOG_PRINTF("ERROR: RUI_AT_PARAMETER_INVALID %d\r\n",RUI_AT_PARAMETER_INVALID);
        return;
    }
    
    read_config(argv[1]);
}

static void lora_write_config(int argc, char *argv[])
{
    if (argc < 2) {
        RUI_LOG_PRINTF("ERROR: RUI_AT_PARAMETER_INVALID %d\r\n",RUI_AT_PARAMETER_INVALID);
        return;
    }    
    write_config(argv[1]);   
}
uint8_t* send_data;
static void lora_send(int argc, char *argv[])
{
    int i = 0;
    char* ch = argv[1];
    int app_len;
    char hex_num[3] = {0}; 
    RUI_LORA_STATUS_T app_lora_status;
    rui_lora_get_status(false,&app_lora_status);   
    
    if (argc != 2) {
        RUI_LOG_PRINTF("ERROR: RUI_AT_PARAMETER_INVALID %d\r\n",RUI_AT_PARAMETER_INVALID);
        return;
    } 
    while(*ch != '\0' && *ch != '\r') 
    {            
        if(*ch == ':') 
        {
            i++; 
            *ch = '\0';           
            ch++;
            if(i==1)
            {
                argv[2] = ch;
            }
            if(i==2)
            {                
                argv[3] = ch;
            }
        }
        else
            ch++;
    }


    if(strcmp(argv[1],"uart")==0)
    {
        if(i!=2)
        {
            RUI_LOG_PRINTF("ERROR: RUI_AT_PARAMETER_INVALID %d\r\n",RUI_AT_PARAMETER_INVALID);
            return ;
        }
          
        send_data=argv[3]; 
        app_len = strlen(argv[3]);  

        rui_return_status = rui_uart_send(atoi(argv[2]),send_data,app_len); 
        switch(rui_return_status)
        {
            case RUI_STATUS_OK:RUI_LOG_PRINTF("\r\nUart%d send success\r\nOK\r\n",atoi(argv[2])); 
                break;
            case RUI_STATUS_UART_SEND_ERROR:RUI_LOG_PRINTF("ERROR: RUI_AT_UART_SEND_ERROR %d\r\n",RUI_AT_UART_SEND_ERROR);
                return FAIL;
            default :RUI_LOG_PRINTF("ERROR: RUI_AT_PARAMETER_INVALID %d\r\n",RUI_AT_PARAMETER_INVALID);
        }
          
    }
    else if(app_lora_status.work_mode == RUI_P2P)
    {
        if(strcmp(argv[1],"lorap2p") == 0)
        {
            if(i!=1)
            {
                RUI_LOG_PRINTF("ERROR: RUI_AT_PARAMETER_INVALID %d\r\n",RUI_AT_PARAMETER_INVALID);
                return ;
            } 

            // memset(send_data,0,256);
            // strcmp(send_data,argv[2]); 
            send_data=argv[2]; 
            app_len = strlen(argv[2]);
            if (app_len%2) 
            {
                RUI_LOG_PRINTF("ERROR: RUI_AT_PARAMETER_INVALID %d\r\n",RUI_AT_PARAMETER_INVALID);
                return ;
            }
            for (int i = 0; i < app_len; i++) 
            {
                if (!isxdigit(send_data[i])) 
                {
                    RUI_LOG_PRINTF("ERROR: RUI_AT_PARAMETER_INVALID %d\r\n",RUI_AT_PARAMETER_INVALID);
                    return ;   
                }
            }
            app_len = app_len/2;
            for (int i = 0; i < app_len; i++) 
            {
                memcpy(hex_num, &send_data[i*2], 2);
                send_data[i] = strtoul(hex_num, NULL, 16);
            } 

            rui_lorap2p_send(send_data,app_len);
            RUI_LOG_PRINTF("LoRaP2P send success\r\nOK\r\n");
            return;
        }else
        {
            RUI_LOG_PRINTF("ERROR: RUI_AT_PARAMETER_INVALID %d\r\n",RUI_AT_PARAMETER_INVALID);
            return ;
        }

    }else if(app_lora_status.work_mode == RUI_LORAWAN)
    {
        if(strcmp(argv[1],"lora") != 0)
        {
            RUI_LOG_PRINTF("ERROR: RUI_AT_PARAMETER_INVALID %d\r\n",RUI_AT_PARAMETER_INVALID);
            return ;
        }
        
    
        if(i!=2)
        {
            RUI_LOG_PRINTF("ERROR: RUI_AT_PARAMETER_INVALID %d\r\n",RUI_AT_PARAMETER_INVALID);
            return ;
        } 

        send_data=argv[3];            

        app_len = strlen(argv[3]);
        if (app_len%2) 
        {
            RUI_LOG_PRINTF("ERROR: RUI_AT_PARAMETER_INVALID %d\r\n",RUI_AT_PARAMETER_INVALID);
            return;
        }
        for (int i = 0; i < app_len; i++) 
        {
            if (!isxdigit(send_data[i])) 
            {
                RUI_LOG_PRINTF("ERROR: RUI_AT_PARAMETER_INVALID %d\r\n",RUI_AT_PARAMETER_INVALID);
                return ;   
            }
        }
        app_len = app_len/2;
        for (int i = 0; i < app_len; i++) 
        {
            memcpy(hex_num, &send_data[i*2], 2);
            send_data[i] = strtoul(hex_num, NULL, 16);
        } 

        rui_return_status = rui_lora_send(atoi(argv[2]),&send_data[0],app_len);
        switch(rui_return_status)
        {
            case RUI_STATUS_OK:break;
            case RUI_LORA_STATUS_NO_NETWORK_JOINED:RUI_LOG_PRINTF("ERROR: RUI_LORA_STATUS_NO_NETWORK_JOINED %d\r\n",RUI_LORA_STATUS_NO_NETWORK_JOINED);break;
            case RUI_STATUS_PARAMETER_INVALID:RUI_LOG_PRINTF("ERROR: RUI_AT_PARAMETER_INVALID %d\r\n",RUI_AT_PARAMETER_INVALID);
                break;
            default: RUI_LOG_PRINTF("ERROR: RUI_RETURN_STATUS %d\r\n",rui_return_status);
                break;
        } 
        return;
    }else
    {
        RUI_LOG_PRINTF("ERROR: RUI_AT_UNSUPPORT %d\r\n",RUI_AT_UNSUPPORT);
        return ;
    }
}

static void atcmd_help(int argc, char *argv[])
{
    RUI_LOG_PRINTF("OK\r\n*************************************************\r\n===============AT Commands List==================\r\n");
    RUI_LOG_PRINTF("Device AT commands:\r\n");
    RUI_LOG_PRINTF("  at+version\r\n");
    RUI_LOG_PRINTF("  at+help\r\n");
    RUI_LOG_PRINTF("  at+run\r\n");  //exit boot mode
    RUI_LOG_PRINTF("  at+set_config=device:restart\r\n");       
    RUI_LOG_PRINTF("  at+set_config=device:sleep:X\r\n");
    RUI_LOG_PRINTF("  at+set_config=device:boot\r\n"); 
    RUI_LOG_PRINTF("  at+get_config=device:status\r\n"); 
    
    RUI_LOG_PRINTF("  at+set_config=device:uart:X:Y\r\n");
    RUI_LOG_PRINTF("  at+set_config=device:uart_mode:X:Y\r\n");
    RUI_LOG_PRINTF("  at+send=uart:X:YYY\r\n"); 
    RUI_LOG_PRINTF("  at+set_config=device:gpio:X:Y\r\n");
    RUI_LOG_PRINTF("  at+get_config=device:gpio:X\r\n"); 
    RUI_LOG_PRINTF("  at+get_config=device:adc:X\r\n");
    RUI_LOG_PRINTF("  at+set_config=device:i2c:X:YY:ZZ:LL\r\n"); 
    RUI_LOG_PRINTF("\r\n");

    RUI_LOG_PRINTF("LoRaWAM AT commands:\r\n");
    RUI_LOG_PRINTF("  at+join\r\n");
    RUI_LOG_PRINTF("  at+send=lora:X:YYY\r\n");
    RUI_LOG_PRINTF("  at+set_config=lora:region:XXX\r\n");
    RUI_LOG_PRINTF("  at+get_config=lora:channel\r\n");    
    RUI_LOG_PRINTF("  at+set_config=lora:dev_eui:XXXX\r\n");
    RUI_LOG_PRINTF("  at+set_config=lora:app_eui:XXXX\r\n");
    RUI_LOG_PRINTF("  at+set_config=lora:app_key:XXXX\r\n");
    RUI_LOG_PRINTF("  at+set_config=lora:dev_addr:XXXX\r\n");
    RUI_LOG_PRINTF("  at+set_config=lora:apps_key:XXXX\r\n");
    RUI_LOG_PRINTF("  at+set_config=lora:nwks_key:XXXX\r\n");    
    RUI_LOG_PRINTF("  at+set_config=lora:join_mode:X\r\n");
    RUI_LOG_PRINTF("  at+set_config=lora:work_mode:X\r\n");
    RUI_LOG_PRINTF("  at+set_config=lora:ch_mask:X:Y\r\n");
    RUI_LOG_PRINTF("  at+set_config=lora:class:X\r\n");
    RUI_LOG_PRINTF("  at+set_config=lora:confirm:X\r\n");    
    RUI_LOG_PRINTF("  at+set_config=lora:dr:X\r\n");
    RUI_LOG_PRINTF("  at+set_config=lora:tx_power:X\r\n");
    RUI_LOG_PRINTF("  at+set_config=lora:adr:X\r\n");
    RUI_LOG_PRINTF("  at+set_config=lora:send_interval:X:Y\r\n");
    RUI_LOG_PRINTF("  at+get_config=lora:status\r\n");    
    RUI_LOG_PRINTF("\r\n");

    RUI_LOG_PRINTF("LoRaP2P AT commands:\r\n");
    RUI_LOG_PRINTF("  at+set_config=lorap2p:XXX:Y:Z:A:B:C\r\n");
    RUI_LOG_PRINTF("  at+send=lorap2p:XXX\r\n");  
    RUI_LOG_PRINTF("\r\n");

    RUI_LOG_PRINTF("Sensor AT commands:\r\n"); 
    RUI_LOG_PRINTF("  at+set_config=device:gps_timeout:X\r\n");
    RUI_LOG_PRINTF("  at+set_config=device:gps_format:X\r\n");


    RUI_LOG_PRINTF("===================List End======================\r\n");
    RUI_LOG_PRINTF("*************************************************\r\n"); 
}



