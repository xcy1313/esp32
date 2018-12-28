
#include "vesync_unixtime.h"
#include "esp_err.h"
#include <time.h>  
#include "esp_log.h"

static const char *TAG = "vesync_rtc";

#define RTC_Write(cnt,dev_addr,reg_addr,reg_data)	    1							
#define RTC_Read(cnt,dev_addr,reg_addr,reg_data)	    1

#define UTC_BASE_YEAR 			1970
#define MONTH_PER_YEAR 			12
#define DAY_PER_YEAR 			365
#define SEC_PER_DAY 			86400
#define SEC_PER_HOUR 			3600
#define SEC_PER_MIN 			60

#define bcd_2_hex(x)		((((x)/10)<<4)|(((x)%10)&0xf))
#define hex_2_bcd(x)		((((x)/16)*10)+((x)%16))

#define CHINA_TIME				0x8

char time_zone = CHINA_TIME;

const unsigned char g_day_per_mon[MONTH_PER_YEAR] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

mytime_struct utcTime;
/*
 * ���ܣ�
 *     �ж��Ƿ�������
 * ������
 *     year����Ҫ�жϵ������
 *
 * ����ֵ��
 *     ���귵��1�����򷵻�0
 */
static unsigned char applib_dt_is_leap_year(unsigned short year)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if ((year % 400) == 0) {
        return 1;
    } else if ((year % 100) == 0) {
        return 0;
    } else if ((year % 4) == 0) {
        return 1;
    } else {
        return 0;
    }
}

/*
 * ���ܣ�
 *     �õ�ÿ�����ж�����
 * ������
 *     month����Ҫ�õ��������·���
 *     year����������Ӧ�������
 *
 * ����ֵ��
 *     �����ж�����
 *
 */
static unsigned char applib_dt_last_day_of_mon(unsigned char month, unsigned short year)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if ((month == 0) || (month > 12)){
        return g_day_per_mon[1] + applib_dt_is_leap_year(year);
    }

    if (month != 2) {
        return g_day_per_mon[month - 1];
    } else {
        return g_day_per_mon[1] + applib_dt_is_leap_year(year);
    }
}

/*
 * ���ܣ�
 *     ���ݸ��������ڵõ���Ӧ������
 * ������
 *     year�����������
 *     month���������·�
 *     day������������
 *
 * ����ֵ��
 *     ��Ӧ����������0 - ������ ... 6 - ������
 */
static unsigned char applib_dt_dayindex(unsigned short year, unsigned char month, unsigned char day)
{
    char century_code, year_code, month_code, day_code;
    int week = 0;

    century_code = year_code = month_code = day_code = 0;

    if (month == 1 || month == 2) {
        century_code = (year - 1) / 100;
        year_code = (year - 1) % 100;
        month_code = month + 12;
        day_code = day;
    } else {
        century_code = year / 100;
        year_code = year % 100;
        month_code = month;
        day_code = day;
    }

    /* ���ݲ��չ�ʽ�������� */
    week = year_code + year_code / 4 + century_code / 4 - 2 * century_code + 26 * ( month_code + 1 ) / 10 + day_code - 1;
    week = week > 0 ? (week % 7) : ((week % 7) + 7);

    return week;
}

/*
 * ���ܣ�
 *     ����UTCʱ����õ���Ӧ������
 * ������
 *     utc_sec��������UTCʱ���
 *     result��������Ľ��
 *     daylightSaving���Ƿ�������ʱ
 *
 * ����ֵ��
 *     ��
 */
static void utc_sec_2_mytime(unsigned int utc_sec, mytime_struct *result, bool daylightSaving)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    int sec, day;
    unsigned short y;
    unsigned char m;
    unsigned short d;
    //unsigned char dst;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/

    if (daylightSaving) {
        utc_sec += SEC_PER_HOUR;
    }

    /* hour, min, sec */
    /* hour */
    sec = utc_sec % SEC_PER_DAY;
    result->nHour = sec / SEC_PER_HOUR;

    /* min */
    sec %= SEC_PER_HOUR;
    result->nMin = sec / SEC_PER_MIN;

    /* sec */
    result->nSec = sec % SEC_PER_MIN;

    /* year, month, day */
    /* year */
    /* year */
    day = utc_sec / SEC_PER_DAY;
    for (y = UTC_BASE_YEAR; day > 0; y++) {
        d = (DAY_PER_YEAR + applib_dt_is_leap_year(y));
        if (day >= d){
            day -= d;
        }
        else{
            break;
        }
    }

    result->nYear = y;

    for (m = 1; m < MONTH_PER_YEAR; m++) {
        d = applib_dt_last_day_of_mon(m, y);
        if (day >= d) {
            day -= d;
        } else {
            break;
        }
    }
		//result->nZone = 
    result->nMonth = m;
    result->nDay = (unsigned char) (day + 1);
    /* ���ݸ��������ڵõ���Ӧ������ */
    result->nWeek = applib_dt_dayindex(result->nYear, result->nMonth, result->nDay);
}


