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

#include "sht30.h"

static const char* TAG = "vesync_user";

/**
 * @brief 
 * @param args 
 */
void application_task(void *args)
{
    sht30_init();
    sleep(1);
    float temp;
    float humi;

    while(1)
    {
        sht30_get_temp_and_humi(&temp, &humi);
        LOG_I(TAG, "Get data done, temp : %f, humi : %f", temp, humi);
        usleep(500*1000);
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
