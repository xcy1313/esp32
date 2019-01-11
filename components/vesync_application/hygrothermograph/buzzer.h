/**
 * @file buzzer.h
 * @brief 蜂鸣器驱动
 * @author WatWu
 * @date 2019-01-11
 */

#ifndef BUZZER_H
#define BUZZER_H

/**
 * @brief 蜂鸣器启动鸣响
 * @param beep_times    [当次开启的鸣响次数]
 * @param beep_period   [当次开启的鸣响周期，即两次鸣响间的时间间隔]
 */
void buzzer_beeps(uint32_t beep_times, uint32_t beep_period);

/**
 * @brief 初始化蜂鸣器
 */
void buzzer_init(void);

/**
 * @brief 改变蜂鸣器的频率
 * @param freq_hz [频率值]
 */
void change_buzzer_freq(uint32_t freq_hz);

#endif
