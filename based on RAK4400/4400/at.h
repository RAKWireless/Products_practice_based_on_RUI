#ifndef __AT_H__
#define __AT_H__



#define AT_BLE_MASK     0x01
#define AT_UART_MARK    0x02
#define AT_USBD_MASK	0x04
//at cmd return info, include 128 kinds

//successful return ok
#define RAK_OK					0x00	//	0000 0000

//error return error first and reason followed
#define RAK_ERROR				0x80	//	1000 0000
#define READ_FLASH_FAIL			0x81	//	1000 0001
#define WRITE_FLASH_FAIL		0x82	//  1000 0010

#define RAK_ERROR_LORA		    0x10
#define RAK_ERROR_NOT_JOIN      0x11

#define AT_HELP     \
"at+version----Get the current firmware version number \n\
at+set_config=device:sleep:X----device sleep command \n\
at+set_config=device:restart----device restart cmd \n\
at+set_config=ble:work_mode:X:Y----set ble work mode \n\
at+help----show all at command supported \n"

#endif

