#include "rui.h"
#include "lis3dh.h"
#include "opt3001.h"
#include "shtc3.h"
#include "lps22hb.h"

uint16_t ble_advdata_search_extra(uint8_t const * p_encoded_data,
                                    uint16_t        data_len,
                                    uint16_t      * p_offset,
                                    uint8_t         ad_type)
{
    if ((p_encoded_data == NULL) || (p_offset == NULL))
        return 0;

    uint16_t i = 0;

    while (((i < *p_offset) || (p_encoded_data[i + 1] != ad_type)) && (i < data_len))
    {
        // Jump to next data.
        i += (p_encoded_data[i] + 1);
    }

    if (i >= data_len)
        return 0;
    else
    {
        *p_offset = i + 2;
        return (p_encoded_data[i] - 1);
    }
}

uint8_t * ble_advdata_parse_extra(uint8_t  * p_encoded_data,
                                uint16_t   data_len,
                                uint8_t    ad_type)
{
    uint16_t offset = 0;
    uint16_t len    = ble_advdata_search_extra(p_encoded_data, data_len, &offset, ad_type);

    if (len == 0)
        return NULL;
    else
        return &p_encoded_data[offset];
}

void rui_ble_scan_adv(int8_t rssi_value, uint8_t *p_adv_data, uint16_t adv_data_len, uint8_t *p_device_mac)
{
    /*
       Company: Apple, Inc.<0x004C>
       Type: Beacon <0x02>
     */
    uint8_t filter_data[]={0x4C, 0x00, 0x02};
    /* Search the pointer of manufacturer specific data */
    uint8_t *p_manu_data = ble_advdata_parse_extra(p_adv_data, adv_data_len, BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA);

    /* Filter ble device by broadcast data */
    if (0x00 == memcmp(filter_data, p_manu_data, 3))
    {
        RUI_LOG_PRINTF("Target beacon mac is %02x-%02x-%02x-%02x-%02x-%02x \n",
                        p_device_mac[5], p_device_mac[4], p_device_mac[3],
                        p_device_mac[2], p_device_mac[1], p_device_mac[0]);
    }
}

RUI_I2C_ST st = {0};

void sensor_on(void)
{

    st.PIN_SDA = 14;
    st.PIN_SCL = 13;
    st.FREQUENCY = RUI_I2C_FREQ_400K;
    rui_i2c_init(&st);

    //lis3dh init
    lis3dh_init();
    //opt3001 init
    opt3001_init();
	//shtc3 init
    SHTC3_Wakeup();
    //lps22hb init 1 wake up
    lps22hb_mode(1);

}

void sensor_off(void)
{
    lis3dh_sleep_init();
    sensorOpt3001Enable(0);
    SHTC3_Sleep();
    lps22hb_mode(0);
}

void main(void)
{
    //system init 
    rui_sensor_register_callback(sensor_on,sensor_off);
    rui_init();

    while(1)
    {
        //do your work here, then call rui_device_sleep(1) to sleep

        //here run system work and do not modify
        rui_running();
    }
}