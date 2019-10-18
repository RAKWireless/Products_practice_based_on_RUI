//==============================================================================
//    S E N S I R I O N   AG,  Laubisruetistr. 50, CH-8712 Staefa, Switzerland
//==============================================================================
// Project   :  SHTC3 Sample Code (V1.0)
// File      :  shtc3.c (V1.0)
// Author    :  RFU
// Date      :  24-Nov-2017
// Controller:  STM32F100RB
// IDE       :  µVision V5.17.0.0
// Compiler  :  Armcc
// Brief     :  Sensor Layer: Implementation of functions for sensor access.
//==============================================================================
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "rui.h"
#include "shtc3.h"


typedef enum{
  READ_ID            = 0xEFC8, // command: read ID register
  SOFT_RESET         = 0x805D, // soft reset
  SLEEP              = 0xB098, // sleep
  WAKEUP             = 0x3517, // wakeup
  MEAS_T_RH_POLLING  = 0x7866, // meas. read T first, clock stretching disabled
  MEAS_T_RH_CLOCKSTR = 0x7CA2, // meas. read T first, clock stretching enabled
  MEAS_RH_T_POLLING  = 0x58E0, // meas. read RH first, clock stretching disabled
  MEAS_RH_T_CLOCKSTR = 0x5C24  // meas. read RH first, clock stretching enabled
}etCommands;


static uint8_t _Address = 0x70;
extern RUI_I2C_ST st;
//------------------------------------------------------------------------------
void SHTC3_Init(){

    return NRF_SUCCESS;
}


//------------------------------------------------------------------------------
static float SHTC3_CalcTemperature(uint16_t rawValue){
  // calculate temperature [°C]
  // T = -45 + 175 * rawValue / 2^16
  return 175 * (float)rawValue / 65536.0f - 45.0f;
}

//------------------------------------------------------------------------------
static float SHTC3_CalcHumidity(uint16_t rawValue){
  // calculate relative humidity [%RH]
  // RH = rawValue / 2^16 * 100
  return 100 * (float)rawValue / 65536.0f;
}

//------------------------------------------------------------------------------
uint32_t SHTC3_GetTempAndHumi(float *temp, float *humi){

  uint16_t rawValueTemp = 0; // temperature raw value from sensor
  uint16_t rawValueHumi = 0; // humidity raw value from sensor

  // measure, read temperature first, clock streching enabled, read temprature first.

  uint8_t tx[2] = {0x7C, 0xA2};
  rui_i2c_rw(&st,RUI_IF_WRITE,_Address,0,tx,2);
  uint8_t rx[6] = {0};
  rui_i2c_rw(&st,RUI_IF_READ,_Address,0,rx,6);

  rawValueTemp = rx[1] | (rx[0] << 8);
  //rawValueTemp = (100 * rawValueTemp * 175) / 65535 - 45 * 100;
  rawValueHumi = rx[4] | (rx[3] << 8);
  //rawValueHumi = (100 * rawValueHumi * 100) / 65535;

  *temp = SHTC3_CalcTemperature(rawValueTemp);
  *humi = SHTC3_CalcHumidity(rawValueHumi);
}

//------------------------------------------------------------------------------
uint32_t SHTC3_GetId(uint16_t *id){

    uint8_t tx[2] = {0xEF, 0xC8};
    rui_i2c_rw(&st,RUI_IF_WRITE,_Address,0,tx,2);

    int8_t rx[3] = {0};
    rui_i2c_rw(&st,RUI_IF_READ,_Address,0,rx,3);
    *id = (rx[0] << 8) | rx[1];
}

//------------------------------------------------------------------------------
uint32_t SHTC3_Sleep(void) {
    uint8_t tx[2] = {0xB0, 0x98};
    rui_i2c_rw(&st,RUI_IF_WRITE,_Address,0,tx,2);
}

//------------------------------------------------------------------------------
uint32_t SHTC3_Wakeup(void) {
    uint16_t id = 0;
    uint8_t tx[2] = {0x35, 0x17};
    rui_i2c_rw(&st,RUI_IF_WRITE,_Address,0,tx,2);
    delay_ms(500);
    SHTC3_GetId(&id);
    RUI_LOG_PRINTF("shtc3 id = %d",id);
}

//------------------------------------------------------------------------------
uint32_t SHTC3_SoftReset(void){
    uint8_t tx[2] = {0x80, 0x5D};
    rui_i2c_rw(&st,RUI_IF_WRITE,_Address,0,tx,2);
}
