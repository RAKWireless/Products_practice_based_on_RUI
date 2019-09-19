Name:DS3231
Type:RTC
Bus:I2C
Note: Serial port of 5010 has used IO3 and IO4. The I2C device could only use IO1 and IO2. 
And call rui_device_sleep(1) before your device init.