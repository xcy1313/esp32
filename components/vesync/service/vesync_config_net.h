/**
 * @file vesync_config_net.h
 * @brief vesync设备配网操作
 * @author WatWu
 * @date 2018-11-21
 */

#ifndef VESYNC_CONFIG_NET
#define VESYNC_CONFIG_NET

#include <stdint.h>

/**
 * @brief 设备配网操作结果
 */
typedef enum
{
	CONFIG_NET_TIMEOUT = 0,
	CONFIG_NET_SUCCEED = 1,
	CONFIG_NET_CANCEL  = 2,
} config_net_result_e;

/**
 * @brief 配网模式
 */
typedef enum
{
	DONOT_CONFIG_NET = 0,				//处于非配网模式
	SMART_CONFIG	 = 1,				//smartconfig模式配网
	AP_CONFIG		 = 2,				//AP模式配网
	RESET_NETWORK	 = 3,				//清除保存的配网数据
	CONFIG_TIME_OUT  = 4,				//配网超时
} config_network_mode_e;

/**
 * [config_network_cb_t 配网操作结果回调函数指针]
 * @param  int [配网操作结果，CONFIG_NET_SUCCEED - 配网成功；CONFIG_NET_TIMEOUT - 配网超时；CONFIG_NET_CANCEL - 配网取消]
 * @return     [无]
 */
typedef void (*config_network_cb_t)(int);

/**
 * [vesync_get_network_configmode 获取当前配网模式]
 * @return  [当前配网模式]
 */
config_network_mode_e vesync_get_network_configmode(void);

/**
 * [vesync_set_network_configmode 设置配网模式]
 * @param  mode [配网模式]
 * @return      [无]
 */
void vesync_set_network_configmode(config_network_mode_e mode);

/**
 * [vesync_configure_network 启动配网]
 * @param  config_mode [配网模式，SMART_CONFIG - smartconfig模式配网；AP_CONFIG - AP模式配网；RESET_NETWORK - 清除配网数据]
 * @param  resultCB    [配网结果回调函数]
 * @return             [无]
 */
void vesync_configure_network(config_network_mode_e config_mode, config_network_cb_t resultCB);

/**
 * @brief 取消配网
 */
void vesync_cancel_config_net(void);

/**
 * @brief 启动配网监控定时器
 * @param out_time [设置定时器超时时间，单位毫秒]
 */
void vesync_start_config_net_monitor_timer(uint32_t out_time);

#endif
