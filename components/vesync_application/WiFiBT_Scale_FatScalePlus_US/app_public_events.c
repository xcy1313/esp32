/**
 * @file app_public_events.c
 * @brief 体脂称公共事件业务逻辑处理
 * @author Jiven 
 * @date 2018-12-21
 */
#include "app_public_events.h"
#include "esp_log.h"
#include "app_handle_scales.h"
#include "app_handle_phone.h"
#include "vesync_flash.h"
#include "vesync_log.h"

static const char *TAG = "app_public_events";

TaskHandle_t app_public_events_taskhd;

bool enter_default_factory_function_mode = false;

/**
 * @brief 应用层网络服务通知
 */
void app_public_send_notify_bit(uint32_t bit )
{
    xTaskNotify(app_public_events_taskhd, bit, eSetBits);			//通知事件处理中心任务
}

/**
 * @brief 通知设备恢复出厂设置;
 */
static void app_public_events_enter_factory(void)
{
    if(!enter_default_factory_function_mode){
        uint8_t action;
        enter_default_factory_function_mode = true;

        /*************清除存储信息开始*****************/
        vesync_nvs_erase_data(UNIT_NAMESPACE,UNIT_KEY);	//恢复默认单位;
        vesync_erase_net_info();	//清除配网信息
        vesync_flash_erase_partiton(USER_MODEL_NAMESPACE);	//删除用户模型分区
        vesync_flash_erase_partiton(USER_HISTORY_DATA_NAMESPACE);//删除用户历史数据分区
        /*************清除存储信息结束*****************/
        app_scale_suspend_start();  //唤醒称体
        vTaskDelay(1000 / portTICK_PERIOD_MS);	//正常使用10ms；
        action = 3; //通知称体进入"clr"模式
        app_uart_encode_send(MASTER_SET,CMD_CLEAR_USER,&action,sizeof(uint8_t),true);

        vTaskDelay(5000 / portTICK_PERIOD_MS);	//正常使用10ms；

        action = 4; //通知称体退出"clr"模式
        app_uart_encode_send(MASTER_SET,CMD_CLEAR_USER,&action,sizeof(uint8_t),true);
        vTaskDelay(1000 / portTICK_PERIOD_MS);	//正常使用10ms；
        esp_restart();
    }
}

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
			ESP_LOGI(TAG, "app_public_events_task_handler new notified : %x.", notified_value);

			if(notified_value & RESTORE_FACTORY_NOTIFY_BIT){
                app_public_events_enter_factory();
			}
        }
    }
    vTaskDelete(NULL);
}

/**
 * @brief app端创建通知任务
 */
void app_public_events_task_create(void)
{
    xTaskCreate(app_public_events_task_handler, "app_public_events_task_handler", 4096, NULL, 4, &app_public_events_taskhd);
}






