/*
************************************************************************************************
* File name        :	HalApiDS3231.c
* Function         :	I2C drive simulate by gpio.
* Author           : 	shaoyang_v
* Date             :	2018/11/08
* Version          :	v0.1
* Description      :	通过GPIO，模拟I2C驱动。实现DS3231模块的基本操作。
* ModifyRecord     :
************************************************************************************************
*/
#include "ds3231.h"
#include "rui.h"

RUI_I2C_ST rui_i2c_ds3231;


void ds3231_init()
{
    rui_i2c_ds3231.PIN_SDA = 28;
    rui_i2c_ds3231.PIN_SCL = 29;
    rui_i2c_ds3231.FREQUENCY = RUI_I2C_FREQ_400K;
    rui_i2c_init(&rui_i2c_ds3231);
}

void ds3231_regs_read(uint8_t reg, uint8_t *data, uint8_t len)
{
    rui_i2c_rw(&rui_i2c_ds3231, RUI_IF_READ, DS3231_ADDR, reg, data, len);
}

void ds3231_regs_write(uint8_t reg, uint8_t *data, uint8_t len)
{
    rui_i2c_rw(&rui_i2c_ds3231, RUI_IF_WRITE, DS3231_ADDR, reg, data, len);
}

uint8_t ds3231_bcd2hex(uint8_t value)
{
	return ((value >> 4) * 10) + (value & 0x0F);// convert BCD(Binary Coded Decimal) to Decimal
}

uint8_t ds3231_hex2bcd(uint8_t value)
{
  return ((value / 10) << 4) + (value % 10);// convert decimal to BCD
}

uint32_t ds3231_get_time(tm *tm_st)
{
	uint8_t timebuf[7];

	ds3231_regs_read(0x00, timebuf, 7);
	tm_st->tm_sec  = ds3231_bcd2hex(timebuf[0]);
	tm_st->tm_min  = ds3231_bcd2hex(timebuf[1]);
	tm_st->tm_hour = ds3231_bcd2hex(timebuf[2]);
	tm_st->tm_mday = ds3231_bcd2hex(timebuf[4]);
	tm_st->tm_mon  = ds3231_bcd2hex(timebuf[5])-1;
	tm_st->tm_year = ds3231_bcd2hex(timebuf[6])+2000-1900;

	return 0;
}

void ds3231_rtc_print(tm tm_st)
{
	ds3231_printf("%4d-%02d-%02d %02d:%02d:%02d \r\n", tm_st.tm_year+1900, tm_st.tm_mon+1, tm_st.tm_mday, tm_st.tm_hour, tm_st.tm_min, tm_st.tm_sec);
}




