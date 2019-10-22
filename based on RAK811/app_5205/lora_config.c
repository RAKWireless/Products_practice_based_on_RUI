#include <stdint.h>
#include <string.h>
#include "at_cmd.h"
#include "lora_config.h"
#include "rui.h"
#include "bsp.h"


#define MAX_ARGV        10
static RUI_LORA_STATUS_T app_lora_status; //record lora status 


static uint32_t handle_device_config(lora_config_t *config, int argc, char *argv[], char *in);
static uint32_t handle_lora_config(lora_config_t *config, int argc, char *argv[], char *in);
static uint32_t handle_lorap2p_config(RUI_LORA_STATUS_T *config, int argc, char *argv[], char *in);
static uint32_t handle_device_status(void);
static uint32_t user_set_gps_timeout(uint32_t gpstimeout);
struct board_config_cmd
{
    char *name;
    board_config_Enum board_enum;
};
struct board_config_cmd cmd_str[]=
{
    "restart",restart,
    "sleep",sleep,
    "boot",boot,
    "status",status,
    "uart",uart,
    "uart_mode",uart_mode,
    "gpio",gpio,
    "adc",adc,
    "i2c",i2c,
    "region",region,
    "channel",channel,     
    "dev_eui",dev_eui,
    "app_eui",app_eui,
    "app_key",app_key,
    "dev_addr",dev_addr,
    "apps_key",apps_key,
    "nwks_key",nwks_key,
    "join_mode",join_mode,
    "work_mode",work_mode,
    "ch_mask",ch_mask,
    "class",class,
    "confirm",confirm,
    "dr",dr,
    "tx_power",tx_power,
    "adr",adr,
    "send_interval",send_interval,

    "gps_timeout",gps_timeout
};
/** Structure for registering CONFIG commands */
struct config_cmd
{
    /** The name of the CONFIG command */
    char *name;
    /** The help text associated with the command */
    //const char *help;
    /** The function that should be invoked for this command. */
    int (*function) (RUI_LORA_STATUS_T *config, int argc, char *argv[], char *in);
};

struct config_cmd config_cmds[] = 
{
    "device",                handle_device_config,
    "lora",                  handle_lora_config,
    "lorap2p",               handle_lorap2p_config,
};

static int parse_args(char* str, char* argv[], char **end)
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

        while(*ch != ',' && *ch != '\0' && *ch != '&') {
            // if(*ch == ':' && i == 1) {
            if(*ch == ':' ) {
                break;
            }
            else
                ch++;
        }
        
        if (*ch == '&') {
            *ch = '\0';
            *end = ++ch;
            break;
        } else if (*ch == '\0'){
            *end = NULL;
            break;
        }
        
        *ch = '\0';
        ch++;
        while(*ch == ',') {
            ch++;
        }
    }
    return i;
}

static int read_config_string(RUI_LORA_STATUS_T *config, const char *in)
{
    int i;
    int ret;
    int argc;
    char *argv[MAX_ARGV];
    char *end;

    do
    {
        argc = parse_args(in, argv, &end);

        if((strcmp(argv[1],"gpio") == 0) || (strcmp(argv[1],"adc") == 0))
        {
            if (argc > 3) 
            {
                RUI_LOG_PRINTF("Too many parameters.\r\n");
                return -1;
            }else if(argc < 3)
            {
                RUI_LOG_PRINTF("Too few parameters.\r\n");
                return -1;
            }
        }else if (argc > 2) 
        {
            RUI_LOG_PRINTF("Too many parameters.\r\n");
            return -1;
        }else if(argc < 2) 
        {
            RUI_LOG_PRINTF("Too few parameters.\r\n");
            return -1;
        }

        for (i = 0; i < sizeof(config_cmds)/sizeof(struct config_cmd); i++) 
        {
            if (strcmp(in, config_cmds[i].name) == 0) 
            {
                ret = config_cmds[i].function(config,argc - 1,&argv[1], NULL);
                if (ret < 0) 
                {
                    return ret;
                }
                break;
            }
        }  
        if (i == sizeof(config_cmds)/sizeof(struct config_cmd)) 
        {
            RUI_LOG_PRINTF("The AT Command is invalid\r\n");
            return -1;
        }  
    }while(end != NULL); 
    return 0;   
}

