/*
 ******************************************************************************
 * @file    lis2mdl_reg.c
 * @author  MEMS Software Solution Team
 * @date    06-October-2017
 * @brief   LIS2MDL driver file
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "lis2mdl.h"
#include "rui.h"


#define LIS2MDL_ADDR          0x1E

lis2mdl_ctx_t dev_ctx;
RUI_I2C_ST lis2mdl_st = {0};
uint32_t lis2mdl_twi_init(void)
{
    lis2mdl_st.PIN_SDA = 13;
    lis2mdl_st.PIN_SCL = 11;
    lis2mdl_st.FREQUENCY = RUI_I2C_FREQ_400K;
    rui_i2c_init(&lis2mdl_st);
    return NRF_SUCCESS;
}

static int32_t platform_write(void *handle, uint8_t Reg, uint8_t *Bufp,
                              uint16_t len)
{
    uint32_t err_code;
    err_code = rui_i2c_rw(&lis2mdl_st,RUI_IF_WRITE,LIS2MDL_ADDR,Reg,Bufp,len);
    return err_code;
}


static int32_t platform_read(void *handle, uint8_t Reg, uint8_t *Bufp,
                             uint16_t len)
{
    uint32_t err_code;

    err_code = rui_i2c_rw(&lis2mdl_st,RUI_IF_READ,LIS2MDL_ADDR,Reg,Bufp,len);
    return err_code;
}


int lis2mdl_init(void)
{
    uint32_t err_code;

    //³õÊ¼»¯TWI
    err_code = lis2mdl_twi_init();
    if(err_code != NRF_SUCCESS) return -1;

    /*
     *  Initialize mems driver interface
     */

    dev_ctx.write_reg = platform_write;
    dev_ctx.read_reg = platform_read;

    uint8_t whoamI, rst;
    /*
     *  Check device ID
     */
    whoamI = 0;
    lis2mdl_device_id_get(&dev_ctx, &whoamI);
    if ( whoamI != LIS2MDL_ID )
    {
        //while(1); /*manage here device not found */
        return -1;
    }
    /*
     *  Restore default configuration
     */
    lis2mdl_reset_set(&dev_ctx, PROPERTY_ENABLE);
    do
    {
        lis2mdl_reset_get(&dev_ctx, &rst);
    }
    while (rst);
    /*
     *  Enable Block Data Update
     */
    lis2mdl_block_data_update_set(&dev_ctx, PROPERTY_ENABLE);
    /*
     * Set Output Data Rate
     */
    lis2mdl_data_rate_set(&dev_ctx, LIS2MDL_ODR_10Hz);
    /*
     * Set / Reset sensor mode
     */
    lis2mdl_set_rst_mode_set(&dev_ctx, LIS2MDL_SENS_OFF_CANC_EVERY_ODR);
    /*
     * Enable temperature compensation
     */
    lis2mdl_offset_temp_comp_set(&dev_ctx, PROPERTY_ENABLE);
    /*
     * Set device in continuos mode
     */
    lis2mdl_operating_mode_set(&dev_ctx, LIS2MDL_CONTINUOUS_MODE);

    return 0;
}

