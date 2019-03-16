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

#define RESTORE_FACTORY_NOTIFY_BIT	0x00000001

extern TaskHandle_t app_public_events_task;
extern bool enter_default_factory_function_mode;

/**
 * @brief 体脂称公共业务通知处理任务
 */
void app_public_events_task_create(void);

void app_public_send_notify_bit(uint32_t bit );

#endif