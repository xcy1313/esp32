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

//蓝牙通信命令定义
#define REPORT_TEMP_HUMI                        0x0301      //设备上报温湿度值
#define QUERY_HISTORY                           0x0302      //查询温湿度历史记录
#define CLEAR_HISTORY                           0x0303      //清空温湿度历史记录
#define QUERY_PREWARNING                        0x0304      //查询温湿度预警值
#define SET_PREWARNING                          0x0305      //设置温湿度预警值

extern TaskHandle_t app_event_center_taskhd;

#endif