static int write_config_string(RUI_LORA_STATUS_T *config, char *in)
{
    int i;
    int ret;
    int argc;
    char *argv[MAX_ARGV];
    char *end;
    do 
    {
        argc = parse_args(in, argv, &end);
        if (argc <= 2) 
        {
            if ((strcmp(argv[0], config_cmds[0].name) != 0) && ( (strcmp(argv[0], config_cmds[2].name) != 0)))
            {
                RUI_LOG_PRINTF("Too few parameters.\r\n");
                return -1;
            }
        }

        in = end;
        for (i = 0; i < sizeof(config_cmds)/sizeof(struct config_cmd); i++) 
        {
            if (strcmp(argv[0], config_cmds[i].name) == 0) 
            {
                ret = config_cmds[i].function(config, argc - 1, &argv[1], NULL);
                if (ret < 0) 
                {
                    return ret;
                }
                break;
            }
        }
        if (i == sizeof(config_cmds)/sizeof(struct config_cmd)) 
        {
            RUI_LOG_PRINTF("The AT Command is invalid.\r\n");
            return -1;
        }          
    }while (end != NULL);
    
    return 0; 
}


int write_config(char *in)
{    
    int ret;
    ret = write_config_string(&app_lora_status, in);
    if (ret < 0) {
        return ret;
    }      
    return ret;
}

int read_config(char *in)
{
    int ret;  
    ret = read_config_string(&app_lora_status, in);
    if (ret < 0) {
        return ret;
    }
    return ret;
}


