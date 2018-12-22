/**
 * @file vesync_freertos_timer.c
 * @brief 公共接口调用freertos软件定时器方法
 * @author Jiven (you@domain.com)
 * @date 2018-12-21
 */
#include "vesync_freertos_timer.h"
#include "esp_log.h"

/**
 * @brief 启动freertos软件定时器方法
 * @param timer 对应相应定时器
 * @return true 
 * @return false 
 */
bool method_timer_start(TimerHandle_t *timer)
{
    bool status = false;

    if (xTimerStart(*timer, portMAX_DELAY) != pdPASS) {
        status = false;
    } else {
        status =  true;
    }

    return status;
}

/**
 * @brief 停止freertos软件定时器方法
 * @param timer 
 * @return true 
 * @return false 
 */
bool method_timer_stop(TimerHandle_t *timer)
{
    bool status = false;

    if (xTimerStop(*timer, portMAX_DELAY) != pdPASS) {
        status = false;
    } else {
        status = true;
    }

    return status;
}

/**
 * @brief 删除软件定时器
 * @param timer 
 * @return true 
 * @return false 
 */
bool method_timer_delete(TimerHandle_t *timer)
{
    xTimerDelete(*timer, portMAX_DELAY);
    *timer = NULL;

    return true;
}

/**
 * @brief 修改计时周期
 * @param timer 
 * @param time_out 
 * @return true 
 * @return false 
 */
bool method_timer_change_period(TimerHandle_t *timer,uint32_t time_out)
{
    xTimerChangePeriod(*timer,time_out/portTICK_PERIOD_MS,portMAX_DELAY);

    return true;
}

/**
 * @brief 创建软件定时器
 * @param timer 
 * @param time_out 
 * @param reload 是否重载
 * @param cb 定时到期回调
 * @return true 
 * @return false 
 */
bool method_timer_create(TimerHandle_t *timer,uint32_t time_out ,bool reload,TimerCallbackFunction_t cb)
{
    if(NULL == cb) return false;
    *timer = xTimerCreate("method_timer", time_out / portTICK_PERIOD_MS, reload,
                                         NULL, cb);
    if(*timer == NULL){
        return false;
    }else{
        return true;
    }
}
