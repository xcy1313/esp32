
#include "vesync_sntp.h"
#include "vesync_unixtime.h"
#include "vesync_public.h"
#include "esp_log.h"

//sntp service
#include <time.h>
#include <sys/time.h>
#include "lwip/err.h"
#include "lwip/apps/sntp.h"

static const char* TAG = "vesync_sntp";

/**
 * @brief 初始化sntp服务
 */
void vesync_init_sntp_service(uint32_t unix_time,uint8_t area,char *url)
{
	ESP_LOGI(TAG, "Initializing SNTP");
	struct timeval tv = {
		.tv_sec = unix_time,
	};
	struct timezone tz = {
		0, 0
	};
	settimeofday(&tv, &tz);
	sntp_setoperatingmode(SNTP_OPMODE_POLL);
	sntp_setservername(0, url);
	sntp_init();
	// Set timezone to China Standard Time
	setenv("TZ", "CST-8", 1);
	tzset();
	struct timeval tv_start;
    gettimeofday(&tv_start, NULL);
    ESP_LOGI(TAG, "The set is %ld", tv_start.tv_sec);
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
	if(timeinfo.tm_year < (2016 - 1900)){
		ESP_LOGE(TAG, "Time is not set yet. Please connect to network first.");
		return;
	}
	struct timeval tv_start;
	gettimeofday(&tv_start, NULL);

	strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
	ESP_LOGI(TAG, "The current time is: [%ld]，%s",tv_start.tv_sec,strftime_buf);
}
