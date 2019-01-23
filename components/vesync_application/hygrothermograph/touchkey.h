/**
 * @file touchkey.h
 * @brief 触摸按键
 * @author WatWu
 * @date 2019-01-11
 */

#ifndef TOUCH_KEY_H
#define TOUCH_KEY_H

#define TOUCH_KEY_ON            1
#define TOUCH_KEY_OFF           0

#define POWER_KEY_UP            1
#define POWER_KEY_DOWN          0

#define POWER_ON                1
#define POWER_OFF               0

#define REACTION_AUTO           1
#define REACTION_MANUAL         0

/**
 * @brief 初始化触摸接口
 */
void touch_key_init(void);

/**
 * @brief 获取触摸状态
 * @return int [触摸状态，1为有触摸，0为无触摸]
 */
int get_touch_key_status(void);

/**
 * @brief 获取电源按键状态
 * @return int [按键状态，0为按下，1为弹起]
 */
int get_power_key_status(void);

/**
 * @brief 获取人体红外感应开关状态
 * @return int [人体红外感应开关状态，1为自动模式，0为手动模式]
 */
int get_reaction_key_status(void);

/**
 * @brief 检查电源按键状态，实时更新电源状态
 */
void check_power_key_status(void);

/**
 * @brief 获取设备电源开关状态
 * @return uint8_t [电源开关状态]
 */
uint8_t get_device_power_status(void);

#endif
