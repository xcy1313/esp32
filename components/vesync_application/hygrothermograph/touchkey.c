/**
 * @file touchkey.c
 * @brief 触摸按键
 * @author WatWu
 * @date 2019-01-11
 */

#include "driver/gpio.h"

#define TOUCH_KEY_GPIO                  17              //触摸按键
#define POWER_KEY                       5               //电源按键
#define REACTION_KEY                    18              //人体感应开关按键

/**
 * @brief 初始化触摸接口
 */
void touch_key_init(void)
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = 1ULL << TOUCH_KEY_GPIO;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    io_conf.pin_bit_mask = 1ULL << POWER_KEY;
    gpio_config(&io_conf);

    io_conf.pin_bit_mask = 1ULL << REACTION_KEY;
    gpio_config(&io_conf);
}

/**
 * @brief 获取触摸状态
 * @return int [触摸状态，1为有触摸，0为无触摸]
 */
int get_touch_key_status(void)
{
    return gpio_get_level(TOUCH_KEY_GPIO);
}

/**
 * @brief 获取电源按键状态
 * @return int [按键状态，0为按下，1为弹起]
 */
int get_power_key_status(void)
{
    return gpio_get_level(POWER_KEY);
}

/**
 * @brief 获取人体红外感应开关状态
 * @return int [人体红外感应开关状态，1为自动模式，0为手动模式]
 */
int get_reaction_key_status(void)
{
    return gpio_get_level(REACTION_KEY);
}
