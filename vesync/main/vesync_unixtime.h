#ifndef _VESYNC_UNIX_TIME_H
#define _VESYNC_UNIX_TIME_H

#include "c_types.h"
#include <stdbool.h>

typedef struct{
	unsigned char nSec;
	unsigned char nMin;
	unsigned char nHour;
	unsigned char nDay;
	unsigned char nWeek;
	unsigned char nMonth;
	unsigned short nYear;
}mytime_struct;
extern mytime_struct utcTime;
extern char time_zone;

void Rtc_SyncSet_Time(unsigned int *unix_time,char zone);
unsigned int Rtc_SyncGet_Time(void);
mytime_struct unix_2_localtime(unsigned int *unix_time,char zone);















































#endif