int lis2mdl_sleep_init(void)
{
    uint32_t err_code;

    //³õÊ¼»¯TWI
    err_code = lis2mdl_twi_init();
    if(err_code != NRF_SUCCESS) return -1;

    /*
     *  Initialize mems driver interface
     */

    dev_ctx.write_reg = platform_write;
    dev_ctx.read_reg = platform_read;

    uint8_t whoamI, rst;
    /*
     *  Check device ID
     */
    whoamI = 0;
    lis2mdl_device_id_get(&dev_ctx, &whoamI);
    if ( whoamI != LIS2MDL_ID )
    {
        //while(1); /*manage here device not found */
        return -1;
    }
    /*
     *  Restore default configuration
     */
    lis2mdl_reset_set(&dev_ctx, PROPERTY_ENABLE);
    do
    {
        lis2mdl_reset_get(&dev_ctx, &rst);
    }
    while (rst);
    /*
     *  Enable Block Data Update
     */
    lis2mdl_block_data_update_set(&dev_ctx, PROPERTY_ENABLE);
    /*
     * Set Output Data Rate
     */
    lis2mdl_data_rate_set(&dev_ctx, LIS2MDL_ODR_10Hz);
    /*
     * Set / Reset sensor mode
     */
    lis2mdl_set_rst_mode_set(&dev_ctx, LIS2MDL_SENS_OFF_CANC_EVERY_ODR);
    /*
     * Enable temperature compensation
     */
    lis2mdl_offset_temp_comp_set(&dev_ctx, PROPERTY_ENABLE);
    /*
     * Set device in continuos mode
     */
    lis2mdl_operating_mode_set(&dev_ctx, LIS2MDL_POWER_DOWN);

    return 0;
}
void get_lis2mdl_data(float *magnetic_x, float *magnetic_y, float *magnetic_z)
{
    axis3bit16_t data_raw_magnetic;
    axis1bit16_t data_raw_temperature;
    float magnetic_mG[3];
    float temperature_degC;
//	uint8_t whoamI, rst;
//	 /*
//   *  Check device ID
//   */
//  whoamI = 0;
//  lis2mdl_device_id_get(&dev_ctx, &whoamI);
//  if ( whoamI != LIS2MDL_ID )
//    while(1); /*manage here device not found */
//	/*
//   *  Restore default configuration
//   */
//  lis2mdl_reset_set(&dev_ctx, PROPERTY_ENABLE);
//  do {
//    lis2mdl_reset_get(&dev_ctx, &rst);
//  } while (rst);
//  /*
//   *  Enable Block Data Update
//   */
//  lis2mdl_block_data_update_set(&dev_ctx, PROPERTY_ENABLE);
//  /*
//   * Set Output Data Rate
//   */
//  lis2mdl_data_rate_set(&dev_ctx, LIS2MDL_ODR_10Hz);
//  /*
//   * Set / Reset sensor mode
//   */
//  lis2mdl_set_rst_mode_set(&dev_ctx, LIS2MDL_SENS_OFF_CANC_EVERY_ODR);
//  /*
//   * Enable temperature compensation
//   */
//  lis2mdl_offset_temp_comp_set(&dev_ctx, PROPERTY_ENABLE);
//  /*
//   * Set device in continuos mode
//   */
//  lis2mdl_operating_mode_set(&dev_ctx, LIS2MDL_CONTINUOUS_MODE);

    /*
     * Read samples in polling mode (no int)
     */
    uint8_t cnt=1;
    uint8_t valid=0;
    while(cnt--)
    {
        /*
         * Read output only if new value is available
         */
        lis2mdl_reg_t reg;
        lis2mdl_status_get(&dev_ctx, &reg.status_reg);

        if (reg.status_reg.zyxda)
        {
            /* Read magnetic field data */
            memset(data_raw_magnetic.u8bit, 0x00, 3*sizeof(int16_t));
            lis2mdl_magnetic_raw_get(&dev_ctx, data_raw_magnetic.u8bit);
            *magnetic_x = LIS2MDL_FROM_LSB_TO_mG( data_raw_magnetic.i16bit[0]);
            *magnetic_y = LIS2MDL_FROM_LSB_TO_mG( data_raw_magnetic.i16bit[1]);
            *magnetic_z = LIS2MDL_FROM_LSB_TO_mG( data_raw_magnetic.i16bit[2]);

//      sprintf((char*)buf, "%4.2f\t%4.2f\t%4.2f\r\n",
//              magnetic_mG[0], magnetic_mG[1], magnetic_mG[2]);
//      DPRINTF(LOG_INFO, "%s", buf);
            valid = 1;

        }
    }
}

void lis2mdl_deinit()
{
    rak_i2c_deinit();
}

/**
  * @addtogroup  lis2mdl
  * @brief  This file provides a set of functions needed to drive the
  *         lis2mdl enanced inertial module.
  * @{
  */

/**
  * @addtogroup  interfaces_functions
  * @brief  This section provide a set of functions used to read and write
  *         a generic register of the device.
  * @{
  */

/**
  * @brief  Read generic device register
  *
  * @param  lis2mdl_ctx_t* ctx: read / write interface definitions
  * @param  uint8_t reg: register to read
  * @param  uint8_t* data: pointer to buffer that store the data read
  * @param  uint16_t len: number of consecutive register to read
  *
  */
int32_t lis2mdl_read_reg(lis2mdl_ctx_t* ctx, uint8_t reg, uint8_t* data,
                         uint16_t len)
{
    return ctx->read_reg(ctx->handle, reg, data, len);
}

/**
  * @brief  Write generic device register
  *
  * @param  lis2mdl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t reg: register to write
  * @param  uint8_t* data: pointer to data to write in register reg
  * @param  uint16_t len: number of consecutive register to write
  *
*/
int32_t lis2mdl_write_reg(lis2mdl_ctx_t* ctx, uint8_t reg, uint8_t* data,
                          uint16_t len)
{
    return ctx->write_reg(ctx->handle, reg, data, len);
}

/**
  * @}
  */

/**
  * @addtogroup  data_generation
  * @brief   This section group all the functions concerning data generation
  * @{
  */

/**
  * @brief  mag_user_offset: [set]  These registers comprise a 3 group of
  *                                 16-bit number and represent hard-iron
  *                                 offset in order to compensate environmental
  *                                 effects. Data format is the same of
  *                                 output data raw: two’s complement with
  *                                 1LSb = 1.5mG. These values act on the
  *                                 magnetic output data value in order to
  *                                 delete the environmental offset.
  *
  * @param  lis2mdl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t * : buffer that contains data to write
  *
  */
int32_t lis2mdl_mag_user_offset_set(lis2mdl_ctx_t *ctx, uint8_t *buff)
{
    return lis2mdl_read_reg(ctx, LIS2MDL_OFFSET_X_REG_L, buff, 6);
}

