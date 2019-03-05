/**
 * @file vesync_user.c
 * @brief vesync平台应用层
 * @author WatWu
 * @date 2018-09-04
 */

#include "vesync_api.h"

#include "driver/gpio.h"
#include "driver/rtc_io.h"
#include "driver/adc.h"
#include "esp32/ulp.h"
#include "ulp_adc.h"

#include "sht30.h"
#include "vadisplay.h"
#include "buzzer.h"
#include "analog_param.h"
#include "touchkey.h"

#include "vesync_user.h"

static const char* TAG = "vesync_user";

static TimerHandle_t temp_humi_update_timer;

//任务句柄定义
TaskHandle_t app_event_center_taskhd = NULL;

/**
 * @brief 温湿度计参数更新定时器回调函数，读取最新温湿度计值
 * @param arg [无]
 */
static void temp_humi_update_timer_callback(void *arg)
{
    float temp;
    float humi;

    sht30_get_temp_and_humi(&temp, &humi);
    LOG_I(TAG, "Get sht30 data, temp : %f, humi : %f", temp, humi);

    LOG_I(TAG, "Battery voltage : %dmv", 4 * analog_adc_read_battery_mv());
    LOG_I(TAG, "Bettery charge status : %d", get_battery_charge_status());
    LOG_I(TAG, "Bettery charge fully status : %d", get_battery_charge_fully_status());
    // LOG_I(TAG, "TLV8811 out mv : %d", analog_adc_read_tlv8811_out_mv());

    va_display_temperature(temp, CELSIUS_UNIT);
    va_display_humidity(humi);
    bu9796a_display_ble_icon(true);
    bu9796a_display_wifi_icon(true);

    LOG_I(TAG, "last result=%d, sample counter=%d, wake up counter=%d,adc max=%d, min=%d.",
          ulp_last_result & UINT16_MAX, ulp_all_sample_counter & UINT16_MAX, ulp_wakeup_counter & UINT16_MAX,
          ulp_adc_max & UINT16_MAX, ulp_adc_min & UINT16_MAX);
}

/**
 * @brief 应用层任务入口
 * @param args [无]
 */
void application_task(void *args)
{
    //停止协处理器的硬件定时器，暂停ulp的运行
    CLEAR_PERI_REG_MASK(RTC_CNTL_STATE0_REG, RTC_CNTL_ULP_CP_SLP_TIMER_EN);

    sht30_init();
    buzzer_init();
    analog_adc_init();
    va_display_init();
    // vesync_client_connect_wifi("R6100-2.4G", "12345678");
    temp_humi_update_timer = xTimerCreate("temp_humi", 1000 / portTICK_RATE_MS, pdTRUE, NULL, temp_humi_update_timer_callback);
    xTimerStart(temp_humi_update_timer, TIMER_BLOCK_TIME);

    // adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_11db);

    // esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
    // if(cause != ESP_SLEEP_WAKEUP_ULP && cause != ESP_SLEEP_WAKEUP_TOUCHPAD)
    // {
    //     LOG_I(TAG, "Not ULP or touch wakeup.");
    //     init_ulp_program();
    // }
    // else
    //     LOG_I(TAG, "Deep sleep wakeup.");

    BaseType_t notified_ret;
    uint32_t notified_value;

    while(1)
    {
        notified_ret = xTaskNotifyWait(0x00000000, 0xFFFFFFFF, &notified_value, 10000 / portTICK_RATE_MS);
        if(pdPASS == notified_ret)
        {
            LOG_I(TAG, "Event center get new notified : %x.", notified_value);

            if(notified_value & NETWORK_CONNECTED)
            {
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
    LOG_I(TAG, "Application layer start !");
    xTaskCreate(application_task, "app_task", 4096, NULL, TASK_PRIORITY_NORMAL, &app_event_center_taskhd);
    vTaskDelete(NULL);
}