/*
 * ���ܣ�
 *     ����ʱ������UTCʱ���
 * ������
 *     currTime��������ʱ��
 *     daylightSaving���Ƿ�������ʱ
 *
 * ����ֵ��
 *     UTCʱ���
 */
static unsigned int mytime_2_utc_sec(mytime_struct *currTime, bool daylightSaving)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    unsigned short i;
    unsigned int no_of_days = 0;
    int utc_time;
    unsigned char dst;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if (currTime->nYear < UTC_BASE_YEAR){
        return 0;
    }

    /* year */
    for (i = UTC_BASE_YEAR; i < currTime->nYear; i++){
        no_of_days += (DAY_PER_YEAR + applib_dt_is_leap_year(i));
    }

    /* month */
    for (i = 1; i < currTime->nMonth; i++){
        no_of_days += applib_dt_last_day_of_mon((unsigned char) i, currTime->nYear);
    }

    /* day */
    no_of_days += (currTime->nDay - 1);

    /* sec */
    utc_time = (unsigned int) no_of_days * SEC_PER_DAY + (unsigned int) (currTime->nHour * SEC_PER_HOUR +
                                                                currTime->nMin * SEC_PER_MIN + currTime->nSec);

    if (dst && daylightSaving){
        utc_time -= SEC_PER_HOUR;
    }

    return utc_time;
}

static mytime_struct scale_trans_hex(mytime_struct *p_data ,unsigned char len)
{
	mytime_struct utc_time ;
	
	utc_time.nYear = bcd_2_hex((uint8_t)(p_data->nYear-2000));
	utc_time.nSec = bcd_2_hex(p_data->nSec);
	utc_time.nMonth = bcd_2_hex(p_data->nMonth);
	utc_time.nWeek = bcd_2_hex(p_data->nWeek);
	utc_time.nMin = bcd_2_hex(p_data->nMin);
	utc_time.nHour = bcd_2_hex(p_data->nHour);
	utc_time.nDay = bcd_2_hex(p_data->nDay);
	
    ESP_LOGI(TAG, "nYear[%d] nMonth[%d] nWeek[%d] nDay[%d] nHour[%d] nMin[%d] nSec[%d\r\n" ,
                    utc_time.nYear,utc_time.nMonth,utc_time.nWeek,utc_time.nDay,utc_time.nHour,utc_time.nMin,utc_time.nSec);

	return utc_time;
}

static mytime_struct scale_trans_bcd(mytime_struct *p_data ,unsigned char len)
{
	mytime_struct utc_time ;
	
	utc_time.nYear = hex_2_bcd((uint8_t)(p_data->nYear))+2000;
	utc_time.nSec = hex_2_bcd(p_data->nSec);
	utc_time.nMonth = hex_2_bcd(p_data->nMonth);
	utc_time.nWeek = hex_2_bcd(p_data->nWeek);
	utc_time.nMin = hex_2_bcd(p_data->nMin);
	utc_time.nHour = hex_2_bcd(p_data->nHour);
	utc_time.nDay = hex_2_bcd(p_data->nDay);
	
	return utc_time;
}

mytime_struct unix_2_localtime(unsigned int *unix_time,char zone){
	mytime_struct my_time;

	utc_sec_2_mytime((*unix_time) + zone * SEC_PER_HOUR, &my_time, false);

	my_time = scale_trans_hex(&my_time,sizeof(mytime_struct));

	printf("\r\n");
	ESP_LOGI(TAG, "year[%02x] month[%02x] day[%02x] week[%02x]  hour[%02x]  min[%02x]  sec[%02x]",my_time.nYear,my_time.nMonth,my_time.nDay,my_time.nWeek,my_time.nHour,my_time.nMin,my_time.nSec);
	printf("\r\n");

	return my_time;
}

//���ñ���ʱ��;
void Rtc_SyncSet_Time(unsigned int *unix_time,char zone){
	mytime_struct my_time;
	
	utc_sec_2_mytime((*unix_time) + zone * SEC_PER_HOUR, &my_time, false);

	my_time = scale_trans_hex(&my_time,sizeof(mytime_struct));
	time_zone = zone;

	ESP_ERROR_CHECK(RTC_Write(sizeof(mytime_struct),PCF86053_ADDR,CONFIG_ADDR,(unsigned char *)&my_time));
}

//����Unixʱ��;			
unsigned int Rtc_SyncGet_Time(void){
	unsigned int unix_time;
	mytime_struct my_time;

	ESP_ERROR_CHECK(RTC_Read(sizeof(mytime_struct),PCF86053_ADDR,CONFIG_ADDR,(unsigned char *)&my_time));
	
	my_time = scale_trans_bcd(&my_time,sizeof(mytime_struct));

	unix_time = mytime_2_utc_sec(&my_time, false) - time_zone * SEC_PER_HOUR;
	
	return unix_time;
}
