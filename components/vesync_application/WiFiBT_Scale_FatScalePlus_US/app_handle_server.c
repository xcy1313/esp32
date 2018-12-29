/**
 * @file app_public_events.c
 * @brief 处理和网络对接相关
 * @author Jiven 
 * @date 2018-12-21
 */
#include "app_handle_server.h"
#include "vesync_flash.h"

#include "app_handle_scales.h"

#include "esp_log.h"

static const char *TAG = "app_handle_server";

/**
 * @brief 
 * @param pvParameters 
 */
static void app_handle_server_task_handler(void *pvParameters)
{
    while(1){

    }
    vTaskDelete(NULL);
}
/**
 * @brief app端创建通知任务
 */
void app_hadle_server_create(void)
{
    vesync_flash_config(true ,USER_HISTORY_DATA_NAMESPACE);//初始化用户沉淀数据flash区域
    //xTaskCreate(app_handle_server_task_handler, "app_handle_server_task_handler", 8192, NULL, 13, NULL);
}