/**
  * @brief  mag_user_offset: [get]  These registers comprise a 3 group of
  *                                 16-bit number and represent hard-iron
  *                                 offset in order to compensate environmental
  *                                 effects. Data format is the same of
  *                                 output data raw: two’s complement with
  *                                 1LSb = 1.5mG. These values act on the
  *                                 magnetic output data value in order to
  *                                 delete the environmental offset.
  *
  * @param  lis2mdl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t * : buffer that stores data read
  *
  */
int32_t lis2mdl_mag_user_offset_get(lis2mdl_ctx_t *ctx, uint8_t *buff)
{
    return lis2mdl_read_reg(ctx, LIS2MDL_OFFSET_X_REG_L, buff, 6);
}

/**
  * @brief  operating_mode: [set]  Operating mode selection.
  *
  * @param  lis2mdl_ctx_t *ctx: read / write interface definitions
  * @param  lis2mdl_md_t: change the values of md in reg CFG_REG_A
  *
  */
int32_t lis2mdl_operating_mode_set(lis2mdl_ctx_t *ctx, lis2mdl_md_t val)
{
    lis2mdl_reg_t reg;
    int32_t mm_error;

    mm_error = lis2mdl_read_reg(ctx, LIS2MDL_CFG_REG_A, &reg.byte, 1);
    reg.cfg_reg_a.md = val;
    mm_error = lis2mdl_write_reg(ctx, LIS2MDL_CFG_REG_A, &reg.byte, 1);

    return mm_error;
}

/**
  * @brief  operating_mode: [get]  Operating mode selection.
  *
  * @param  lis2mdl_ctx_t *ctx: read / write interface definitions
  * @param  lis2mdl_md_t: Get the values of md in reg CFG_REG_A
  *
  */
int32_t lis2mdl_operating_mode_get(lis2mdl_ctx_t *ctx, lis2mdl_md_t *val)
{
    lis2mdl_reg_t reg;
    int32_t mm_error;

    mm_error = lis2mdl_read_reg(ctx, LIS2MDL_CFG_REG_A, &reg.byte, 1);
    *val = (lis2mdl_md_t) reg.cfg_reg_a.md;

    return mm_error;
}

/**
  * @brief  data_rate: [set]  Output data rate selection.
  *
  * @param  lis2mdl_ctx_t *ctx: read / write interface definitions
  * @param  lis2mdl_odr_t: change the values of odr in reg CFG_REG_A
  *
  */
int32_t lis2mdl_data_rate_set(lis2mdl_ctx_t *ctx, lis2mdl_odr_t val)
{
    lis2mdl_reg_t reg;
    int32_t mm_error;

    mm_error = lis2mdl_read_reg(ctx, LIS2MDL_CFG_REG_A, &reg.byte, 1);
    reg.cfg_reg_a.odr = val;
    mm_error = lis2mdl_write_reg(ctx, LIS2MDL_CFG_REG_A, &reg.byte, 1);

    return mm_error;
}

/**
  * @brief  data_rate: [get]  Output data rate selection.
  *
  * @param  lis2mdl_ctx_t *ctx: read / write interface definitions
  * @param  lis2mdl_odr_t: Get the values of odr in reg CFG_REG_A
  *
  */
int32_t lis2mdl_data_rate_get(lis2mdl_ctx_t *ctx, lis2mdl_odr_t *val)
{
    lis2mdl_reg_t reg;
    int32_t mm_error;

    mm_error = lis2mdl_read_reg(ctx, LIS2MDL_CFG_REG_A, &reg.byte, 1);
    *val = (lis2mdl_odr_t) reg.cfg_reg_a.odr;

    return mm_error;
}

/**
  * @brief  power_mode: [set]  Enables high-resolution/low-power mode.
  *
  * @param  lis2mdl_ctx_t *ctx: read / write interface definitions
  * @param  lis2mdl_lp_t: change the values of lp in reg CFG_REG_A
  *
  */
int32_t lis2mdl_power_mode_set(lis2mdl_ctx_t *ctx, lis2mdl_lp_t val)
{
    lis2mdl_reg_t reg;
    int32_t mm_error;

    mm_error = lis2mdl_read_reg(ctx, LIS2MDL_CFG_REG_A, &reg.byte, 1);
    reg.cfg_reg_a.lp = val;
    mm_error = lis2mdl_write_reg(ctx, LIS2MDL_CFG_REG_A, &reg.byte, 1);

    return mm_error;
}

/**
  * @brief  power_mode: [get]  Enables high-resolution/low-power mode.
  *
  * @param  lis2mdl_ctx_t *ctx: read / write interface definitions
  * @param  lis2mdl_lp_t: Get the values of lp in reg CFG_REG_A
  *
  */
int32_t lis2mdl_power_mode_get(lis2mdl_ctx_t *ctx, lis2mdl_lp_t *val)
{
    lis2mdl_reg_t reg;
    int32_t mm_error;

    mm_error = lis2mdl_read_reg(ctx, LIS2MDL_CFG_REG_A, &reg.byte, 1);
    *val = (lis2mdl_lp_t) reg.cfg_reg_a.lp;

    return mm_error;
}

