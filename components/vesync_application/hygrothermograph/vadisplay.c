/**
 * @file vadisplay.c
 * @brief va屏显示
 * @author WatWu
 * @date 2019-01-21
 */

#include "driver/rtc_io.h"
#include "driver/adc.h"

#include "vesync_api.h"

#include "bu9796a.h"
#include "touchkey.h"
#include "analog_param.h"
#include "vadisplay.h"
#include "buzzer.h"
#include "low_power.h"

static const char *TAG = "va";

static TimerHandle_t backlight_timer;
static TimerHandle_t charging_timer;
static uint8_t just_power_on = true;
static uint8_t just_charging = false;

/**
 * @brief VA屏显示刷新任务
 */
static void va_display_update(void *args)
{
    while(1)
    {
        if(1 == get_battery_charge_status())            //不在充电中
        {
            if(true == just_charging)                   //刚刚在充电，现停止充电
            {
                just_charging = false;
                va_display_stop_charging();
            }

            check_power_key_status();
            if(POWER_ON == get_device_power_status())
            {
                if(true == just_power_on)               //刚开机
                {
                    just_power_on = false;
                    va_display_trun_on_backlight(10);
                    // buzzer_beeps(2, 100);
                }
                va_display_bat_dump_energy(4 * analog_adc_read_battery_mv());
                bu9796a_update_display();
                if(TOUCH_KEY_ON == get_touch_key_status())
                    va_display_trun_on_backlight(10);
            }
            else
            {
                if(false == just_power_on)              //刚关机
                {
                    just_power_on = true;
                    buzzer_beeps(3, 200);
                    bu9796a_backlight_off();
                    sleep(2);
                    enter_low_power_mode();
                }
                bu9796a_backlight_off();
            }
        }
        else if(0 == get_battery_charge_status())
        {
            if(false == just_charging)  //刚充电
            {
                just_charging = true;
                va_display_start_charging();
            }
            bu9796a_backlight_on();
        }
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
    enter_low_power_mode();
}

/**
 * @brief va屏显示充电中
 */
static void va_display_bat_charging(void)
{
    bu9796a_clear_display();
    if(0 == get_battery_charge_fully_status())      //已经充满电
    {
        bu9796a_display_bat_power_icon(3);
    }
    else
    {
        static int dump_power = 0;
        bu9796a_display_bat_power_icon(dump_power++);
        if(dump_power > 3)
            dump_power = 0;
    }
    bu9796a_update_display();
}

/**
 * @brief 充电中定时器回调函数，循环递增电量指示
 * @param arg [无]
 */
static void charging_timer_callback(void *arg)
{
    va_display_bat_charging();
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
 * @brief 显示屏开始指示充电
 */
void va_display_start_charging(void)
{
    va_display_bat_charging();
    xTimerStart(charging_timer, TIMER_BLOCK_TIME);
}

/**
 * @brief 显示屏停止指示充电
 */
void va_display_stop_charging(void)
{
    xTimerStop(charging_timer, TIMER_BLOCK_TIME);
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
    charging_timer = xTimerCreate("charge", 1000 / portTICK_RATE_MS, pdTRUE, NULL, charging_timer_callback);
    va_display_trun_on_backlight(10);

    if(pdPASS != xTaskCreate(va_display_update,
                             "va_display",
                             2048 / sizeof(portSTACK_TYPE),
                             NULL,
                             TASK_PRIORITY_NORMAL,
                             NULL))
    {
        LOG_E(TAG, "Create va display task fail !");
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

/**
 * @brief VA屏显示剩余电量
 * @param bat_mv [电池电压]
 */
void va_display_bat_dump_energy(uint32_t bat_mv)
{
    if(bat_mv > 4010 && bat_mv < 4250)
        bu9796a_display_bat_power_icon(3);
    else if(bat_mv > 3810 && bat_mv < 4000)
        bu9796a_display_bat_power_icon(2);
    else if(bat_mv > 3610 && bat_mv < 3800)
        bu9796a_display_bat_power_icon(1);
    else if(bat_mv > 3410 && bat_mv < 3600)
        bu9796a_display_bat_power_icon(0);
}
