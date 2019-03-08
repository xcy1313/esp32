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
#include "low_power.h"

#include "vesync_user.h"

#include "vesync_build_cfg.h"

static const char* TAG = "vesync_user";

static TimerHandle_t temp_humi_update_timer;            //温湿度值刷新定时器
static TimerHandle_t enter_low_power_timer;             //开机后进入低功耗模式定时器

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
    // printf_os_task_manager();

    LOG_I(TAG, "Battery voltage : %dmv", 4 * analog_adc_read_battery_mv());
    LOG_I(TAG, "Bettery charge status : %d", get_battery_charge_status());
    LOG_I(TAG, "Bettery charge fully status : %d", get_battery_charge_fully_status());
    LOG_I(TAG, "Touch value : %d", get_touch_key_value());

    va_display_temperature(temp, CELSIUS_UNIT);
    va_display_humidity(humi);
    bu9796a_display_ble_icon(true);
    bu9796a_display_wifi_icon(true);

    LOG_I(TAG, "last result=%d, sample counter=%d, wake up counter=%d,adc max=%d, min=%d.",
          ulp_last_result & UINT16_MAX, ulp_all_sample_counter & UINT16_MAX, ulp_wakeup_counter & UINT16_MAX,
          ulp_adc_max & UINT16_MAX, ulp_adc_min & UINT16_MAX);
}

/**
 * @brief 进入低功耗模式定时器回调函数
 * @param arg [无]
 */
static void enter_low_power_timer_callback(void *arg)
{
    LOG_E(TAG, "Enter low power mode !");
    enter_low_power_mode();
}

/**
 * @brief 蓝牙数据接收回调函数
 * @param data  [接收的蓝牙数据]
 * @param len   [数据长度]
 */
static void ble_recv_data_callback(const unsigned char *data, unsigned char len)
{
    LOG_I(TAG, "ble recv : %s, len : %d", data, len);
}

/**
 * @brief 蓝牙状态回调
 * @param bt_status [当前实时蓝牙状态]
 */
static void ble_status_callback(BT_STATUS_T bt_status)
{
    LOG_I(TAG, "set ble status :%d", bt_status);
    switch(bt_status)
    {
        case BT_CREATE_SERVICE:
            break;
        case BT_ADVERTISE_START:
            break;
        case BT_ADVERTISE_STOP:
            break;
        case BT_CONNTED:
            vesync_bt_advertise_stop();
            break;
        case BT_DISCONNTED:
            vesync_bt_advertise_start(120 * 1000);
            break;
        default:
            break;
    }
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
    enter_low_power_timer = xTimerCreate("low_power", 60 * 1000 / portTICK_RATE_MS, pdFALSE, NULL, enter_low_power_timer_callback);
    xTimerStart(temp_humi_update_timer, TIMER_BLOCK_TIME);
    xTimerStart(enter_low_power_timer, TIMER_BLOCK_TIME);

    vesync_bt_client_init(PRODUCT_NAME, PRODUCT_VER, "0001", PRODUCT_TYPE, PRODUCT_NUM, NULL, true, ble_status_callback, ble_recv_data_callback);
    vesync_bt_advertise_start(0);

    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
    if(cause != ESP_SLEEP_WAKEUP_ULP && cause != ESP_SLEEP_WAKEUP_TOUCHPAD)
    {
        LOG_E(TAG, "Not ULP or touch wakeup : %d.", cause);
        // init_ulp_program();
    }
    else
        LOG_E(TAG, "Deep sleep wakeup : %d.", cause);

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