/**
  * @brief  offset_temp_comp: [set]  Enables the magnetometer temperature
  *                                  compensation.
  *
  * @param  lis2mdl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of comp_temp_en in reg CFG_REG_A
  *
  */
int32_t lis2mdl_offset_temp_comp_set(lis2mdl_ctx_t *ctx, uint8_t val)
{
    lis2mdl_reg_t reg;
    int32_t mm_error;

    mm_error = lis2mdl_read_reg(ctx, LIS2MDL_CFG_REG_A, &reg.byte, 1);
    reg.cfg_reg_a.comp_temp_en = val;
    mm_error = lis2mdl_write_reg(ctx, LIS2MDL_CFG_REG_A, &reg.byte, 1);

    return mm_error;
}

/**
  * @brief  offset_temp_comp: [get]  Enables the magnetometer temperature
  *                                  compensation.
  *
  * @param  lis2mdl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of comp_temp_en in reg CFG_REG_A
  *
  */
int32_t lis2mdl_offset_temp_comp_get(lis2mdl_ctx_t *ctx, uint8_t *val)
{
    lis2mdl_reg_t reg;
    int32_t mm_error;

    mm_error = lis2mdl_read_reg(ctx, LIS2MDL_CFG_REG_A, &reg.byte, 1);
    *val = reg.cfg_reg_a.comp_temp_en;

    return mm_error;
}

/**
  * @brief  low_pass_bandwidth: [set]  Low-pass bandwidth selection.
  *
  * @param  lis2mdl_ctx_t *ctx: read / write interface definitions
  * @param  lis2mdl_lpf_t: change the values of lpf in reg CFG_REG_B
  *
  */
int32_t lis2mdl_low_pass_bandwidth_set(lis2mdl_ctx_t *ctx,
                                       lis2mdl_lpf_t val)
{
    lis2mdl_reg_t reg;
    int32_t mm_error;

    mm_error = lis2mdl_read_reg(ctx, LIS2MDL_CFG_REG_B, &reg.byte, 1);
    reg.cfg_reg_b.lpf = val;
    mm_error = lis2mdl_write_reg(ctx, LIS2MDL_CFG_REG_B, &reg.byte, 1);

    return mm_error;
}

/**
  * @brief  low_pass_bandwidth: [get]  Low-pass bandwidth selection.
  *
  * @param  lis2mdl_ctx_t *ctx: read / write interface definitions
  * @param  lis2mdl_lpf_t: Get the values of lpf in reg CFG_REG_B
  *
  */
int32_t lis2mdl_low_pass_bandwidth_get(lis2mdl_ctx_t *ctx,
                                       lis2mdl_lpf_t *val)
{
    lis2mdl_reg_t reg;
    int32_t mm_error;

    mm_error = lis2mdl_read_reg(ctx, LIS2MDL_CFG_REG_B, &reg.byte, 1);
    *val = (lis2mdl_lpf_t) reg.cfg_reg_b.lpf;

    return mm_error;
}

/**
  * @brief  set_rst_mode: [set]
  *
  * @param  lis2mdl_ctx_t *ctx: read / write interface definitions
  * @param  lis2mdl_set_rst_t: change the values of set_rst in
  *                            reg CFG_REG_B
  *
  */
int32_t lis2mdl_set_rst_mode_set(lis2mdl_ctx_t *ctx, lis2mdl_set_rst_t val)
{
    lis2mdl_reg_t reg;
    int32_t mm_error;

    mm_error = lis2mdl_read_reg(ctx, LIS2MDL_CFG_REG_B, &reg.byte, 1);
    reg.cfg_reg_b.set_rst = val;
    mm_error = lis2mdl_write_reg(ctx, LIS2MDL_CFG_REG_B, &reg.byte, 1);

    return mm_error;
}

/**
  * @brief  set_rst_mode: [get]
  *
  * @param  lis2mdl_ctx_t *ctx: read / write interface definitions
  * @param  lis2mdl_set_rst_t: Get the values of set_rst in reg CFG_REG_B
  *
  */
int32_t lis2mdl_set_rst_mode_get(lis2mdl_ctx_t *ctx, lis2mdl_set_rst_t *val)
{
    lis2mdl_reg_t reg;
    int32_t mm_error;

    mm_error = lis2mdl_read_reg(ctx, LIS2MDL_CFG_REG_B, &reg.byte, 1);
    *val = (lis2mdl_set_rst_t) reg.cfg_reg_b.set_rst;

    return mm_error;
}

/**
  * @brief   set_rst_sensor_single: [set] Enables offset cancellation
  *                                       in single measurement mode.
  *                                       The OFF_CANC bit must be set
  *                                       to 1 when enabling offset
  *                                       cancellation in single measurement
  *                                       mode this means a call function:
  *                                       set_rst_mode(SENS_OFF_CANC_EVERY_ODR)
  *                                       is need.
  *
  * @param  lis2mdl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of off_canc_one_shot in
  *                      reg CFG_REG_B
  *
  */
