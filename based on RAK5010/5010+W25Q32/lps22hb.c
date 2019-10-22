
/* -----------------------------------------------------------------------------
*                                          Includes
* ------------------------------------------------------------------------------
*/
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "rui.h"
#include "lps22hb.h"
#include "math.h"
/* -----------------------------------------------------------------------------
*                                           Constants
* ------------------------------------------------------------------------------
*/
#define LPS22HB_ADDR          0x5C
/* Register addresses */
#define REG_CTRL1                      0x10
#define REG_CTRL2                      0x11
#define REG_PRE_XL		                0x28
#define REG_PRE_L		              0x29
#define REG_PRE_H                    0x2A
#define REG_ID                      0x0F

extern RUI_I2C_ST st;
uint32_t lps22hb_twi_init(void)
{

    return NRF_SUCCESS;
}

int lps22hb_init(void)
{
    uint8_t id = 0;
    rui_i2c_rw(&st,RUI_IF_READ,LPS22HB_ADDR,REG_ID,&id,1);
    RUI_LOG_PRINTF("lps22hb id = %d",id);
    return 0;
}
int lps22hb_mode(uint8_t mode)
{
    uint8_t cmd = 0;
    if (mode == 1)//wake up
    {
        cmd = 0x50;
    }
    else    //sleep
    {
        cmd = 0x00;
    }
    rui_i2c_rw(&st,RUI_IF_WRITE,LPS22HB_ADDR,REG_CTRL1,&cmd,1);
    return 0;
}
int get_lps22hb_data(float *pressure_data)
{
    uint8_t p_xl = 0;
    uint8_t p_l = 0;
    uint8_t p_h = 0;
    int32_t tmp = 0;
    bool ret;
    rui_i2c_rw(&st,RUI_IF_READ,LPS22HB_ADDR,REG_PRE_XL, &p_xl,1);
    rui_i2c_rw(&st,RUI_IF_READ,LPS22HB_ADDR,REG_PRE_L, &p_l,1);
    rui_i2c_rw(&st,RUI_IF_READ,LPS22HB_ADDR,REG_PRE_H, &p_h,1);

    tmp = ((uint32_t)p_h << 16) | ((uint32_t)p_l << 8) | p_xl;
    if(tmp & 0x00800000)
    {
        tmp |= 0xFF000000;
    }
    *pressure_data = tmp / 4096.00;
    return ret;
}


