/**
 * @file vadisplay.c
 * @brief va屏显示
 * @author WatWu
 * @date 2019-01-21
 */

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "unistd.h"

#include "vesync_task_def.h"
#include "vesync_log.h"
#include "vesync_main.h"

#include "bu9796a.h"
#include "touchkey.h"
#include "analog_param.h"
#include "vadisplay.h"

static TimerHandle_t backlight_timer;

/**
 * @brief VA屏显示刷新任务
 */
static void va_display_update(void *args)
{
    uint8_t key;

    while(1)
    {
        bu9796a_update_display();
        key = get_touch_key_status();
        uint32_t voltage;
        voltage = analog_adc_read_tlv8811_out_mv();
        if(key == TOUCH_KEY_ON || judge_voltage_change(voltage, 8))
            va_display_trun_on_backlight(10);
        // LOG_I("va", "power key : %d", get_power_key_status());
        // LOG_I("va", "reaction key : %d", get_reaction_key_status());
        usleep(50 * 1000);
    }
}

/**
 * @brief 背光定时器回调函数，关闭背光
 * @param arg [无]
 */
static void backlight_timer_callback(void *arg)
{
    bu9796a_backlight_off();
}

/**
 * @brief 显示屏打开背光，同时背光会延时关闭
 * @param off_sec [延时关闭背光，单位秒]
 */
void va_display_trun_on_backlight(uint32_t off_sec)
{
    bu9796a_backlight_on();
    xTimerStop(backlight_timer, TIMER_BLOCK_TIME);
    xTimerChangePeriod(backlight_timer, off_sec * 1000 / portTICK_RATE_MS, TIMER_BLOCK_TIME);
    xTimerStart(backlight_timer, TIMER_BLOCK_TIME);
}

/**
 * @brief 初始化VA屏显示
 */
void va_display_init(void)
{
    bu9796a_init();
    bu9796a_initialize_sequence();
    bu9796a_display_off_sequence();
    touch_key_init();

    backlight_timer = xTimerCreate("buzzer", 100 / portTICK_RATE_MS, pdFALSE, NULL, backlight_timer_callback);
    va_display_trun_on_backlight(10);

    if(pdPASS != xTaskCreate(va_display_update,
                             "va_display",
                             2048 / sizeof(portSTACK_TYPE),
                             NULL,
                             TASK_PRIORITY_NORMAL,
                             NULL))
    {
        LOG_E("va", "Create va display task fail !");
    }
}

/**
 * @brief VA屏显示温度
 * @param temp  [温度值]
 * @param units [待显示的温度单位，0为摄氏度，1为华氏度]
 */
void va_display_temperature(float temp, uint8_t units)
{
    uint8_t decade;
    if(temp >= 0)
    {
        decade = (uint8_t)temp % 100 / 10;
        if(0 == decade && temp < 100)
            bu9796a_display_number_clear(0);
        else
            bu9796a_display_number_to_ram(0, decade);
        bu9796a_display_num_one((uint8_t)temp / 100);
    }
    else
    {
        temp = -temp;
        bu9796a_display_minus();
        bu9796a_display_num_one(false);
    }
    bu9796a_display_number_to_ram(1, (uint8_t)temp % 10);
    bu9796a_display_number_to_ram(2, (uint8_t)((uint32_t)(temp * 10) % 10));
    bu9796a_display_num_point(true);
    bu9796a_display_temp_units(units);
}

/**
 * @brief VA屏显示湿度
 * @param humi [湿度值]
 */
void va_display_humidity(float humi)
{
    bu9796a_display_number_to_ram(3, (uint8_t)humi / 10);
    bu9796a_display_number_to_ram(4, (uint8_t)humi % 10);
    bu9796a_display_humi_icon(true);
}
