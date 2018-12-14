/**
 * @file vesync_hw_timer_hal.c
 * @brief vesync设备硬件定时器硬件抽象层
 * @author WatWu
 * @date 2018-11-19
 */

#include "freertos/FreeRTOS.h"

// #include "driver/hw_timer.h"
// #include "vesync_hw_timer_hal.h"

// //供cpu使用率统计的高频硬件定时器计数值，频率为系统心跳的10～20倍
// volatile unsigned long ulHighFrequencyTimerTicks = 0;

// /**
//  * @brief 高频硬件定时器回调函数
//  * @param arg [无]
//  */
// static void hw_timer_callback(void *arg)
// {
// 	ulHighFrequencyTimerTicks++;
// }

// /**
//  * @brief 初始化硬件定时器供vTaskGetRunTimeStats函数使用，定时器中断频率为系统心跳的10～20倍
//  */
// void vConfigureTimerForRunTimeStats(void)
// {
// 	hw_timer_init(hw_timer_callback, NULL);
// 	hw_timer_alarm_us(100, 1);
// }
