/**
 * @file buzzer.c
 * @brief 蜂鸣器驱动
 * @author WatWu
 * @date 2019-01-11
 */

#include "driver/gpio.h"
#include "driver/ledc.h"

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "vesync_main.h"

#define BUZZER_TIMER_NUM                LEDC_TIMER_0
#define BUZZER_TIMER_SPEED_MODE         LEDC_HIGH_SPEED_MODE
#define BUZZER_PWM_CHANNEL              LEDC_CHANNEL_0
#define BUZZER_FREQ                     3000
#define BUZZER_DUTY                     512
#define BUZZER_GPIO                     16
#define BUZZER_ON                       1
#define BUZZER_OFF                      0

static TimerHandle_t buzzer_timer;
static uint32_t buzzer_beeps_times = 1; //蜂鸣器鸣响次数

/**
 * @brief 开启蜂鸣器
 */
static void buzzer_on(void)
{
    ledc_channel_config_t ledc_channel =
    {
        .channel    = BUZZER_PWM_CHANNEL,
        .duty       = BUZZER_DUTY,
        .gpio_num   = BUZZER_GPIO,
        .speed_mode = BUZZER_TIMER_SPEED_MODE,
        .timer_sel  = BUZZER_TIMER_NUM
    };
    ledc_channel_config(&ledc_channel);
}

/**
 * @brief 关闭蜂鸣器
 */
static void buzzer_off(void)
{
    ledc_stop(BUZZER_TIMER_SPEED_MODE, BUZZER_PWM_CHANNEL, 0);
}

/**
 * @brief 蜂鸣器定时器回调函数，关闭蜂鸣器
 * @param arg [无]
 */
static void buzzer_timer_callback(void *arg)
{
    buzzer_beeps_times--;
    if(buzzer_beeps_times <= 0)             //达到了预期的鸣响次数，则关闭蜂鸣器
        buzzer_off();
    else if((buzzer_beeps_times % 2) == 1)  //奇数次，关闭蜂鸣器
    {
        buzzer_off();
        xTimerStart(buzzer_timer, TIMER_BLOCK_TIME);
    }
    else if((buzzer_beeps_times % 2) == 0)  //偶数次，重新开启蜂鸣器
    {
        buzzer_on();
        xTimerStart(buzzer_timer, TIMER_BLOCK_TIME);
    }
}

/**
 * @brief 蜂鸣器启动鸣响
 * @param beep_times    [当次开启的鸣响次数]
 * @param beep_period   [当次开启的鸣响周期，即两次鸣响间的时间间隔]
 */
void buzzer_beeps(uint32_t beep_times, uint32_t beep_period)
{
    buzzer_beeps_times = beep_times << 1;           //多次鸣响时，翻转总次数乘以2
    buzzer_on();
    xTimerStop(buzzer_timer, TIMER_BLOCK_TIME);
    xTimerChangePeriod(buzzer_timer, beep_period / portTICK_RATE_MS, TIMER_BLOCK_TIME);
    xTimerStart(buzzer_timer, TIMER_BLOCK_TIME);
}

/**
 * @brief 初始化蜂鸣器
 */
void buzzer_init(void)
{
    ledc_timer_config_t ledc_timer =
    {
        .duty_resolution = LEDC_TIMER_10_BIT,   // resolution of PWM duty
        .freq_hz = BUZZER_FREQ,                 // frequency of PWM signal
        .speed_mode = BUZZER_TIMER_SPEED_MODE,  // timer mode
        .timer_num = BUZZER_TIMER_NUM           // timer index
    };
    ledc_timer_config(&ledc_timer);

    buzzer_off();

    buzzer_timer = xTimerCreate("buzzer", 100 / portTICK_RATE_MS, pdFALSE, NULL, buzzer_timer_callback);
}

/**
 * @brief 改变蜂鸣器的频率
 * @param freq_hz [频率值]
 */
void change_buzzer_freq(uint32_t freq_hz)
{
    ledc_set_freq(BUZZER_TIMER_SPEED_MODE, BUZZER_TIMER_NUM, freq_hz);
}
