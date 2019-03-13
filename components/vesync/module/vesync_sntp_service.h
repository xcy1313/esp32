
#ifndef _VESYNC_SNTP_SERVICE_H
#define _VESYNC_SNTP_SERVICE_H
#include <stdint.h>

/**
 * @brief 初始化sntp服务
 */
void vesync_init_sntp_service(char *url);

/**
 * @brief 设置utc时间
 * @param utc_time 
 * @param time_zone 
 */
void vesync_set_time(uint32_t utc_time ,int8_t time_zone);

/**
 * @brief 打印系统时间
 */
void vesync_printf_system_time(void);

#endif 
