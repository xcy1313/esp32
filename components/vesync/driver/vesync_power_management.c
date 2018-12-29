/**
 * @file vesync_power_management.c
 * @brief 配置低功耗工作模式
 * @author Jiven 
 * @date 2018-12-29
 */
#include "vesync_power_management.h"
#include "driver/rtc_io.h"
#include "driver/adc.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

static const char* TAG = "vesync_power_management";

/**
 * @brief  进入深度休眠模式 io中断唤醒 rtc内部上拉 低电平唤醒
 * @param pin 
 */
void vesync_power_save_enter(uint32_t pin)
{
    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
    ESP_LOGI(TAG,"sleep source %d\r\n",cause);

    switch (esp_sleep_get_wakeup_cause()){
        case ESP_SLEEP_WAKEUP_EXT1: {
            uint64_t wakeup_pin_mask = esp_sleep_get_ext1_wakeup_status();
            if (wakeup_pin_mask == 0) {
                int pin = __builtin_ffsll(wakeup_pin_mask) - 1;
                printf("Wake up from GPIO %d\n", pin);
            }else{
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


