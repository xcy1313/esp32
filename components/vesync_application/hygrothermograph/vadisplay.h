/**
 * @file vadisplay.h
 * @brief va屏显示
 * @author WatWu
 * @date 2019-01-21
 */

#ifndef VADISPLAY_H
#define VADISPLAY_H

#include "bu9796a.h"

/**
 * @brief 初始化VA屏显示
 */
void va_display_init(void);

/**
 * @brief VA屏显示温度
 * @param temp  [温度值]
 * @param units [待显示的温度单位，0为摄氏度，1为华氏度]
 */
void va_display_temperature(float temp, uint8_t units);

/**
 * @brief VA屏显示湿度
 * @param humi [湿度值]
 */
void va_display_humidity(float humi);

/**
 * @brief 显示屏打开背光，同时背光会延时关闭
 * @param off_sec [延时关闭背光，单位秒]
 */
void va_display_trun_on_backlight(uint32_t off_sec);

/**
 * @brief 显示屏开始指示充电
 */
void va_display_start_charging(void);

/**
 * @brief 显示屏停止指示充电
 */
void va_display_stop_charging(void);

/**
 * @brief VA屏显示剩余电量
 * @param bat_mv [电池电压]
 */
void va_display_bat_dump_energy(uint32_t bat_mv);

/**
 * @brief 进入深度睡眠模式
 */
void enter_deep_sleep_mode(void);

#endif
