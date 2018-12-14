/**
 * @file vesync_sntp_service.h
 * @brief vesync平台sntp时间服务
 * @author WatWu
 * @date 2018-12-07
 */

#ifndef VESYNC_SNTP_SERVICE_H
#define VESYNC_SNTP_SERVICE_H

/**
 * @brief 初始化sntp服务
 */
void vesync_init_sntp_service(void);

/**
 * @brief 打印系统时间
 */
void vesync_printf_system_time(void);

#endif
