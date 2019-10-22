#ifndef _LPS22HB_H
#define _LPS22HB_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "stdint.h"
#include "stdbool.h"
  
int get_lps22hb_data(float *pressure_data);
int lps22hb_init(void);
uint32_t lps22hb_twi_init(void);
int lps22hb_mode(uint8_t mode);
#ifdef __cplusplus
}
#endif

#endif /* SENSOR_OPT3001_H */
