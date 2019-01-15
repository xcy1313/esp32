
#ifndef _VESYNC_SNTP_SERVICE_H
#define _VESYNC_SNTP_SERVICE_H
#include <stdint.h>

/**
 * @brief 初始化sntp服务
 */
void vesync_init_sntp_service(uint32_t unix_time,uint8_t area,char *url);

/**
 * @brief 打印系统时间
 */
void vesync_printf_system_time(void);

#endif 
