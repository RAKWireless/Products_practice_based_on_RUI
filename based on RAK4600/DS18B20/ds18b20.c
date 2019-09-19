#include "ds18b20.h"



void DS18B20_DQ_IN(void)
{
    rui_gpio_ds18b20.pin_num = 13;
    rui_gpio_ds18b20.dir = RUI_GPIO_PIN_DIR_INPUT;
    rui_gpio_ds18b20.pull = RUI_GPIO_PIN_NOPULL;

    rui_gpio_init(&rui_gpio_ds18b20);
}

void DS18B20_DQ_OUT(void)
{ 
    rui_gpio_ds18b20.pin_num = 13;
    rui_gpio_ds18b20.dir = RUI_GPIO_PIN_DIR_OUTPUT;
    rui_gpio_ds18b20.pull = RUI_GPIO_PIN_NOPULL;

    rui_gpio_init(&rui_gpio_ds18b20);
}

void DS18B20_DQ_H(void)
{
    uint8_t state = 1;
    rui_gpio_rw(RUI_IF_WRITE, &rui_gpio_ds18b20, &state);
}

void DS18B20_DQ_L(void)
{
    uint8_t state = 0;
    rui_gpio_rw(RUI_IF_WRITE, &rui_gpio_ds18b20, &state);
}

uint8_t DS18B20_DQ_READ(void)
{
    uint8_t state;
    rui_gpio_rw(RUI_IF_READ, &rui_gpio_ds18b20, &state);

    return state;
}

void DS18B20_DQ_WRITE(uint8_t state)
{
    rui_gpio_rw(RUI_IF_WRITE, &rui_gpio_ds18b20, &state);
}

void DS18B20_WriteByte(uint8_t WB) 
{
    uint8_t i;

    DS18B20_DQ_H();
    DS18B20_DQ_OUT();
    for (i=8;i>0;i--)
    {
        DS18B20_DQ_L();
        ds18b20_delay_us(5);

        DS18B20_DQ_WRITE((bool)(WB&0x01)) ;
        WB>>=1;
        ds18b20_delay_us(60);
        DS18B20_DQ_H();
        ds18b20_delay_us(5);
    }
}

uint8_t DS18B20_ReadByte(void) 
{
    uint8_t i,tmp;

    tmp=0;
    for (i=8;i>0;i--)
    {
        DS18B20_DQ_OUT();
        DS18B20_DQ_L();
        ds18b20_delay_us(5);
        DS18B20_DQ_H();
        DS18B20_DQ_IN();
        ds18b20_delay_us(5);
        tmp>>=1;
        if(DS18B20_DQ_READ())
            tmp+=0x80;
        ds18b20_delay_us(60);
    }
    return tmp;
}

uint8_t DS18B20_RST(void)
{
    uint8_t bState;

    DS18B20_DQ_H();
    DS18B20_DQ_OUT();
    DS18B20_DQ_L();
    ds18b20_delay_us(500);
    DS18B20_DQ_H();
    DS18B20_DQ_IN();
    ds18b20_delay_us(50);
    bState=DS18B20_DQ_READ();
    ds18b20_delay_us(200);

    return !bState;
}

void DS18B20_GetTemp(float *temperature)
{
    uint8_t TH,TL;
    short wTemp;
    uint8_t flag;

    DS18B20_RST();
    DS18B20_WriteByte(0xcc);
    DS18B20_WriteByte(0x44);

    DS18B20_RST();
    DS18B20_WriteByte(0xcc);
    DS18B20_WriteByte(0xbe);
    TL = DS18B20_ReadByte();
    TH = DS18B20_ReadByte();


    if(TH>7)
    {
        TH=~TH;
        TL=~TL; 
        flag=0;
    }
    else 
        flag=1;

    wTemp=TH; 
    wTemp<<=8;
    wTemp+=TL;
    wTemp=((float)wTemp*0.625 +0.5);

    if(flag)
        *temperature = wTemp;
    else
        *temperature = -wTemp;
} 

