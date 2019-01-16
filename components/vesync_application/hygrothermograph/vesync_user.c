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
#include "analog_param.h"

static const char* TAG = "vesync_user";

/**
 * @brief 应用层任务入口
 * @param args [无]
 */
void application_task(void *args)
{
    sht30_init();
    bu9796a_init();
    bu9796a_initialize_sequence();
    bu9796a_display_off_sequence();
    buzzer_init();
    touch_key_init();
    analog_adc_init();

    while(1)
    {
        float temp;
        float humi;
        uint8_t key;
        uint32_t adc;

        sht30_get_temp_and_humi(&temp, &humi);
        LOG_I(TAG, "Get sht30 data, temp : %f, humi : %f", temp, humi);

        key = get_touch_key_status();
        LOG_I(TAG, "Get touch status : %d", key);

        adc = analog_adc_read_tlv8811_out_mv();
        LOG_I(TAG, "Get adc data : %d", adc);

        bu9796a_display_number_to_ram(0, (uint8_t)temp / 10);
        bu9796a_display_number_to_ram(1, (uint8_t)temp % 10);
        bu9796a_display_number_to_ram(2, (uint8_t)((uint32_t)(temp * 10) % 10));
        if(key == 1)
        {
            adc = adc % 100;
            bu9796a_display_number_to_ram(3, (uint8_t)adc / 10);
            bu9796a_display_number_to_ram(4, (uint8_t)adc % 10);
        }
        else
        {
            bu9796a_display_number_to_ram(3, (uint8_t)humi / 10);
            bu9796a_display_number_to_ram(4, (uint8_t)humi % 10);
        }

        bu9796a_update_display();

        // printf_os_task_manager();
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
