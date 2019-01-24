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
#include "touchkey.h"

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
    // vesync_client_connect_wifi("R6100-2.4G", "12345678");

    while(1)
    {
        float temp;
        float humi;

        sht30_get_temp_and_humi(&temp, &humi);
        LOG_I(TAG, "Get sht30 data, temp : %f, humi : %f", temp, humi);

        LOG_I(TAG, "Battery voltage : %dmv", 4 * analog_adc_read_battery_mv());
        LOG_I(TAG, "Bettery charge status : %d", get_battery_charge_status());
        LOG_I(TAG, "Bettery charge fully status : %d", get_battery_charge_fully_status());

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
