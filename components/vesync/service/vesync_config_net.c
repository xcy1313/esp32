/**
 * @file vesync_config_net.c
 * @brief vesync设备配网操作
 * @author WatWu
 * @date 2018-11-21
 */

#include "vesync_config_net.h"
#include "vesync_wifi.h"
#include "vesync_main.h"
#include "freertos/timers.h"

// static const char* TAG = "vesync_confnet";

static config_network_mode_e config_network_mode = DONOT_CONFIG_NET; 	//配网模式，默认处于非配网模式
static TimerHandle_t netconf_timeout_timer;								//配网超时判断定时器
static uint8_t netconf_timer_init_flag = false;							//配网超时判断定时器初始化标志
static config_network_cb_t	config_network_callback = NULL;				//定义配网操作结果回调函数指针

/**
 * [vesync_get_network_configmode 获取当前配网模式]
 * @return  [当前配网模式]
 */
config_network_mode_e vesync_get_network_configmode(void)
{
	return config_network_mode;
}

/**
 * [vesync_set_network_configmode 设置配网模式]
 * @param  mode [配网模式]
 * @return      [无]
 */
void vesync_set_network_configmode(config_network_mode_e mode)
{
	config_network_mode = mode;
}

/**
 * [vesync_configure_network 启动配网]
 * @param  config_mode [配网模式，SMART_CONFIG - smartconfig模式配网；AP_CONFIG - AP模式配网；RESET_NETWORK - 清除配网数据]
 * @param  resultCB    [配网结果回调函数]
 * @return             [无]
 */
void vesync_configure_network(config_network_mode_e config_mode, config_network_cb_t resultCB)
{
	config_network_callback = resultCB;

	switch(config_mode)
	{
		case SMART_CONFIG:
			//未配网则启动配网，已配网则清除数据并重新进行配网
			// if(vesync_get_device_status() == DEV_CONFNET_NOT_CON)			//未配网，直接发送配网消息
			// {
			// 	//此处无动作
			// }
			// else if(vesync_get_device_status() == DEV_CONFNET_ONLINE)		//已连接服务器，先断线，然后清空配置再启动重连
			// {
			// 	if(vesync_mqtt_get_status() == ON_LINE)
			// 	{
			// 		vesync_mqtt_disconnect();
			// 	}
			// 	vesync_set_device_status(DEV_CONFNET_OFFLINE);

			// 	vesync_reset_device_conf(&g_device_config);					//恢复出厂设置，但暂不写入flash
			// }
			// else if(vesync_get_device_status() == DEV_CONFNET_OFFLINE)		//未连接，清空配置后启动重连
			// {
			// 	vesync_reset_device_conf(&g_device_config);					//恢复出厂设置
			// }

			// if(vesync_get_wifi_mode() != WIFI_MODE_AP_ONLY)
			// {
			// 	wifi_connection_disconnect_ap();
			// }

			// //不可放到switch外面
			// vesync_set_mqtt_ready_flag(FALSE);										//清除mqtt初始化完成标记
			// //set_config_log_saved_flag(FALSE);								//清除当前配网错误日志保存标志
			break;

		case AP_CONFIG:
			//未配网则启动配网，已配网则清除数据并重新进行配网
			// if(vesync_get_device_status() == DEV_CONFNET_ONLINE)		//已连接服务器，先断线，然后清空配置再启动重连
			// {
			// 	if(vesync_mqtt_get_status() == ON_LINE)
			// 	{
			// 		vesync_mqtt_disconnect();
			// 	}
			// 	vesync_set_device_status(DEV_CONFNET_OFFLINE);

			// 	vesync_reset_device_conf(&g_device_config);					//恢复出厂设置，但暂不写入flash
			// }
			// else if(vesync_get_device_status() == DEV_CONFNET_OFFLINE)		//未连接，清空配置后启动重连
			// {
			// 	vesync_reset_device_conf(&g_device_config);					//恢复出厂设置
			// }

			// if(vesync_get_wifi_mode() != WIFI_MODE_AP_ONLY)
			// {
			// 	wifi_connection_disconnect_ap();
			// }
			// wifi_config_set_opmode(WIFI_MODE_STA_ONLY);						//先进入STA模式，以断开可能的原先作为热点时连接上的设备

			// //不可放到switch外面
			// vesync_set_mqtt_ready_flag(FALSE);								//清除mqtt初始化完成标记
			// //set_config_log_saved_flag(FALSE);								//清除当前配网错误日志保存标志

			// vesync_set_wifiled_behavior(WIFI_LED_APN_CONFIG);				//AP模式时，1秒的闪烁频率，连续闪烁
			// vesync_set_network_configmode(AP_CONFIG);
			// vesync_set_ap_mode("VeSync_template");
			// vesync_tcp_server_start(VESYNC_TCP_SERVER_PORT, vesync_tcpserver_recv_callback);
			vesync_start_config_net_monitor_timer(5 * 60 * 1000);					//5分钟配网超时判断
			break;

		case RESET_NETWORK:
			// vesync_set_wifiled_behavior(WIFI_LED_RESET_DEVICE);
			// vesync_reset_device_conf(&g_device_config);										//恢复出厂设置
			// int ret = vesync_configuration_write_flash(NVDM_ITEM_DEVICE_CONFIG, &g_device_config, sizeof(g_device_config));	//保存至flash
			// if(NVDM_STATUS_OK != ret)
			// {
			// 	LOG_E(TAG, "Configuration write to flash error : %d !", ret);
			// }
			// LOG_I(TAG, "Restore device success!");

			// if(NULL != restore_device_callback)
			// {
			// 	restore_device_callback();
			// }

			// //不可放到switch外面
			// vesync_set_mqtt_ready_flag(FALSE);										//清除mqtt初始化完成标记
			//set_config_log_saved_flag(FALSE);								//清除当前配网错误日志保存标志

			//清除数据成功，启动系统复位定时器，3秒后重启系统
			break;

		default:
			break;
	}
}