static uint32_t handle_device_config(lora_config_t *config, int argc, char *argv[], char *in)
{
    uint8_t i;
    float x0,y0,z0;
    float f_data;
    RUI_GPIO_ST rui_gpio;
    RUI_UART_BAUDRATE br;

    for (i = 0; i < sizeof(cmd_str)/sizeof(struct board_config_cmd); i++)
    {
        if (strcmp(argv[0], cmd_str[i].name) == 0)
        {
            break;
        }        
    } 
    if (i == sizeof(cmd_str)/sizeof(struct board_config_cmd)) 
    {
        RUI_LOG_PRINTF("The AT Command is invalid.\r\n");
        return -1;            
    }
    
    switch(cmd_str[i].board_enum)
    {
        case restart:
            RUI_LOG_PRINTF("OK,restart ...\r\n");
            rui_delay_ms(100);
            rui_device_reset();
            break;
        case sleep:
            if(argc != 2)return -1;
            if(atoi(argv[1]) <= 2)
            {
                rui_device_sleep(atoi(argv[1]));
                return -1;
            }else return -1;
            break; 
        case boot:
            rui_device_boot();  
            break;  
        case uart:
            if(argc < 3)
            {
                RUI_LOG_PRINTF("Too few parameters.\r\n");
                return -1;
            }else if(argc > 3)
            {
                RUI_LOG_PRINTF("Too many parameters.\r\n");
                return -1;
            }else
            {
                switch(atoi(argv[2]))
                {
                    case 1200:br = BAUDRATE_1200;
                        break;
                    case 2400:br = BAUDRATE_2400;
                        break;
                    case 4800:br = BAUDRATE_4800;
                        break;
                    case 9600:br = BAUDRATE_9600;
                        break;
                    case 19200:br = BAUDRATE_19200;
                        break;
                    case 38400:br = BAUDRATE_38400;
                        break;
                    case 57600:br = BAUDRATE_57600;
                        break;
                    case 115200:br = BAUDRATE_115200;
                        break;
                    default:RUI_LOG_PRINTF("Parameter is invalid.\r\n");
                    return -1;
                    break;
                }
                if(rui_uart_init(atoi(argv[1]),br) < 0)return -1;
            }
            break; 
        case uart_mode:
            if(argc < 3)
            {
                RUI_LOG_PRINTF("Too few parameters.\r\n");
                return -1;
            }else if(argc > 3)
            {
                RUI_LOG_PRINTF("Too many parameters.\r\n");
                return -1;
            }else if(rui_uart_mode_config(atoi(argv[1]),atoi(argv[2])) == 0)
            {
                switch(atoi(argv[2]))
                {
                    case RUI_UART_NORAMAL: RUI_LOG_PRINTF("Current AT uart work mode:normal mode\r\n"); 
                        break;
                    case RUI_UART_UNVARNISHED:RUI_LOG_PRINTF("Current AT uart work mode:unvarnished transmit mode\r\n");
                        break;   
                }
            }else return -1;

            break;
        case gpio:
            if(argc == 2)
            {                
                uint8_t pinVal;
                if(atoi(argv[0]) != 0)
                {
                    RUI_LOG_PRINTF("Parameter format error.\r\n");
                    return -1;
                }
                rui_gpio.pin_num = atoi(argv[1]);
                rui_gpio.dir = RUI_GPIO_PIN_DIR_INPUT;
                rui_gpio_init(&rui_gpio);
                if(rui_gpio_rw(0,&rui_gpio,&pinVal) < 0)
                {
                    rui_gpio_uninit(&rui_gpio);
                    return -1;
                }
                RUI_LOG_PRINTF("OK,pin level is:%d\r\n", pinVal);
                rui_gpio_uninit(&rui_gpio);
            }
            else if(argc == 3)
            {
                uint8_t pinVal;
                pinVal = atoi(argv[2]);
                rui_gpio.pin_num = atoi(argv[1]);
                rui_gpio.dir = RUI_GPIO_PIN_DIR_OUTPUT;
                rui_gpio_init(&rui_gpio);
                if(rui_gpio_rw(1,&rui_gpio,&pinVal) < 0)
                {
                    rui_gpio_uninit(&rui_gpio);
                    return -1;
                }
            }
            else return -1;
            break;         
        case adc:
            if(argc == 2)
            {
                uint16_t adc_value;
                rui_gpio.pin_num = atoi(argv[1]);
                rui_gpio.dir = RUI_GPIO_PIN_DIR_INPUT;
                rui_adc_init(&rui_gpio);
                if(rui_adc_get(&rui_gpio,&adc_value) < 0)
                {
                    rui_adc_uninit(&rui_gpio);
                    return -1;
                }
                RUI_LOG_PRINTF("OK,Voltage: %dmV.\r\n",adc_value);
                rui_adc_uninit(&rui_gpio);
            }
            else return -1;
            break;
        case i2c:
            if(argc < 5)
            {
                RUI_LOG_PRINTF("Too few parameters.\r\n");
                return -1;
            }else if(argc >5)
            {
                RUI_LOG_PRINTF("Too many parameters.\r\n");
                return -1;
            }else
            {
                uint8_t i2c_data[64]; 
                int app_len;
                char hex_num[3] = {0}; 
                char* send_data;
                RUI_I2C_ST I2c_temp;
                I2c_temp.INSTANCE_ID = 1;
                if(atoi(argv[1])==0)
                {
                    if(rui_i2c_rw(&I2c_temp,RUI_IF_READ,strtoul(argv[2],0,16),(uint16_t)strtoul(argv[3],0,16),i2c_data,(uint16_t)atoi(argv[4])) != 0)
                    {
                        RUI_LOG_PRINTF("i2c read error.\r\n");                        
                        return -1;
                    }else
                    {
                        RUI_LOG_PRINTF("i2cdata: ");                        
                        for(int8_t z=0; z<(uint16_t)atoi(argv[4]); z++)RUI_LOG_PRINTF("%d ",i2c_data[z]);
                        RUI_LOG_PRINTF("\r\n");
                    }
                }else if(atoi(argv[1])==1)
                {
                    app_len = strlen(argv[4]);
                    send_data = argv[4];
                    if (app_len%2) 
                    {
                        RUI_LOG_PRINTF("Parameter format error.\r\n");
                        return -1;
                    }

                    for (int i = 0; i < app_len; i++) 
                    {
                        if (!isxdigit(send_data[i])) 
                        {
                            RUI_LOG_PRINTF("Please entry hexadecimal character.\r\n");
                            return -1;   
                        }
                    }
                    
                    app_len = app_len/2;
                    for (int i = 0; i < app_len; i++) 
                    {
                        memcpy(hex_num, &send_data[i*2], 2);
                        i2c_data[i] = strtoul(hex_num, NULL, 16);
                    }
                    
                    if(rui_i2c_rw(&I2c_temp,RUI_IF_WRITE,strtoul(argv[2],0,16),(uint16_t)strtoul(argv[3],0,16),i2c_data,(uint16_t)atoi(argv[4])) != 0)
                    {
                        RUI_LOG_PRINTF("i2c write error.\r\n");
                        return -1;
                    } 
                }else
                {
                    RUI_LOG_PRINTF("i2c read/write format error.\r\n");
                    return -1;
                }
                               
            }            
            break;
        case status:handle_device_status();
            break;
        case gps_timeout:
            if(argc == 2)user_set_gps_timeout(atoi(argv[1]));
            else return -1;
            break;
        default :RUI_LOG_PRINTF("Parameter is invalid.\r\n");return -1;
            break;
    }
    return 0;

}

