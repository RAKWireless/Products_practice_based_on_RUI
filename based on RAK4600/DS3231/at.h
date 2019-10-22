#ifndef __AT_H__
#define __AT_H__



#define AT_BLE_MASK     0x01
#define AT_UART_MARK    0x02
#define AT_USBD_MASK	0x04


// rui return code
#define RAK_OK					0x00	//	0000 0000
#define RAK_ERROR				0x80	//	1000 0000
#define READ_FLASH_FAIL			0x81	//	1000 0001
#define WRITE_FLASH_FAIL		0x82	//  1000 0010
#define RAK_ERROR_LORA		    0x10
#define RAK_ERROR_NOT_JOIN      0x11


#define AT_HELP     \
"at+version----Get the current firmware version number \n\
at+set_config=device:restart----device restart cmd \n\
at+set_config=device:sleep:X----device sleep command \n\
at+help----show all at command supported \n\
at+set_config=ble:work_mode:----ble mode choose \n\
at+join----lora join \n\
at+send=lora:X:YYY----send data via lora \n\
at+set_config=lora:work_mode:X----config lora work mode \n\
at+set_config=lora:join_mode:X----lora join mode \n\
at+set_config=lora:class:X----lora class A/B/C/ \n\
at+set_config=lora:region:XXX----config lora region \n\
at+set_config=lora:confirm:X----lora send confirm/unconfirm \n\
at+set_config=lora:ch_mask:X:Y----channel mask \n\
at+set_config=lora:dev_eui:XXXX----config lora dev_eui \n\
at+set_config=lora:app_eui:XXXX----config lora app_eui \n\
at+set_config=lora:app_key:XXXX----config lora app_key \n\
at+set_config=lora:dev_addr:XXXX----config lora dev_addr \n\
at+set_config=lora:apps_key:XXXX----config lora apps_key \n\
at+set_config=lora:nwks_key:XXXX----config lora nwks_key \n\
at+set_config=lora:adr:X----lora adapt data rate on/off \n\
at+set_config=lora:dr:X----lora data rate \n\
at+set_config=lora:send_interval:X:Y----lora send interval \n\
at+get_config=lora:status----lora status \n\
at+get_config=lora:channel----lora channel \n"

#endif

