# Products_practice_based_on_RUI+RAK5010
1. app_task.c 	
   This file is the period task which gathers sensors data and send via 2G/3G/4G. User can add
   their code here like new sensors init and work. This task could be set by at command include
   period and on/off.
   
2. user_app.c
   This file is structure of RUI. User can add their code here like timer init. But we advise to
   do something in app_task.c.

3. User sensor
   
   3.1 5010 support 1 x UART or 2 x I2C or 1 x SPI or 1 x ADC or 4 x GPIO, so just choose one type.
   User gpio: Io4, Io3, Io1, Io2
   
   3.2 8212/8212M support 1 x I2C or 3 x ADC or 3 x GPIO.
   User gpio: P28 and P29
   
   For example in 5010, I2C sensor, add in app_task.c
   
   RUI_I2C_ST rui_i2c = {0};
   rui_i2c.PIN_SCL = 19;//Io1
   rui_i2c.PIN_SDA = 20;//Io2
   rui_i2c.FREQUENCY = RUI_I2C_FREQ_100K;
   rui_i2c_init(&rui_i2c);
   uint8_t data[10] = {0};
   //read
   rui_i2c_rw(&rui_i2c,0,devaddr,regaddr,data,len);
   //write
   rui_i2c_rw(&rui_i2c,1,devaddr,regaddr,data,len);
   
   If use your sensor, the VREF must be connected to VCC for connection of all IO.