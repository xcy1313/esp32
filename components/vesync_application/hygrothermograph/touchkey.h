/**
 * @file touchkey.h
 * @brief 触摸按键
 * @author WatWu
 * @date 2019-01-11
 */

#ifndef TOUCH_KEY_H
#define TOUCH_KEY_H

/**
 * @brief 初始化触摸接口
 */
void touch_key_init(void);

/**
 * @brief 获取触摸状态
 * @return int [触摸状态，1为有触摸，0为无触摸]
 */
int get_touch_key_status(void);

#endif
