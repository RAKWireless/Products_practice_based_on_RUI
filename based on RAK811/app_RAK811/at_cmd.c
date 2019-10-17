#include "at_cmd.h"
#include "rui.h"


struct cli_cmd cli_cmds[] = 
{
    "version",        lora_version,         
    "join",           lora_join,
    "get_config",     lora_read_config,
    "set_config",     lora_write_config,
    "send",           lora_send,
    "region",         lora_region, 
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
		
    if ((strncmp(str0, "at+", 3) != 0) || str0[3] == '\0') {
        RUI_LOG_PRINTF("AT format error.\r\n");
        return -1;
    }
    str0 += 3;
    argc = parse_args(str0, argv);
    if (argc > 0) {
        for (i = 0; i < sizeof(cli_cmds)/sizeof(struct cli_cmd); i++) {
            if (strcmp(argv[0], cli_cmds[i].name) == 0) {
                cli_cmds[i].function(argc, argv);
                break;
            }        
        }
        if (i == sizeof(cli_cmds)/sizeof(struct cli_cmd)) {
            RUI_LOG_PRINTF("The AT Command is invalid\r\n");
        }
    }
    else {
        RUI_LOG_PRINTF("Parameter format error.\r\n");
        return -1;
    }
    return 1;
}

int check_hex_invaild(uint8_t *data, uint16_t len) 
{
    uint8_t check1 = 0xff, check2 = 0;
    
    for (int i = 0; i < len; i++) {
        check1 &= data[i];
        check2 |= data[i];
    }
    if (check1 == 0xff || check2 == 0) {
        return 1;
    }
    return 0;
}


static void out_hex_buf( char *buffer, uint16_t size)
{
     char hex_str[3] = {0};
  
    for (int i = 0; i < size; i++) {
        sprintf(hex_str, "%02x", buffer[i]);
        RUI_LOG_PRINTF("%s", hex_str); 
    }
}

bool IsJoiningflag;  //Flag whether is joining LoRaWAN
static void lora_join(int argc, char *argv[])
{
    RUI_LORA_STATUS_T app_lora_status;
    rui_lora_get_status(false,&app_lora_status);     
    if(argv[1] == NULL)
    {
        if(rui_lora_join() != 0)return;
        else if(app_lora_status.join_mode == RUI_ABP)
        {
            LoRaWANJoined_callback(1);
        }else 
        {
            RUI_LOG_PRINTF("OK\r\n");
            IsJoiningflag = true;
        }
    }
    else
    {
        RUI_LOG_PRINTF("Parameter format error.\r\n");
        return ;
    }    
    
}

static void lora_version(int argc, char *argv[])
{
    uint8_t version[20];
    rui_device_version(version);
    RUI_LOG_PRINTF("OK%s\r\n", version);
}

static LORA_REGION rw_String2Region(char* region)
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

static void lora_region(int argc, char *argv[])
{
    LORA_REGION region;
    RUI_LORA_STATUS_T app_lora_status;
    rui_lora_get_status(false,&app_lora_status);
    if (argc == 1) 
    {
        RUI_LOG_PRINTF("OK.%s\r\n", app_lora_status.region);
        return;
    } 
    else if (argc == 2) 
    {
        if ( 0==strcmp(argv[1], app_lora_status.region)) 
        { 
            RUI_LOG_PRINTF("OK.\r\n");
        } 
        else 
        {	
            region = rw_String2Region(argv[1]);
            if (region == 100) 
            {
                RUI_LOG_PRINTF("No region found.\r\n");;
                return;
            }
            else return rui_lora_set_region(region);
        }

    } else
    {
        RUI_LOG_PRINTF("Parameter format error.\r\n");
    }
   
}


static void lora_read_config(int argc, char *argv[])
{
    int ret;
    if (argc != 2) {
        RUI_LOG_PRINTF("Parameter format error.\r\n");
        return;
    }
    
    ret = read_config(argv[1]);
    if (ret < 0) {
        return;
    } 
}

static void lora_write_config(int argc, char *argv[])
{
    int ret; 
    if (argc < 2) {
        RUI_LOG_PRINTF("Parameter format error.\r\n");
        return;
    }
    
    ret = write_config(argv[1]);
    if (ret < 0) {
        return ;
    } else {
        RUI_LOG_PRINTF("OK\r\n");
    }
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
        RUI_LOG_PRINTF("Parameter format error.\r\n");
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
            RUI_LOG_PRINTF("Parameter format error.\r\n");
            return ;
        }
          
        send_data=argv[3]; 
        app_len = strlen(argv[3]);  

        rui_uart_send(atoi(argv[2]),send_data,app_len); 
        RUI_LOG_PRINTF("\r\nUart%d send OK\r\n",atoi(argv[2]));   
    }
    else if(app_lora_status.work_mode == RUI_P2P)
    {
        if(strcmp(argv[1],"lorap2p") == 0)
        {
            if(i!=1)
            {
                RUI_LOG_PRINTF("Parameter format error.\r\n");
                return ;
            } 

            // memset(send_data,0,256);
            // strcmp(send_data,argv[2]); 
            send_data=argv[2]; 
            app_len = strlen(argv[2]);
            if (app_len%2) 
            {
                RUI_LOG_PRINTF("Parameter format error.\r\n");
                return ;
            }
            for (int i = 0; i < app_len; i++) 
            {
                if (!isxdigit(send_data[i])) 
                {
                    RUI_LOG_PRINTF("Please entry hexadecimal character.\r\n");
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
            RUI_LOG_PRINTF("OK\r\n");
            return;
        }else
        {
            RUI_LOG_PRINTF("Parameter format error.\r\n");
            return ;
        }

    }else if(app_lora_status.work_mode == RUI_LORAWAN)
    {
        if(strcmp(argv[1],"lora") != 0)
        {
            RUI_LOG_PRINTF("Parameter format error.\r\n");
            return ;
        }
        
    
        if(i!=2)
        {
            RUI_LOG_PRINTF("Parameter format error.\r\n");
            return ;
        } 

        send_data=argv[3];            

        app_len = strlen(argv[3]);
        if (app_len%2) 
        {
            RUI_LOG_PRINTF("Parameter format error.\r\n");
            return;
        }
        for (int i = 0; i < app_len; i++) 
        {
            if (!isxdigit(send_data[i])) 
            {
                RUI_LOG_PRINTF("Please entry hexadecimal character.\r\n");
                return ;   
            }
        }
        app_len = app_len/2;
        for (int i = 0; i < app_len; i++) 
        {
            memcpy(hex_num, &send_data[i*2], 2);
            send_data[i] = strtoul(hex_num, NULL, 16);
        } 
        if(rui_lora_send(atoi(argv[2]),&send_data[0],app_len) < 0 )
        {
            return -1;
        } 
        RUI_LOG_PRINTF("OK\r\n");
        return;
    }else
    {
        RUI_LOG_PRINTF("Parameter format error.\r\n");
        return ;
    }
}

static void atcmd_help(int argc, char *argv[])
{
    RUI_LOG_PRINTF("OK.\r\n*************************************************\r\n===============AT Commands List==================\r\n");
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
    RUI_LOG_PRINTF("===================List End======================\r\n");
    RUI_LOG_PRINTF("*************************************************\r\n"); 
}



