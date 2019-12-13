#ifndef __AT_H__
#define __AT_H__



#define AT_BLE_MASK     0x01
#define AT_UART_MARK    0x02
#define AT_USBD_MASK	0x04


#define AT_HELP     \
"************************************************** \n\
================ AT Command List ================ \n\
at+version \n\
at+set_config=device:sleep:X \n\
at+set_config=device:restart \n\
at+set_config=lora:dev_eui:XXXX \n\
at+set_config=lora:app_eui:XXXX \n\
at+set_config=lora:app_key:XXXX \n\
at+set_config=lora:dev_addr:XXXX \n\
at+set_config=lora:apps_key:XXXX \n\
at+set_config=lora:nwks_key:XXXX \n\
at+set_config=lora:region:XXX \n\
at+set_config=lora:join_mode:X \n\
at+join \n\
at+send=lora:X:YYYY \n\
at+set_config=lora:work_mode:X \n\
at+set_config=lora:class:X \n\
at+set_config=lora:confirm:X \n\
at+set_config=lora:send_interval:X:Y \n\
at+get_config=lora:status \n\
at+get_config=lora:channel \n\
at+set_config=lora:ch_mask:X:Y \n\
at+set_config=device:uart_mode:X:Y \n\
at+set_config=ble:work_mode:X:Y \n\
at+help \n\
=================== List  End ===================\n\
**************************************************\n"


void uart_log_printf(const char *fmt, ...);


#endif