static int verify_config_data(uint8_t argc,char* buffer,char len,char* lora_id)
{
    char hex_num[3] = {0};
    if (argc != 2) 
    {
        return -1;
    }             
    if (strlen(buffer) != 2*len) {
        RUI_LOG_PRINTF("Parameters length is invalid.\r\n");
        return -1;
    }
    for (int i = 0; i < 2*len; i++) {
        if (!isxdigit(buffer[i])) {
            RUI_LOG_PRINTF("Please entry hexadecimal character parameter.\r\n");
            return -1;    
        }
    }
    for (int i = 0; i < len; i++) {
        memcpy(hex_num, &buffer[i*2], 2);
        lora_id[i] = strtoul(hex_num, NULL, 16);
    }
   return 0;
}

static uint32_t handle_lora_config(lora_config_t *config, int argc, char *argv[], char *in)
{
    uint8_t i;
    char lora_id[32];
    RUI_LORA_STATUS_T* st;

    for (i = 0; i < sizeof(cmd_str)/sizeof(struct board_config_cmd); i++)
    {
        if (strcmp(argv[0], cmd_str[i].name) == 0)
        {
            break;
        }        
    }
    if (i == sizeof(cmd_str)/sizeof(struct board_config_cmd)) 
    {
        RUI_LOG_PRINTF("The AT Command is invalid.\r\n");
        return -1;
    }     

   switch(cmd_str[i].board_enum)
    {
        case region:
            rui_lora_get_status(false,&app_lora_status);
            if ( 0==strcmp(argv[1], app_lora_status.region)) 
            { 
                RUI_LOG_PRINTF("No switch region.Current region:%s\r\n",app_lora_status.region);
            } 
            else 
            {	
                if (rw_Str2Region(argv[1]) == 100) 
                {
                    RUI_LOG_PRINTF("No region found.\r\n");
                    return;
                }
                else return rui_lora_set_region(rw_Str2Region(argv[1]));
            }
            break;
        case channel:rui_get_channel_list();
            break;
        case status:
            rui_lora_get_status(true,st);
            break;
        case dev_eui:            
            if(verify_config_data(argc,argv[1],8,lora_id) == -1)return -1;        
            rui_lora_set_dev_eui(lora_id);
            break;
        case app_eui:
            if(verify_config_data(argc,argv[1],8,lora_id) == -1)return -1; 
            rui_lora_set_app_eui(lora_id);
            break;
        case app_key:
            if(verify_config_data(argc,argv[1],16,lora_id) < 0) return -1;  
            rui_lora_set_app_key(lora_id);        
            break;
        case dev_addr:            
            if(verify_config_data(argc,argv[1],4,lora_id) == -1)return -1;  
            rui_lora_set_dev_addr(lora_id);    
            break;
        case apps_key:
            if(verify_config_data(argc,argv[1],16,lora_id) == -1)return -1;
            rui_lora_set_apps_key(lora_id);   
            break;
        case nwks_key:
            if(verify_config_data(argc,argv[1],16,lora_id) == -1)return -1;
            rui_lora_set_nwks_key(lora_id);            
            break;                
        case join_mode:
            if(rui_lora_set_join_mode(atoi(argv[1])) < 0)return -1;  
            if(atoi(argv[1])==0)RUI_LOG_PRINTF("join_mode:OTAA\r\n");     
            if(atoi(argv[1])==1)RUI_LOG_PRINTF("join_mode:ABP\r\n");     
            break;
        case work_mode:
            if(rui_lora_set_work_mode(atoi(argv[1])) < 0)return -1;          
            break;
        case ch_mask:
            if(rui_lora_set_channel_mask(atoi(argv[1]),atoi(argv[2])) < 0)return -1;
            break;
        case class:
            if(rui_lora_set_class(atoi(argv[1])) < 0)return -1; 
            switch(atoi(argv[1]))
            {
                case 0:RUI_LOG_PRINTF("Class:A\r\n");
                    break;
                case 1:RUI_LOG_PRINTF("Class:B\r\n");
                    break;
                case 2:RUI_LOG_PRINTF("Class:C\r\n");
                    break;
                default: break;
            }        
            break;
        case confirm: 
            if(rui_lora_set_confirm(atoi(argv[1])) < 0)return -1; 
            switch(atoi(argv[1]))
            {
                case 0:RUI_LOG_PRINTF("LoRaWAN Ack:unconfirm\r\n");
                    break;
                case 1:RUI_LOG_PRINTF("LoRaWAN Ack:confirm\r\n");
                    break;
                default: break;
            }       
            break;   
        case dr: 
            if(rui_lora_set_dr(atoi(argv[1])) < 0)return -1;        
            break;
        case tx_power:
            if(rui_lora_set_tx_power(atoi(argv[1])) < 0)return -1;
            break; 
        case adr: 
            if(rui_lora_adr(atoi(argv[1])) < 0)return -1;        
            break; 
        case send_interval: 
            if (argc != 3){return -1;} 
            if(atoi(argv[1]) > 2)return -1;
            if(atoi(argv[1]) == 0)RUI_LOG_PRINTF("Close auto send data.\r\n");
            else if (atoi(argv[1]) == 1)
            {
                RUI_LOG_PRINTF("Start auto send data with sleep.\r\n");                     
            }else if (atoi(argv[1]) == 2)
            {
                RUI_LOG_PRINTF("Start auto send data,no sleep.\r\n");                     
            }
            if(rui_lora_set_send_interval(atoi(argv[1]),atoi(argv[2])) < 0)return -1;
            break;
        default :return -1;
            break;
    }  
    return 0;  
}

