/**
 * @file vesync_sntp_service.c
 * @brief vesync平台sntp时间服务
 * @author WatWu
 * @date 2018-12-07
 */

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_event_loop.h"
#include "lwip/apps/sntp.h"

#include "vesync_log.h"

static const char *TAG = "vesync_sntp";

#define SNTP_SERVER_ADDR			"ntp.vesync.com"
// #define SNTP_SERVER_ADDR			"pool.ntp.org"
// #define SNTP_SERVER_ADDR			"time.windows.com"

/**
 * @brief 初始化sntp服务
 */
void vesync_init_sntp_service(void)
{
	LOG_I(TAG, "Initializing SNTP");
	sntp_setoperatingmode(SNTP_OPMODE_POLL);
	sntp_setservername(0, SNTP_SERVER_ADDR);
	sntp_init();
	// Set timezone to China Standard Time
	setenv("TZ", "CST-8", 1);
	tzset();
}

/**
 * @brief 打印系统时间
 */
void vesync_printf_system_time(void)
{
	time_t now;
	struct tm timeinfo;
	char strftime_buf[64];

	time(&now);
	localtime_r(&now, &timeinfo);

	// Is time set? If not, tm_year will be (1970 - 1900).
	if(timeinfo.tm_year < (2016 - 1900))
	{
		LOG_E(TAG, "Time is not set yet. Please connect to network first.");
		return;
	}

	strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
	LOG_I(TAG, "The current time is: %s", strftime_buf);
}
