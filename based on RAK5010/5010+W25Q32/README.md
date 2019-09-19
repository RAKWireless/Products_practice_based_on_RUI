Name:W25Q32
Type:4M Flash
Bus:SPI
Note: Serial port of 5010 has used IO3 and IO4. The SPI device will make it unuseful. 
And call rui_device_sleep(1) before your device init.