int32_t lis2mdl_set_rst_sensor_single_set(lis2mdl_ctx_t *ctx, uint8_t val)
{
    lis2mdl_reg_t reg;
    int32_t mm_error;

    mm_error = lis2mdl_read_reg(ctx, LIS2MDL_CFG_REG_B, &reg.byte, 1);
    reg.cfg_reg_b.off_canc_one_shot = val;
    mm_error = lis2mdl_write_reg(ctx, LIS2MDL_CFG_REG_B, &reg.byte, 1);

    return mm_error;
}

/**
  * @brief   set_rst_sensor_single: [get] Enables offset cancellation
  *                                       in single measurement mode.
  *                                       The OFF_CANC bit must be set to
  *                                       1 when enabling offset cancellation
  *                                       in single measurement mode this
  *                                       means a call function:
  *                                       set_rst_mode(SENS_OFF_CANC_EVERY_ODR)
  *                                       is need.
  *
  * @param  lis2mdl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of off_canc_one_shot in reg CFG_REG_B
  *
  */
int32_t lis2mdl_set_rst_sensor_single_get(lis2mdl_ctx_t *ctx, uint8_t *val)
{
    lis2mdl_reg_t reg;
    int32_t mm_error;

    mm_error = lis2mdl_read_reg(ctx, LIS2MDL_CFG_REG_B, &reg.byte, 1);
    *val = reg.cfg_reg_b.off_canc_one_shot;

    return mm_error;
}

/**
  * @brief  block_data_update: [set] Blockdataupdate.
  *
  * @param  lis2mdl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of bdu in reg CFG_REG_C
  *
  */
int32_t lis2mdl_block_data_update_set(lis2mdl_ctx_t *ctx, uint8_t val)
{
    lis2mdl_reg_t reg;
    int32_t mm_error;

    mm_error = lis2mdl_read_reg(ctx, LIS2MDL_CFG_REG_C, &reg.byte, 1);
    reg.cfg_reg_c.bdu = val;
    mm_error = lis2mdl_write_reg(ctx, LIS2MDL_CFG_REG_C, &reg.byte, 1);

    return mm_error;
}

/**
  * @brief  block_data_update: [get] Blockdataupdate.
  *
  * @param  lis2mdl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of bdu in reg CFG_REG_C
  *
  */
int32_t lis2mdl_block_data_update_get(lis2mdl_ctx_t *ctx, uint8_t *val)
{
    lis2mdl_reg_t reg;
    int32_t mm_error;

    mm_error = lis2mdl_read_reg(ctx, LIS2MDL_CFG_REG_C, &reg.byte, 1);
    *val = reg.cfg_reg_c.bdu;

    return mm_error;
}

/**
  * @brief  mag_data_ready: [get]  Magnetic set of data available.
  *
  * @param  lis2mdl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of zyxda in reg STATUS_REG
  *
  */
int32_t lis2mdl_mag_data_ready_get(lis2mdl_ctx_t *ctx, uint8_t *val)
{
    lis2mdl_reg_t reg;
    int32_t mm_error;

    mm_error = lis2mdl_read_reg(ctx, LIS2MDL_STATUS_REG, &reg.byte, 1);
    *val = reg.status_reg.zyxda;

    return mm_error;
}

/**
  * @brief  mag_data_ovr: [get]  Magnetic set of data overrun.
  *
  * @param  lis2mdl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of zyxor in reg STATUS_REG
  *
  */
int32_t lis2mdl_mag_data_ovr_get(lis2mdl_ctx_t *ctx, uint8_t *val)
{
    lis2mdl_reg_t reg;
    int32_t mm_error;

    mm_error = lis2mdl_read_reg(ctx, LIS2MDL_STATUS_REG, &reg.byte, 1);
    *val = reg.status_reg.zyxor;

    return mm_error;
}

/**
  * @brief  magnetic_raw: [get]  Magnetic output value.
  *
  * @param  lis2mdl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t * : buffer that stores data read
  *
  */
int32_t lis2mdl_magnetic_raw_get(lis2mdl_ctx_t *ctx, uint8_t *buff)
{
    return lis2mdl_read_reg(ctx, LIS2MDL_OUTX_L_REG, buff, 6);
}

/**
  * @brief  temperature_raw: [get]  Temperature output value.
  *
  * @param  lis2mdl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t * : buffer that stores data read
  *
  */
int32_t lis2mdl_temperature_raw_get(lis2mdl_ctx_t *ctx, uint8_t *buff)
{
    return lis2mdl_read_reg(ctx, LIS2MDL_TEMP_OUT_L_REG, buff, 2);
}

/**
  * @}
  */

/**
  * @addtogroup  common
  * @brief   This section group common usefull functions
  * @{
  */

/**
  * @brief  device_id: [get] DeviceWhoamI.
  *
  * @param  lis2mdl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t * : buffer that stores data read
  *
  */
int32_t lis2mdl_device_id_get(lis2mdl_ctx_t *ctx, uint8_t *buff)
{
    return lis2mdl_read_reg(ctx, LIS2MDL_WHO_AM_I, buff, 1);
}

