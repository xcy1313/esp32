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

#define BLE_CONNECTED           1
#define BLE_DISCONNECTED        0

static const char* TAG = "vesync_user";

static TimerHandle_t temp_humi_update_timer;            //温湿度值刷新定时器
static TimerHandle_t enter_low_power_timer;             //开机后进入低功耗模式定时器
static float temperature;
static float humidity;
static uint32_t ble_connect_status = BLE_DISCONNECTED;

//任务句柄定义
TaskHandle_t app_event_center_taskhd = NULL;

/**
 * @brief 温湿度计参数更新定时器回调函数，读取最新温湿度计值
 * @param arg [无]
 */
static void temp_humi_update_timer_callback(void *arg)
{
    sht30_get_temp_and_humi(&temperature, &humidity);
    LOG_I(TAG, "Get sht30 data, temperature : %f, humidity : %f", temperature, humidity);
    // printf_os_task_manager();

    LOG_I(TAG, "Battery voltage : %dmv", 4 * analog_adc_read_battery_mv());
    LOG_I(TAG, "Bettery charge status : %d", get_battery_charge_status());
    LOG_I(TAG, "Bettery charge fully status : %d", get_battery_charge_fully_status());
    LOG_I(TAG, "Touch value : %d", get_touch_key_value());
    LOG_I(TAG, "TLV8811 out : %d", analog_adc_read_tlv8811_out_mv());

    va_display_temperature(temperature, CELSIUS_UNIT);
    va_display_humidity(humidity);
    bu9796a_display_ble_icon(true);
    bu9796a_display_wifi_icon(true);

    LOG_I(TAG, "last result=%d, sample counter=%d, wake up counter=%d,adc max=%d, min=%d.",
          ulp_last_result & UINT16_MAX, ulp_all_sample_counter & UINT16_MAX, ulp_wakeup_counter & UINT16_MAX,
          ulp_adc_max & UINT16_MAX, ulp_adc_min & UINT16_MAX);

    static uint32_t second_count = 0;           //秒统计值
    second_count++;
    if(second_count == 20)
    {
        second_count = 0;
        xTaskNotify(app_event_center_taskhd, UPDATE_TEMP_HUMI_TO_APP, eSetBits);			//通知事件处理中心任务
    }
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
 * @brief 更新温湿度值到APP，通过ble蓝牙
 */
static void update_temp_humi_to_app(void)
{
    static uint8_t cnt = 0;
    frame_ctrl_t send_ctl =     //发送数据格式码
    {
        .data = 0x0
    };
    struct                      //数据包
    {
        int16_t temp;
        int16_t humi;
        uint32_t timestamp;
    } send_data = {0};

    uint16_t send_cmd = REPORT_TEMP_HUMI;
    uint16_t send_len = 8;

    send_data.temp = (int16_t)(temperature * 10);
    send_data.humi = (int16_t)(humidity * 10);
    send_data.timestamp = 1544410793;

    cnt++;
    vesync_bt_notify(send_ctl, &cnt, send_cmd, (unsigned char *)&send_data, send_len);
    LOG_I(TAG, "Update temp humi to app !");
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
            xTimerStop(enter_low_power_timer, TIMER_BLOCK_TIME);
            xTaskNotify(app_event_center_taskhd, UPDATE_TEMP_HUMI_TO_APP, eSetBits);			//通知事件处理中心任务
            ble_connect_status = BLE_CONNECTED;
            break;
        case BT_DISCONNTED:
            // vesync_bt_advertise_start(120 * 1000);
            xTimerStart(enter_low_power_timer, TIMER_BLOCK_TIME);
            ble_connect_status = BLE_DISCONNECTED;
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
    enter_low_power_timer = xTimerCreate("low_power", 10 * 1000 / portTICK_RATE_MS, pdFALSE, NULL, enter_low_power_timer_callback);
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
    LOG_I(TAG, "Application layer start !");
    xTaskCreate(application_task, "app_task", 4096, NULL, TASK_PRIORITY_NORMAL, &app_event_center_taskhd);
    vTaskDelete(NULL);
}
