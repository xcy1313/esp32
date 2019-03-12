/**
 * @file vesync_user.h
 * @brief vesync平台应用层主要头文件
 * @author WatWu
 * @date 2019-03-04
 */

#ifndef VESYNC_USER_H
#define VESYNC_USER_H

#include "vesync_api.h"

//应用层事件处理中心任务的事件标志位定义，高8位用于存放自定义参数，即事件位共24位
#define UPDATE_TEMP_HUMI_TO_APP 				0X00000001

extern TaskHandle_t app_event_center_taskhd;

#endif
