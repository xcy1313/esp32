/**
 * @file touchkey.c
 * @brief 触摸按键
 * @author WatWu
 * @date 2019-01-11
 */

#include "driver/gpio.h"

#define TOUCH_KEY_GPIO                  17

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
}

/**
 * @brief 获取触摸状态
 * @return int [触摸状态，1为有触摸，0为无触摸]
 */
int get_touch_key_status(void)
{
    return gpio_get_level(TOUCH_KEY_GPIO);
}
