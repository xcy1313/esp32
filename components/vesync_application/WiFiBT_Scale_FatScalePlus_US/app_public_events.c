/**
 * @file app_public_events.c
 * @brief 体脂称公共事件业务逻辑处理
 * @author Jiven 
 * @date 2018-12-21
 */
#include "app_public_events.h"
#include "esp_log.h"

static const char *TAG = "app_public_events";

TaskHandle_t app_public_events_task;


/**
 * @brief 
 * @param pvParameters 
 */
static void app_public_events_task_handler(void *pvParameters)
{
    BaseType_t notified_ret;
    uint32_t notified_value;
    while(1){
        notified_ret = xTaskNotifyWait(0x00000000, 0xFFFFFFFF, &notified_value, 10000 / portTICK_RATE_MS);
        if(pdPASS == notified_ret){
			ESP_LOGI(TAG, "Event center get new notified : %x.", notified_value);

			if(notified_value & NET_CONFIG_NOTIFY_BIT){
                
			}
        }
    }
}

/**
 * @brief app端创建通知任务
 */
void app_public_events_task_create(void)
{
    xTaskCreate(app_public_events_task_handler, "app_public_events_task_handler", 8192, NULL, 13, NULL);
}






