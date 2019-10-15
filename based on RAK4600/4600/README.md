# Products_practice_based_on_RUI+RAK4600
1. app_task.c
   This file is the period task which gathers sensors data and send via 2G/3G/4G. User can add
   their code here like new sensors init and work. This task could be set by at command include
   period and on/off.

2. user_app.c
   This file is structure of RUI. User can add their code here like timer init. But we advise to
   do something in app_task.c.

3. User sensor

   3.1 RAK4600 support 1 x UART or 1 x I2C or 1 x SPI or 10 x GPIO, so just choose one type.

   3.2 Here is the sample code.

```C
RUI_GPIO_ST rui_gpio;

void gpio_demo_init(void)
{
    rui_gpio.pin_num = 17;
    rui_gpio.dir = RUI_GPIO_PIN_DIR_OUTPUT;
    rui_gpio.pull = RUI_GPIO_PIN_NOPULL;
    rui_gpio_init(&rui_gpio);
}

void gpio_demo_running(void)
{
    uint8_t pin_value=0;
    
    rui_gpio_rw(RUI_IF_WRITE, &rui_gpio, &pin_value);
}
```



```c
RUI_TIMER_ST rui_timer;

void timer_callback(void)
{
	// add your code here...    
}

void timer_demo_init()
{
    rui_timer.timer_mode = RUI_TIMER_MODE_REPEATED;
    rui_timer_init(&rui_timer, timer_callback);
    rui_timer_setvalue(&rui_timer, 200);
    rui_timer_start(&rui_timer);
}
```



```C
RUI_I2C_ST rui_i2c;

void i2c_demo_init(void)
{
    rui_i2c.PIN_SDA = 13;
    rui_i2c.PIN_SCL = 12;
    rui_i2c.FREQUENCY = RUI_I2C_FREQ_400K;
    rui_i2c_init(&rui_i2c);
}

void i2c_demo_running()
{
    uint8_t buf[2];

    rui_i2c_rw(&rui_i2c, RUI_IF_READ, 0x68, 0x00, buf, sizeof(buf));
    RUI_LOG_PRINTF("Reg value is: %02X %02x.\r\n", buf[0], buf[1]);
}
```



```C
RUI_SPI_ST rui_spi={0};

void spi_init(void)
{
    rui_spi.PIN_CS = 17;
    rui_spi.PIN_SCL = 13;
    rui_spi.PIN_MISO = 12;
    rui_spi.PIN_MOSI = 14;
    
    rui_spi_init(&rui_spi);
}

void spi_running(void)
{
    uint32_t err_code;
    uint8_t cmd[6]={0x90, 0x00, 0x00, 0x00, 0xFF, 0xFF};
    uint8_t data[6]={0xFF};
    
    rui_spi_rw(cmd, 6, data, 6);
    NRF_LOG_INFO("SPI transfer success. Reg data is %02X%02X\r\n", data[4],data[5]);
}
```

