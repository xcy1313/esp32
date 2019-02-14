/**
 * @file vesync_api.h
 * @brief vesync平台标准固件API头文件，用于暴露标准固件内部的头文件、宏定义、部分变量定义、API函数等
 * @author WatWu
 * @date 2018-11-17
 */

#ifndef VESYNC_API_H
#define VESYNC_API_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"

#include "cJSON.h"

// driver
#include "vesync_bt_hal.h"
#include "vesync_button.h"
#include "vesync_flash.h"
#include "vesync_freertos_timer.h"
#include "vesync_uart.h"
#include "vesync_unixtime.h"
#include "vesync_wifi.h"

// module
#include "vesync_developer.h"
#include "vesync_log.h"

// service
#include "vesync_task_def.h"
#include "vesync_config_net.h"
#include "vesync_device.h"
#include "vesync_wifi_led.h"
#include "vesync_net_service.h"
#include "vesync_production.h"
#include "vesync_interface.h"
#include "vesync_main.h"

#endif