/**
  * @brief  reset: [set]  Software reset. Restore the default values in
  *                       user registers.
  *
  * @param  lis2mdl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of soft_rst in reg CFG_REG_A
  *
  */
int32_t lis2mdl_reset_set(lis2mdl_ctx_t *ctx, uint8_t val)
{
    lis2mdl_reg_t reg;
    int32_t mm_error;

    mm_error = lis2mdl_read_reg(ctx, LIS2MDL_CFG_REG_A, &reg.byte, 1);
    reg.cfg_reg_a.soft_rst = val;
    mm_error = lis2mdl_write_reg(ctx, LIS2MDL_CFG_REG_A, &reg.byte, 1);

    return mm_error;
}

/**
  * @brief  reset: [get]  Software reset. Restore the default values
  *                       in user registers.
  *
  * @param  lis2mdl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of soft_rst in reg CFG_REG_A
  *
  */
int32_t lis2mdl_reset_get(lis2mdl_ctx_t *ctx, uint8_t *val)
{
    lis2mdl_reg_t reg;
    int32_t mm_error;

    mm_error = lis2mdl_read_reg(ctx, LIS2MDL_CFG_REG_A, &reg.byte, 1);
    *val = reg.cfg_reg_a.soft_rst;

    return mm_error;
}

/**
  * @brief  boot: [set]  Reboot memory content. Reload the calibration
  *                      parameters.
  *
  * @param  lis2mdl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of reboot in reg CFG_REG_A
  *
  */
int32_t lis2mdl_boot_set(lis2mdl_ctx_t *ctx, uint8_t val)
{
    lis2mdl_reg_t reg;
    int32_t mm_error;

    mm_error = lis2mdl_read_reg(ctx, LIS2MDL_CFG_REG_A, &reg.byte, 1);
    reg.cfg_reg_a.reboot = val;
    mm_error = lis2mdl_write_reg(ctx, LIS2MDL_CFG_REG_A, &reg.byte, 1);

    return mm_error;
}

/**
  * @brief  boot: [get]  Reboot memory content. Reload the
  *                      calibration parameters.
  *
  * @param  lis2mdl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of reboot in reg CFG_REG_A
  *
  */
int32_t lis2mdl_boot_get(lis2mdl_ctx_t *ctx, uint8_t *val)
{
    lis2mdl_reg_t reg;
    int32_t mm_error;

    mm_error = lis2mdl_read_reg(ctx, LIS2MDL_CFG_REG_A, &reg.byte, 1);
    *val = reg.cfg_reg_a.reboot;

    return mm_error;
}

/**
  * @brief  self_test: [set] Selftest.
  *
  * @param  lis2mdl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of self_test in reg CFG_REG_C
  *
  */
int32_t lis2mdl_self_test_set(lis2mdl_ctx_t *ctx, uint8_t val)
{
    lis2mdl_reg_t reg;
    int32_t mm_error;

    mm_error = lis2mdl_read_reg(ctx, LIS2MDL_CFG_REG_C, &reg.byte, 1);
    reg.cfg_reg_c.self_test = val;
    mm_error = lis2mdl_write_reg(ctx, LIS2MDL_CFG_REG_C, &reg.byte, 1);

    return mm_error;
}

/**
  * @brief  self_test: [get] Selftest.
  *
  * @param  lis2mdl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of self_test in reg CFG_REG_C
  *
  */
int32_t lis2mdl_self_test_get(lis2mdl_ctx_t *ctx, uint8_t *val)
{
    lis2mdl_reg_t reg;
    int32_t mm_error;

    mm_error = lis2mdl_read_reg(ctx, LIS2MDL_CFG_REG_C, &reg.byte, 1);
    *val = reg.cfg_reg_c.self_test;

    return mm_error;
}

/**
  * @brief  data_format: [set]  Big/Little Endian data selection.
  *
  * @param  lis2mdl_ctx_t *ctx: read / write interface definitions
  * @param  lis2mdl_ble_t: change the values of ble in reg CFG_REG_C
  *
  */
int32_t lis2mdl_data_format_set(lis2mdl_ctx_t *ctx, lis2mdl_ble_t val)
{
    lis2mdl_reg_t reg;
    int32_t mm_error;

    mm_error = lis2mdl_read_reg(ctx, LIS2MDL_CFG_REG_C, &reg.byte, 1);
    reg.cfg_reg_c.ble = val;
    mm_error = lis2mdl_write_reg(ctx, LIS2MDL_CFG_REG_C, &reg.byte, 1);

    return mm_error;
}

/**
  * @brief  data_format: [get]  Big/Little Endian data selection.
  *
  * @param  lis2mdl_ctx_t *ctx: read / write interface definitions
  * @param  lis2mdl_ble_t: Get the values of ble in reg CFG_REG_C
  *
  */
