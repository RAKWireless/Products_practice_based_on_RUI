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
    uart,
    gpio,
    adc,
    i2c,
    region,
    channel,
    status,
    dev_eui,
    app_eui,
    app_key,
    dev_addr,
    apps_key,
    nwks_key,
    join_mode,
    work_mode,
    uart_mode,
    ch_mask,
    class,
    confirm,
    dr,
    tx_power,
    adr,
    send_interval,

    gps,
    gps_timeout,
    acceleration,
    magnetic,
    gyroscope,
    pressure,
    temperature,
    humidity,
    light_strength,
    voltage,
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