/**
 * @brief 取消配网
 */
void vesync_cancel_config_net(void)
{
	// if(vesync_get_network_configmode() != DONOT_CONFIG_NET)		//还处于配网状态
	// {
	// 	LOG_W(TAG, "Cannel config network !");

	// 	vesync_set_wifiled_behavior(WIFI_LED_NOT_CONFIG); 		//取消配网，进入未配网模式，LED灯常灭

	// 	if(vesync_get_network_configmode() == SMART_CONFIG)		//如果是smartconfig配网还要停止smartconfig
	// 	{
	// 		// smartconfig_stop();
	// 	}

	// 	if(vesync_get_wifi_mode() != WIFI_MODE_STA_ONLY)		//如果是处于非客户端模式，则切换回客户端模式
	// 	{
	// 		wifi_config_set_opmode(WIFI_MODE_STA_ONLY);
	// 	}

	// 	if(NULL != config_network_callback)
	// 		config_network_callback(CONFIG_NET_CANCEL);			//配网操作回调，配网取消

	// 	vesync_set_device_status(DEV_CONFNET_NOT_CON);			//设置设备未配网状态
	// 	vesync_set_network_configmode(DONOT_CONFIG_NET);		//标记退出配网模式
	// 	vesync_mqtt_disconnect();								//停止连接mqtt
	// 	vesync_set_mqtt_ready_flag(FALSE);

	// 	LOG_I(TAG, "Reset ssid and password");
	// 	wifi_connection_disconnect_ap();

	// 	vesync_tcp_server_stop();
	// }
}

/**
 * [netconf_timeout_callback 配网超时判断定时器回调函数]
 * @param  arg [回调函数参数]
 * @return     [无]
 */
static void netconf_timeout_callback(void *arg)
{
	vesync_cancel_config_net();
	// vesync_tcp_server_stop();
	xTimerStop(netconf_timeout_timer, TIMER_BLOCK_TIME);		//停止超时定时器
}

/**
 * @brief 启动配网监控定时器
 * @param out_time [设置定时器超时时间，单位毫秒]
 */
void vesync_start_config_net_monitor_timer(uint32_t out_time)
{
	if(false == netconf_timer_init_flag)
	{
		netconf_timer_init_flag = true;
		//创建配网超时定时器，单次模式
		netconf_timeout_timer = xTimerCreate("net_conf", 1000 / portTICK_RATE_MS, pdTRUE, NULL, netconf_timeout_callback);
	}
	xTimerStop(netconf_timeout_timer, TIMER_BLOCK_TIME);
	xTimerChangePeriod(netconf_timeout_timer, out_time / portTICK_RATE_MS, TIMER_BLOCK_TIME);
	xTimerStart(netconf_timeout_timer, TIMER_BLOCK_TIME);
}
