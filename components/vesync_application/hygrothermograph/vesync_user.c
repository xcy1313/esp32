/**
 * @file vesync_user.c
 * @brief vesync平台应用层
 * @author WatWu
 * @date 2018-09-04
 */

#include "driver/rtc_io.h"
#include "vesync_user.h"
#include "hygrothermograph.h"
#include "interface.h"
#include "vesync_https.h"

static const char* TAG = "vesync_user";

//任务句柄定义
TaskHandle_t app_event_center_taskhd = NULL;

/**
 * @brief 应用层任务入口
 * @param args [无]
 */
void application_task(void *args)
{
    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
    if(cause != ESP_SLEEP_WAKEUP_ULP && cause != ESP_SLEEP_WAKEUP_TOUCHPAD)
    {
        LOG_E(TAG, "Not ULP or touch wakeup : %d.", cause);
    }
    else
        LOG_E(TAG, "Deep sleep wakeup : %d.", cause);

    //初始化温湿度计设备
    hygrothermograph_init();

    uint8_t buffer[sizeof(temp_humi_history_t)] = {0};
    uint16_t data_len;
    vesync_flash_config(true, HUMITURE_FLASH_LABEL);
    int ret = vesync_flash_read(HUMITURE_FLASH_LABEL, HUMITURE_HISTORY_KEY, buffer, &data_len);
    if(ret == 0)
    {
        LOG_I(TAG, "Get flash data !");
    }
    else
    {
        LOG_E(TAG, "Read flash error, ret=%d", ret);
    }

    vesync_connect_wifi("R6100-2.4G", "12345678");
    // char recv_buff[512];
    // int recv_len = 512;
    // char body[] = "{\"traceId\":\"1540170843000\",\"method\":\"refreshDeviceToken\",\"cid\":\"0LWEYn9q8S7l2CJwFfLECk7f5B1D2_61\",\"pid\":\"2b9lja6xd0yd40jc\"}";
    // vesync_https_request("test-online.vesync.com", "443", "/cloud/v1/deviceWeb/refreshDeviceToken", body, recv_buff, &recv_len, 10 * 1000);
    // LOG_E(TAG, "Server reply : %s", recv_buff);

    BaseType_t notified_ret;
    uint32_t notified_value;

    while(1)
    {
        notified_ret = xTaskNotifyWait(0x00000000, 0xFFFFFFFF, &notified_value, 10000 / portTICK_RATE_MS);
        if(pdPASS == notified_ret)
        {
            LOG_I(TAG, "App event center get new notified : %x.", notified_value);

            if(notified_value & UPDATE_TEMP_HUMI_TO_APP)
            {
                update_temp_humi_to_app();
            }
        }
        else
        {
            // LOG_I(TAG, "No notified.");
        }
    }
}

/**
 * @brief vesync平台应用层入口函数
 */
void vesync_user_entry(void *args)
{
    //停止协处理器的硬件定时器，暂停ulp的运行
    CLEAR_PERI_REG_MASK(RTC_CNTL_STATE0_REG, RTC_CNTL_ULP_CP_SLP_TIMER_EN);

    //调试时的cid
    uint8_t test_cid[] = "0LWEYn9q8S7l2CJwFfLECk7f5B1D2_61";	//MAC地址：96:92
	// uint8_t test_cid[] = "0LWEYn9q8S7l2CJwFfLECk7f5B1D2_71";	//MAC地址：BB:8A
	strcpy((char *)product_config.cid,(char *)test_cid);

    vesync_developer_start();

    printf("\n");
    LOG_I(TAG, "------------Application layer start !------------\n");
    xTaskCreate(application_task, "app_task", 10 * 1024, NULL, TASK_PRIORITY_NORMAL, &app_event_center_taskhd);
    vTaskDelete(NULL);
}
