/**
 * @file vesync_main.h
 * @brief vesync平台SDK头文件
 * @author WatWu
 * @date 2018-08-14
 */

#ifndef VESYNC_MAIN_H
#define VESYNC_MAIN_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "vesync_task_def.h"
#include "vesync_log.h"

//vesync事件处理中心任务的事件标志位定义，高8位用于存放自定义参数，即事件位共24位
#define NETWORK_CONNECTED				0X00000001
#define NETWORK_DISCONNECTED			0X00000002
#define MQTT_CONNECTED					0X00000004
#define MQTT_DISCONNECTED				0X00000008
#define RECEIVE_UART_DATA				0X00000010
#define CONFIG_NET_MQTT_READY			0X00000020
#define REFRESH_HTTPS_TROKEN            0x00000040
#define HTTPS_NET_CONFIG_REGISTER       0x00000080
#define TIMER_BLOCK_TIME				100				//定时器操作的最大等待时间，单位系统时钟节拍

extern TaskHandle_t event_center_taskhd;

/**
 * @brief vesync平台入口
 * @param args [无]
 */
void vesync_entry(void *args);

/**
 * @brief vesync平台应用层入口函数
 */
void vesync_user_entry(void *args);

typedef void (*vesync_application_cb_t)(void);
void vesync_register_application_cb(vesync_application_cb_t cb);

#endif
