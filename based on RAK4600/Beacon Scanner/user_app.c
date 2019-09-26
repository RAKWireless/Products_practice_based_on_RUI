#include "rui.h"


void rui_ble_scan_adv(int8_t rssi_value, uint8_t *p_adv_data, uint16_t adv_data_len, uint8_t *p_device_mac)
{
    /*
       Company: Apple, Inc.<0x004C>
       Type: Beacon <0x02>
     */
    uint8_t filter_data[]={0x4C, 0x00, 0x02};
    uint8_t *p_manu_data = ble_advdata_parse(p_adv_data, adv_data_len, BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA);

    /* Filter ble device by broadcast data */
    if (0x00 == memcmp(filter_data, p_manu_data, 3))
    {
        RUI_LOG_PRINTF("Target device mac is %02x-%02x-%02x-%02x-%02x-%02x \n",
                        p_device_mac[5], p_device_mac[4], p_device_mac[3],
                        p_device_mac[2], p_device_mac[1], p_device_mac[0]);
    }
}

void main(void)
{
    //system init 
    rui_init();
    //you can add your init code here, like timer, uart, spi...

    while(1)
    {
        //do your work here, then call rui_device_sleep(1) to sleep

        //here run system work and do not modify
        rui_running();
    }
}

