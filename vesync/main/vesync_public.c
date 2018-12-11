/**
 * @file vesync_public.c
 * @brief  处理vesync各模块公共功能
 * @author Jiven 
 * @date 2018-12-07
 */
#include "vesync_public.h"
#include "vesync_sntp.h"
#include "nvs_flash.h"
#include "driver/rtc_io.h"
#include "driver/adc.h"
#include "esp_log.h"

static const char* TAG = "public";

//任务句柄定义
static TaskHandle_t vesync_scheduler_handle = NULL;

/**
 * @brief vesync状态事件组
 */
EventGroupHandle_t user_event_group;
const int WIFI_CONNECTED_BIT = BIT0;
const int OTA_BIT = BIT1;
const int FLASH_WRITE_BIT = BIT2;
const int DISCONNECTED_BIT = BIT3;
const int BT_CONNECTED_BIT = BIT4;
const int BT_DIS_CONNECTED_BIT = BIT5;

/**
 * @brief  进入深度休眠模式 io中断唤醒 rtc内部上拉 低电平唤醒
 * @param pin 
 */
void vesync_power_save_enter(uint32_t pin)
{
    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
    ESP_LOGI(TAG,"sleep source %d\r\n",cause);

    vesync_bt_deinit();
    vesync_wifi_deinit();
    vesync_uart_deint();
    nvs_flash_deinit();

    switch (esp_sleep_get_wakeup_cause()) {
        case ESP_SLEEP_WAKEUP_EXT1: {
            uint64_t wakeup_pin_mask = esp_sleep_get_ext1_wakeup_status();
            if (wakeup_pin_mask == 0) {
                int pin = __builtin_ffsll(wakeup_pin_mask) - 1;
                printf("Wake up from GPIO %d\n", pin);
            } else {
                printf("Wake up from other GPIO\n");
            }
            break;
        }
        default:
            break;
    }
    //rtc_gpio_isolate(GPIO_NUM_12);

    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
    gpio_pullup_en(pin);
    gpio_pulldown_dis(pin);

    adc_power_off();    //不添加增加1.4mah功耗
    const int ext_wakeup_pin_1 = pin;
    const uint64_t ext_wakeup_pin_1_mask = 1ULL << ext_wakeup_pin_1;

    ESP_LOGI(TAG,"Enabling EXT1 wakeup on pins GPIO%d\n", ext_wakeup_pin_1);
    esp_sleep_enable_ext1_wakeup(ext_wakeup_pin_1_mask, ESP_EXT1_WAKEUP_ALL_LOW);

    vTaskDelay(10000 / portTICK_PERIOD_MS);

    esp_deep_sleep_start();
}

static void vesync_scheduler_task(void *pvParameters){
    BaseType_t notified_ret;
	uint32_t notified_value;

    while(1){
        notified_ret = xTaskNotifyWait(0x00000000, 0xFFFFFFFF, &notified_value, portTICK_RATE_MS);
        if(pdPASS == notified_ret){
            ESP_LOGI(TAG, "Event center get new notified : %x.", notified_value);

			//if(notified_value & NETWORK_CONNECTED)
            {
				// vesync_mqtt_client_connect_to_cloud();
			}

        }else{
            vTaskDelay(500 / portTICK_PERIOD_MS);
            vesync_printf_system_time(); 
        }
    }
}

void vesync_public_init(void)
{
    esp_log_level_set(TAG, ESP_LOG_INFO);
    user_event_group = xEventGroupCreate();
    ESP_LOGI(TAG,"vesync_public_init\n");
    //xTaskCreate(&vesync_scheduler_task, "vesync_scheduler_task", 2048, NULL, 4, &vesync_scheduler_handle);
}

