/**
 * @file app_handle_server.h
 * @brief 处理和网络对接相关
 * @author Jiven 
 * @date 2018-12-29
 */
#ifndef _APP_HANDLE_SERVCER_H
#define _APP_HANDLE_SERVCER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/**
 * @brief 初始化网络功能配置
 */
void app_hadle_server_create(void);

#endif