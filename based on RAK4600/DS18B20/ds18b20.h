#ifndef _DS18B20_H_
#define _DS18B20_H_

#include <stdint.h>

#include "rui.h"
extern RUI_GPIO_ST rui_gpio_ds18b20;

#define   ds18b20_delay_us(n)    rui_delay_us(n)

void DS18B20_GetTemp(float *temperature);

#endif

