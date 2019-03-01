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

static const char* TAG = "vesync_user";

extern const uint8_t ulp_adc_bin_start[] asm("_binary_ulp_adc_bin_start");
extern const uint8_t ulp_adc_bin_end[]   asm("_binary_ulp_adc_bin_end");

/**
 * @brief 初始化ulp程序，加载ulp固件到RTC内存中
 */
static void init_ulp_program()
{
    esp_err_t err = ulp_load_binary(0, ulp_adc_bin_start, (ulp_adc_bin_end - ulp_adc_bin_start) / sizeof(uint32_t));
    ESP_ERROR_CHECK(err);

    esp_deep_sleep_disable_rom_logging();   // 抑制boot上电信息

    ulp_wakeup_counter = 0;                 //ulp唤醒次数统计
    ulp_threshold = 6;                      //人体红外ADC变化阈值
}

/**
 * @brief 开始ulp协处理器程序，复位相关状态字和内存
 */
void start_ulp_program()
{
    init_ulp_program();

    adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_11);     //修改此处的ADC通道，需要同步修改汇编文件中的通道值
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_ulp_enable();

    ulp_adc_pos = 0;
    ulp_all_sample_counter = 0;
    memset((uint32_t*)&ulp_adc_buff, 0, 20);

    rtc_gpio_isolate(GPIO_NUM_12);          //断开GPIO口的内部连接，防止漏电流增加功耗
    rtc_gpio_isolate(GPIO_NUM_15);

    ulp_set_wakeup_period(0, 80 * 1000);    //唤醒周期，50ms
    esp_err_t err = ulp_run(&ulp_entry - RTC_SLOW_MEM);
    ESP_ERROR_CHECK(err);
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

    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
    if(cause != ESP_SLEEP_WAKEUP_ULP && cause != ESP_SLEEP_WAKEUP_TOUCHPAD)
    {
        LOG_I(TAG, "Not ULP or touch wakeup.");
        init_ulp_program();
    }

    else
        LOG_I(TAG, "Deep sleep wakeup.");

    while(1)
    {
        float temp;
        float humi;

        sht30_get_temp_and_humi(&temp, &humi);
        LOG_I(TAG, "Get sht30 data, temp : %f, humi : %f", temp, humi);

        LOG_I(TAG, "Battery voltage : %dmv", 4 * analog_adc_read_battery_mv());
        LOG_I(TAG, "Bettery charge status : %d", get_battery_charge_status());
        LOG_I(TAG, "Bettery charge fully status : %d", get_battery_charge_fully_status());

        va_display_temperature(temp, CELSIUS_UNIT);
        va_display_humidity(humi);
        bu9796a_display_ble_icon(true);
        bu9796a_display_wifi_icon(true);

        LOG_I(TAG, "last result=%d, sample counter=%d, wake up counter=%d,adc max=%d, min=%d.",
              ulp_last_result & UINT16_MAX, ulp_all_sample_counter & UINT16_MAX, ulp_wakeup_counter & UINT16_MAX,
              ulp_adc_max & UINT16_MAX, ulp_adc_min & UINT16_MAX);

        // printf_os_task_manager();
        usleep(1000 * 1000);
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
