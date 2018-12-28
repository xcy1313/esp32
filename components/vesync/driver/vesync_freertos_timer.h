/**
 * @file vesync_freertos_timer.h
 * @brief 公共接口调用freertos软件定时器方法
 * @author Jiven 
 * @date 2018-12-21
 */
#ifndef _VESYNC_FREERTOS_TIMER_H
#define _VESYNC_FREERTOS_TIMER_H

#include <stdint.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"

/**
 * @brief 启动freertos软件定时器方法
 * @param timer 对应相应定时器
 * @return true 
 * @return false 
 */
bool method_timer_start(TimerHandle_t *timer);

/**
 * @brief 停止freertos软件定时器方法
 * @param timer 
 * @return true 
 * @return false 
 */
bool method_timer_stop(TimerHandle_t *timer);

/**
 * @brief 删除软件定时器
 * @param timer 
 * @return true 
 * @return false 
 */
bool method_timer_delete(TimerHandle_t *timer);

/**
 * @brief 修改计时周期
 * @param timer 
 * @param time_out 
 * @return true 
 * @return false 
 */
bool method_timer_change_period(TimerHandle_t *timer,uint32_t time_out);

/**
 * @brief 创建软件定时器 初始化创建time_out不能为0否则panic
 * @param timer 
 * @param time_out 
 * @param reload 是否重载
 * @param cb 定时到期回调
 * @return true 
 * @return false 
 */
bool method_timer_create(TimerHandle_t *timer,uint32_t time_out ,bool reload,TimerCallbackFunction_t cb);

#endif 

