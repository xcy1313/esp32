/**
 * @file app_public_events.h
 * @brief 体脂称公共事件业务逻辑处理
 * @author Jiven 
 * @date 2018-12-21
 */
#ifndef _APP_PUBLIC_EVENTS_H
#define _APP_PUBLIC_EVENTS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


extern TaskHandle_t app_public_events_task;

#define NET_CONFIG_NOTIFY_BIT	0x00000001
 
/**
 * @brief 体脂称公共业务通知处理任务
 */
void app_public_events_task_create(void);

#endif