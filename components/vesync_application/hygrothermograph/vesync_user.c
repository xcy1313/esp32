/**
 * @file vesync_user.c
 * @brief vesync平台应用层
 * @author WatWu
 * @date 2018-09-04
 */

#include <string.h>
#include <stdlib.h>

#include "vesync_log.h"
#include "vesync_api.h"

#include "vesync_wifi.h"
#include "vesync_net_service.h"
#include "vesync_production.h"
#include "vesync_flash.h"
#include "vesync_interface.h"
#include "vesync_developer.h"

#include "sht30.h"
#include "bu9796a.h"
#include "buzzer.h"
#include "touchkey.h"

static const char* TAG = "vesync_user";

/**
 * @brief 应用层任务入口
 * @param args [无]
 */
void application_task(void *args)
{
    float temp;
    float humi;
    sht30_init();
    bu9796a_init();
    bu9796a_initialize_sequence();
    bu9796a_display_all_pixels_on();
    buzzer_init();
    touch_key_init();

    while(1)
    {
        sht30_get_temp_and_humi(&temp, &humi);
        LOG_I(TAG, "Get data done, temp : %f, humi : %f", temp, humi);
        // printf_os_task_manager();
        LOG_I(TAG, "Get touch status : %d", get_touch_key_status());
        sleep(1);
    }
}

/**
 * @brief vesync平台应用层入口函数
 */
void vesync_user_entry(void *args)
{
    LOG_I(TAG, "Application layer start !");
    xTaskCreate(application_task, "app_task", 4096, NULL, 5, NULL);
    vTaskDelete(NULL);
}
