/**
 * @file low_power.c
 * @brief 温湿度计低功耗处理
 * @author WatWu
 * @date 2019-03-04
 */

#include "vesync_api.h"

#include "driver/gpio.h"
#include "driver/rtc_io.h"
#include "driver/adc.h"
#include "esp32/ulp.h"
#include "ulp_adc.h"

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
static void start_ulp_program()
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
 * @brief 进入低功耗模式
 */
void enter_low_power_mode(void)
{
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_AUTO);
    esp_sleep_enable_touchpad_wakeup();
    // gpio_pullup_en(BAT_CHARGING);
    // gpio_pulldown_dis(BAT_CHARGING);
    // esp_sleep_enable_ext1_wakeup(1ULL << BAT_CHARGING, ESP_EXT1_WAKEUP_ALL_LOW);
    // rtc_gpio_pullup_en(POWER_KEY);
    // esp_sleep_enable_ext0_wakeup(POWER_KEY, 0);
    start_ulp_program();
    esp_sleep_enable_ulp_wakeup();
    // adc_power_off();    //不添加增加1.4mah功耗
    esp_deep_sleep_start();
}
