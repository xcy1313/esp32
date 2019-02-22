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

/* This function is called once after power-on reset, to load ULP program into
 * RTC memory and configure the ADC.
 */
static void init_ulp_program()
{
    esp_err_t err = ulp_load_binary(0, ulp_adc_bin_start, (ulp_adc_bin_end - ulp_adc_bin_start) / sizeof(uint32_t));
    ESP_ERROR_CHECK(err);

    /* Configure ADC channel */
    /* Note: when changing channel here, also change 'adc_channel' constant
       in adc.S */
    adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_11);
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_ulp_enable();

    /* Set ULP wake up period to 50ms */
    ulp_set_wakeup_period(0, 100 * 1000);

    /* Disconnect GPIO12 and GPIO15 to remove current drain through
     * pullup/pulldown resistors.
     * GPIO12 may be pulled high to select flash voltage.
     */
    rtc_gpio_isolate(GPIO_NUM_12);
    rtc_gpio_isolate(GPIO_NUM_15);
    esp_deep_sleep_disable_rom_logging(); // suppress boot messages

    /* Reset sample counter */
    ulp_all_sample_counter = 0;
    ulp_wakeup_counter = 0;
    ulp_adc_pos = 0;
    ulp_threshold = 10;
}

/* This function is called every time before going into deep sleep.
 * It starts the ULP program and resets measurement counter.
 */
static void start_ulp_program()
{
    /* Start the program */
    esp_err_t err = ulp_run(&ulp_entry - RTC_SLOW_MEM);
    ESP_ERROR_CHECK(err);
}

/**
 * @brief 应用层任务入口
 * @param args [无]
 */
void application_task(void *args)
{
    // sht30_init();
    // buzzer_init();
    // analog_adc_init();
    // va_display_init();
    // vesync_client_connect_wifi("R6100-2.4G", "12345678");

    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
    if(cause != ESP_SLEEP_WAKEUP_ULP)
    {
        LOG_I(TAG, "Not ULP wakeup.");
        init_ulp_program();
        start_ulp_program();
    }
    else
    {
        LOG_I(TAG, "Deep sleep wakeup.");
    }

    while(1)
    {
        // float temp;
        // float humi;

        // sht30_get_temp_and_humi(&temp, &humi);
        // LOG_I(TAG, "Get sht30 data, temp : %f, humi : %f", temp, humi);

        // // LOG_I(TAG, "Battery voltage : %dmv", 4 * analog_adc_read_battery_mv());
        // // LOG_I(TAG, "Bettery charge status : %d", get_battery_charge_status());
        // LOG_I(TAG, "Bettery charge fully status : %d", get_battery_charge_fully_status());

        // va_display_temperature(temp, CELSIUS_UNIT);
        // va_display_humidity(humi);
        // bu9796a_display_ble_icon(true);
        // bu9796a_display_wifi_icon(true);

        // printf_os_task_manager();
        usleep(100 * 1000);

        // LOG_I(TAG, "tlv8811 mv : %d", analog_adc_read_tlv8811_out_mv());
        if((ulp_wakeup_counter & UINT16_MAX) > 0)
        {
            LOG_I(TAG, "last result=%d, sample counter=%d, wake up counter=%d.",
                  ulp_last_result & UINT16_MAX, ulp_all_sample_counter & UINT16_MAX, ulp_wakeup_counter & UINT16_MAX);
        }
        LOG_I(TAG, "adc buffer : [0]=%d, [1]=%d, [2]=%d, [3]=%d, [4]=%d, [5]=%d, [6]=%d, [7]=%d, [8]=%d, [9]=%d, temp=%d.",
              (&ulp_adc_buff)[0] & UINT16_MAX, (&ulp_adc_buff)[1] & UINT16_MAX, (&ulp_adc_buff)[2] & UINT16_MAX,
              (&ulp_adc_buff)[3] & UINT16_MAX, (&ulp_adc_buff)[4] & UINT16_MAX, (&ulp_adc_buff)[5] & UINT16_MAX,
              (&ulp_adc_buff)[6] & UINT16_MAX, (&ulp_adc_buff)[7] & UINT16_MAX, (&ulp_adc_buff)[8] & UINT16_MAX,
              (&ulp_adc_buff)[9] & UINT16_MAX, ulp_temp & UINT16_MAX);
        // start_ulp_program();
        // esp_sleep_enable_ulp_wakeup();
        // esp_deep_sleep_start();
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
