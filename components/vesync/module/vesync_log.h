/**
 * @file vesync_log.h
 * @brief vesync平台的log日志系统，统一封装log打印接口，实现应用层兼容不同硬件平台
 * @author WatWu
 * @date 2018-11-07
 */

#ifndef VESYNC_LOG_H
#define VESYNC_LOG_H

// 所有平台统一使用以下函数名作为log打印函数，格式举例：LOG_I( tag, format, ... )
// LOG_E - 错误/error
// LOG_W - 警告/warning
// LOG_I - 信息/info
// LOG_D - 调试/debug
// LOG_V - 啰嗦/verbose

//For ESP8266 FreeRTOS 平台
#include "esp_log.h"
#define LOG_E ESP_LOGE
#define LOG_W ESP_LOGW
#define LOG_I ESP_LOGI
#define LOG_D ESP_LOGD
#define LOG_V ESP_LOGV

#endif