int32_t lis2mdl_data_format_get(lis2mdl_ctx_t *ctx, lis2mdl_ble_t *val)
{
    lis2mdl_reg_t reg;
    int32_t mm_error;

    mm_error = lis2mdl_read_reg(ctx, LIS2MDL_CFG_REG_C, &reg.byte, 1);
    *val = (lis2mdl_ble_t) reg.cfg_reg_c.ble;

    return mm_error;
}

/**
  * @brief  status: [get]  Info about device status.
  *
  * @param  lis2mdl_ctx_t *ctx: read / write interface definitions
  * @param  lis2mdl_status_reg_t: registers STATUS_REG
  *
  */
int32_t lis2mdl_status_get(lis2mdl_ctx_t *ctx, lis2mdl_status_reg_t *val)
{
    return lis2mdl_read_reg(ctx, LIS2MDL_STATUS_REG, (uint8_t*) val, 1);
}

/**
  * @}
  */

/**
  * @addtogroup  interrupts
  * @brief   This section group all the functions that manage interrupts
  * @{
  */

/**
  * @brief  offset_int_conf: [set]  The interrupt block recognition checks
  *                                 data after/before the hard-iron correction
  *                                 to discover the interrupt.
  *
  * @param  lis2mdl_ctx_t *ctx: read / write interface definitions
  * @param  lis2mdl_int_on_dataoff_t: change the values of int_on_dataoff in
  *                                   reg CFG_REG_B
  *
  */
int32_t lis2mdl_offset_int_conf_set(lis2mdl_ctx_t *ctx,
                                    lis2mdl_int_on_dataoff_t val)
{
    lis2mdl_reg_t reg;
    int32_t mm_error;

    mm_error = lis2mdl_read_reg(ctx, LIS2MDL_CFG_REG_B, &reg.byte, 1);
    reg.cfg_reg_b.int_on_dataoff = val;
    mm_error = lis2mdl_write_reg(ctx, LIS2MDL_CFG_REG_B, &reg.byte, 1);

    return mm_error;
}

/**
  * @brief  offset_int_conf: [get]  The interrupt block recognition checks
  *                                 data after/before the hard-iron correction
  *                                 to discover the interrupt.
  *
  * @param  lis2mdl_ctx_t *ctx: read / write interface definitions
  * @param  lis2mdl_int_on_dataoff_t: Get the values of int_on_dataoff in
  *                                   reg CFG_REG_B
  *
  */
int32_t lis2mdl_offset_int_conf_get(lis2mdl_ctx_t *ctx,
                                    lis2mdl_int_on_dataoff_t *val)
{
    lis2mdl_reg_t reg;
    int32_t mm_error;

    mm_error = lis2mdl_read_reg(ctx, LIS2MDL_CFG_REG_B, &reg.byte, 1);
    *val = (lis2mdl_int_on_dataoff_t) reg.cfg_reg_b.int_on_dataoff;

    return mm_error;
}

/**
  * @brief  drdy_on_pin: [set]  Data-ready signal on INT_DRDY pin.
  *
  * @param  lis2mdl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of drdy_on_pin in reg CFG_REG_C
  *
  */
int32_t lis2mdl_drdy_on_pin_set(lis2mdl_ctx_t *ctx, uint8_t val)
{
    lis2mdl_reg_t reg;
    int32_t mm_error;

    mm_error = lis2mdl_read_reg(ctx, LIS2MDL_CFG_REG_C, &reg.byte, 1);
    reg.cfg_reg_c.drdy_on_pin = val;
    mm_error = lis2mdl_write_reg(ctx, LIS2MDL_CFG_REG_C, &reg.byte, 1);

    return mm_error;
}

/**
  * @brief  drdy_on_pin: [get]  Data-ready signal on INT_DRDY pin.
  *
  * @param  lis2mdl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of drdy_on_pin in reg CFG_REG_C
  *
  */
int32_t lis2mdl_drdy_on_pin_get(lis2mdl_ctx_t *ctx, uint8_t *val)
{
    lis2mdl_reg_t reg;
    int32_t mm_error;

    mm_error = lis2mdl_read_reg(ctx, LIS2MDL_CFG_REG_C, &reg.byte, 1);
    *val = reg.cfg_reg_c.drdy_on_pin;

    return mm_error;
}

/**
  * @brief  int_on_pin: [set]  Interrupt signal on INT_DRDY pin.
  *
  * @param  lis2mdl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of int_on_pin in reg CFG_REG_C
  *
  */
int32_t lis2mdl_int_on_pin_set(lis2mdl_ctx_t *ctx, uint8_t val)
{
    lis2mdl_reg_t reg;
    int32_t mm_error;

    mm_error = lis2mdl_read_reg(ctx, LIS2MDL_CFG_REG_C, &reg.byte, 1);
    reg.cfg_reg_c.int_on_pin = val;
    mm_error = lis2mdl_write_reg(ctx, LIS2MDL_CFG_REG_C, &reg.byte, 1);

    return mm_error;
}

/**
  * @brief  int_on_pin: [get]  Interrupt signal on INT_DRDY pin.
  *
  * @param  lis2mdl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of int_on_pin in reg CFG_REG_C
  *
  */