static uint32_t  handle_lorap2p_config(RUI_LORA_STATUS_T *config, int argc, char *argv[], char *in)
{ 
    uint32_t Frequency;
    uint8_t  Spreadfact;
    uint8_t  Bandwidth; 
    uint8_t  Codingrate; 
    uint16_t  Preamlen; 
    uint8_t  Powerdbm;

    if(argc > 6)
    {
        RUI_LOG_PRINTF("Too many parameters.\r\n");
        return -1;
    }

    Frequency = atoi(argv[0]);
    RUI_LOG_PRINTF("Frequency=%d\r\n",Frequency);

    if((atoi(argv[1])>12)||(atoi(argv[1])<7))
    {
        RUI_LOG_PRINTF("Spreadfact over limit <7-12>.\r\n");
        return -1;
    } else Spreadfact = atoi(argv[1]);

    if((atoi(argv[2])>2)||(atoi(argv[2])<0))
    {
        RUI_LOG_PRINTF("Bandwidth over limit <0-2>.\r\n");
        return -1;
    } else Bandwidth = atoi(argv[2]);

    if((atoi(argv[3])>4)||(atoi(argv[3])<1))
    {
        RUI_LOG_PRINTF("Codingrate over limit <1-4>.\r\n");
        return -1;
    } else Codingrate = atoi(argv[3]);

    if((atoi(argv[4])>65535)||(atoi(argv[4])<2))
    {
        RUI_LOG_PRINTF("Preamlen over limit <5-65535>.\r\n");
        return -1;
    } else Preamlen = atoi(argv[4]);

    if((atoi(argv[5])>20)||(atoi(argv[5])<0))
    {
        RUI_LOG_PRINTF("Powerdbm over limit <0-20>.\r\n");
        return -1;
    } else Powerdbm = atoi(argv[5]);

    if(rui_lorap2p_config(Frequency,Spreadfact,Bandwidth,Codingrate,Preamlen,Powerdbm)==0)return 0;
    else return -1;
}


