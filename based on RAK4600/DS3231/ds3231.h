#ifndef _DS3231_H_
#define _DS3231_H_
#include <stdint.h>


#define ds3231_printf RUI_LOG_PRINTF


#define DS3231_ADDR             0x68
#define DA3231_ADDR_WRITE       (DS3231_ADDR<<1)
#define DS3231_ADDR_READ        (DS3231_ADDR<<1 | 0x01)


typedef struct {
   int tm_sec;         /* seconds,  range 0 to 59          */
   int tm_min;         /* minutes, range 0 to 59           */
   int tm_hour;        /* hours, range 0 to 23             */
   int tm_mday;        /* day of the month, range 1 to 31  */
   int tm_mon;         /* month, range 0 to 11             */
   int tm_year;        /* The number of years since 1900   */
   int tm_wday;        /* day of the week, range 0 to 6    */
   int tm_yday;        /* day in the year, range 0 to 365  */
   int tm_isdst;       /* daylight saving time             */
}tm;


uint32_t ds3231_get_time(tm *tm_st);

uint32_t ds3231_set_time(tm *tm_st);


#endif