int32_t lis2mdl_int_on_pin_get(lis2mdl_ctx_t *ctx, uint8_t *val)
{
    lis2mdl_reg_t reg;
    int32_t mm_error;

    mm_error = lis2mdl_read_reg(ctx, LIS2MDL_CFG_REG_C, &reg.byte, 1);
    *val = reg.cfg_reg_c.int_on_pin;

    return mm_error;
}

/**
  * @brief  int_gen_conf: [set]  Interrupt generator configuration register
  *
  * @param  lis2mdl_ctx_t *ctx: read / write interface definitions
  * @param  lis2mdl_int_crtl_reg_t: registers INT_CRTL_REG
  *
  */
int32_t lis2mdl_int_gen_conf_set(lis2mdl_ctx_t *ctx,
                                 lis2mdl_int_crtl_reg_t *val)
{
    return lis2mdl_read_reg(ctx, LIS2MDL_INT_CRTL_REG, (uint8_t*) val, 1);
}

/**
  * @brief  int_gen_conf: [get]  Interrupt generator configuration register
  *
  * @param  lis2mdl_ctx_t *ctx: read / write interface definitions
  * @param  lis2mdl_int_crtl_reg_t: registers INT_CRTL_REG
  *
  */
int32_t lis2mdl_int_gen_conf_get(lis2mdl_ctx_t *ctx,
                                 lis2mdl_int_crtl_reg_t *val)
{
    return lis2mdl_read_reg(ctx, LIS2MDL_INT_CRTL_REG, (uint8_t*) val, 1);
}

/**
  * @brief  int_gen_source: [get]  Interrupt generator source register
  *
  * @param  lis2mdl_ctx_t *ctx: read / write interface definitions
  * @param  lis2mdl_int_source_reg_t: registers INT_SOURCE_REG
  *
  */
int32_t lis2mdl_int_gen_source_get(lis2mdl_ctx_t *ctx,
                                   lis2mdl_int_source_reg_t *val)
{
    return lis2mdl_read_reg(ctx, LIS2MDL_INT_SOURCE_REG, (uint8_t*) val, 1);
}

/**
  * @brief  int_gen_treshold: [set]  User-defined threshold value for xl
  *                                  interrupt event on generator.
  *                                  Data format is the same of output
  *                                  data raw: two’s complement with
  *                                  1LSb = 1.5mG.
  *
  * @param  lis2mdl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t * : buffer that contains data to write
  *
  */
int32_t lis2mdl_int_gen_treshold_set(lis2mdl_ctx_t *ctx, uint8_t *buff)
{
    return lis2mdl_read_reg(ctx, LIS2MDL_INT_THS_L_REG, buff, 2);
}

/**
  * @brief  int_gen_treshold: [get]  User-defined threshold value for
  *                                  xl interrupt event on generator.
  *                                  Data format is the same of output
  *                                  data raw: two’s complement with
  *                                  1LSb = 1.5mG.
  *
  * @param  lis2mdl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t * : buffer that stores data read
  *
  */
int32_t lis2mdl_int_gen_treshold_get(lis2mdl_ctx_t *ctx, uint8_t *buff)
{
    return lis2mdl_read_reg(ctx, LIS2MDL_INT_THS_L_REG, buff, 2);
}

/**
  * @}
  */

/**
  * @addtogroup  serial_interface
  * @brief   This section group all the functions concerning serial
  *          interface management
  * @{
  */

/**
  * @brief  i2c_interface: [set]  Enable/Disable I2C interface.
  *
  * @param  lis2mdl_ctx_t *ctx: read / write interface definitions
  * @param  lis2mdl_i2c_dis_t: change the values of i2c_dis in reg CFG_REG_C
  *
  */
int32_t lis2mdl_i2c_interface_set(lis2mdl_ctx_t *ctx, lis2mdl_i2c_dis_t val)
{
    lis2mdl_reg_t reg;
    int32_t mm_error;

    mm_error = lis2mdl_read_reg(ctx, LIS2MDL_CFG_REG_C, &reg.byte, 1);
    reg.cfg_reg_c.i2c_dis = val;
    mm_error = lis2mdl_write_reg(ctx, LIS2MDL_CFG_REG_C, &reg.byte, 1);

    return mm_error;
}

/**
  * @brief  i2c_interface: [get]  Enable/Disable I2C interface.
  *
  * @param  lis2mdl_ctx_t *ctx: read / write interface definitions
  * @param  lis2mdl_i2c_dis_t: Get the values of i2c_dis in reg CFG_REG_C
  *
  */
int32_t lis2mdl_i2c_interface_get(lis2mdl_ctx_t *ctx, lis2mdl_i2c_dis_t *val)
{
    lis2mdl_reg_t reg;
    int32_t mm_error;

    mm_error = lis2mdl_read_reg(ctx, LIS2MDL_CFG_REG_C, &reg.byte, 1);
    *val = (lis2mdl_i2c_dis_t) reg.cfg_reg_c.i2c_dis;

    return mm_error;
}

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/