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
#include "vadisplay.h"
#include "buzzer.h"
#include "analog_param.h"

static const char* TAG = "vesync_user";

/**
 * @brief 应用层任务入口
 * @param args [无]
 */
void application_task(void *args)
{
    sht30_init();
    buzzer_init();
    analog_adc_init();
    va_display_init();

    while(1)
    {
        float temp;
        float humi;

        sht30_get_temp_and_humi(&temp, &humi);
        LOG_I(TAG, "Get sht30 data, temp : %f, humi : %f", temp, humi);

        va_display_temperature(temp, CELSIUS_UNIT);
        va_display_humidity(humi);

        // printf_os_task_manager();
        usleep(1000 * 1000);
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