extern bsp_sensor_data_t bsp_sensor;
extern TimerEvent_t Gps_Cnt_Timer;  //search satellite timer
extern user_store_data_t user_store_data;
static uint32_t handle_device_status(void)
{
    RUI_LOG_PRINTF("OK.\r\n*************************************************\r\n===============Device Status List================\r\n"); 

    RUI_LOG_PRINTF("Board Core:  RAK811\r\n");
    RUI_LOG_PRINTF("MCU:  STM32L151CB_A\r\n");   
    RUI_LOG_PRINTF("LoRa chip:  SX1276\r\n"); 
    RUI_LOG_PRINTF("\r\n");   

    RUI_LOG_PRINTF("Battery Voltage:%d.%d V \r\n",(uint32_t)(bsp_sensor.voltage), (uint32_t)((bsp_sensor.voltage)*1000-((int32_t)(bsp_sensor.voltage)) * 1000));
    
    RUI_LOG_PRINTF("gps_timeout: %d\r\n",user_store_data.gps_timeout_cnt);
    RUI_LOG_PRINTF("GPS data:");
    RUI_LOG_PRINTF("  latitude: %d.%d, longitude: %d.%d , altitude: %d.%dm \r\n",
						(int32_t)bsp_sensor.latitude,abs((int32_t)(bsp_sensor.latitude*1000000-((int32_t)bsp_sensor.latitude) * 1000000)),
						(int32_t)bsp_sensor.longitude,abs((int32_t)(bsp_sensor.longitude*1000000-((int32_t)bsp_sensor.longitude) * 1000000)),    
						bsp_sensor.altitude/10,abs(bsp_sensor.altitude%10));

    RUI_LOG_PRINTF("LIS3DH sensor data:");
    RUI_LOG_PRINTF("  LIS3DH X,Y,Z: %dmg, %dmg, %dmg\r\n",(int32_t)bsp_sensor.triaxial_x , (int32_t)bsp_sensor.triaxial_y , (int32_t)bsp_sensor.triaxial_z );

    RUI_LOG_PRINTF("BME680 sensor data:\r\n");
    RUI_LOG_PRINTF("  Humidity:%d.%d %%RH\r\n",(int32_t)(bsp_sensor.humidity/1000),(int32_t)(bsp_sensor.humidity%1000));		
    RUI_LOG_PRINTF("  Temperature:%d.%d degree\r\n",(int32_t)(bsp_sensor.temperature/100),(int32_t)(bsp_sensor.temperature%100));	
    RUI_LOG_PRINTF("  Pressure:%d.%d hPa\r\n",(int32_t)(bsp_sensor.pressure/100),(int32_t)(bsp_sensor.pressure%100));	
    RUI_LOG_PRINTF("  Gas_resistance: %d ohms \r\n", bsp_sensor.resis);	

    RUI_LOG_PRINTF("===================List End======================\r\n"); 
    RUI_LOG_PRINTF("*************************************************\r\n");       
}

uint32_t user_set_gps_timeout(uint32_t gpstimeout)
{
    user_store_data.gps_timeout_cnt = gpstimeout;
    rui_timer_stop(&Gps_Cnt_Timer); 
    rui_timer_setvalue( &Gps_Cnt_Timer, user_store_data.gps_timeout_cnt * 1000 );
    rui_timer_start(&Gps_Cnt_Timer); //restart search satellite timer
    rui_flash_write(RUI_FLASH_USER,&user_store_data.gps_timeout_cnt,2);
}

