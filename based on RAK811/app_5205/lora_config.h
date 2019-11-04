#ifndef _LORA_CONFIG_H_
#define _LORA_CONFIG_H_

typedef union 
{ 
    uint8_t  data0[4];
    uint32_t data1;
}union_data;

typedef enum
{
    restart,
    sleep,
    boot,
    status,    
    uart,
    uart_mode,
    gpio,
    adc,
    i2c,
    region,
    channel,    
    dev_eui,
    app_eui,
    app_key,
    dev_addr,
    apps_key,
    nwks_key,
    join_mode,
    work_mode,    
    ch_mask,
    class,
    confirm,
    dr,
    tx_power,
    adr,
    send_interval,

/******************************************************
*  The following AT_cmd are user customizations
******************************************************/
    gps_timeout,
    gps_format
/**************** customizations end ******************/
}board_config_Enum;

typedef enum DRIVER_MODE
{
	NORMAL_MODE = 0,
	POWER_ON_MODE,
	POWER_OFF_MODE,
	SLEEP_MODE,
	STANDBY_MODE
}DRIVER_MODE;


int read_config(char *in);

#endif