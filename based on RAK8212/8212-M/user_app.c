#include "rui.h"
#include "lis3dh.h"

RUI_I2C_ST st = {0};

void sensor_on(void)
{

    st.PIN_SDA = 14;
    st.PIN_SCL = 13;
    st.FREQUENCY = RUI_I2C_FREQ_400K;
    rui_i2c_init(&st);

    //lis3dh init
    lis3dh_init();


}

void sensor_off(void)
{
    lis3dh_sleep_init();

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