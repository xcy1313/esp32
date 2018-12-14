/**
 * @file vesync_wifi.c
 * @brief vesync设备的WiFi连接等相关功能
 * @author WatWu
 * @date 2018-11-09
 */

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "vesync_log.h"
#include "vesync_wifi.h"

//WiFi状态、网络状态事件标志组位定义，置位代表连接成功
#define EVENT_BIT_WIFI_STATUS		0X000001
#define EVENT_BIT_NETWORK_STATUS	0X000002

static const char *TAG = "vesync_wifi";

//网络事件标志组，从右往左，0位表示wifi状态，1位表示网络状态，置位代表连接成功
static EventGroupHandle_t s_network_event_group = NULL;

static vesync_wifi_cb s_vesync_wifi_callback = NULL;		//wifi连接状态回调函数指针

/**
 * @brief 连接WiFi的底层回调
 * @param status [WiFi连接状态，包括连接成功和连接掉线，以及连接失败时的错误原因]
 */
static void hal_connect_wifi_callback(vesync_wifi_status_e status)
{
	switch(status)						//驱动层根据需要使用部分WiFi状态
	{
		case VESYNC_WIFI_GOT_IP:
			xEventGroupSetBits(s_network_event_group, EVENT_BIT_NETWORK_STATUS);
			break;
		case VESYNC_WIFI_LOST_IP:
			xEventGroupClearBits(s_network_event_group, EVENT_BIT_NETWORK_STATUS);
			break;
		case VESYNC_WIFI_SCAN_DONE:
			LOG_I(TAG, "VESYNC_WIFI_SCAN_DONE!");
			break;
		default:
			break;
	}

	if(NULL != s_vesync_wifi_callback)	//驱动层调用上一层注册的回调函数，WiFi状态传递给上一层继续处理
	{
		s_vesync_wifi_callback(status);
	}
}

/**
 * @brief 阻塞等待网络连接成功
 * @param wait_time [等待时间，单位：毫秒]
 * @return int 		[等待结果，0 - 等待网络连接成功；-1 - 等待网络连接超时]
 */
int vesync_wait_network_connected(uint32_t	wait_time)
{
	int ret;
	//阻塞等待网络事件标志组网络连接标志置位
	ret = xEventGroupWaitBits(s_network_event_group,
	                          EVENT_BIT_NETWORK_STATUS,
	                          pdFALSE,
	                          pdTRUE,
	                          wait_time / portTICK_RATE_MS);
	if(ret & EVENT_BIT_NETWORK_STATUS)
		return 0;
	else
		return -1;
}

/**
 * @brief 初始化wifi模块
 */
void vesync_init_wifi_module(void)
{
	vesync_hal_init_wifi_module();

	s_network_event_group = xEventGroupCreate();
	LOG_I(TAG, "Init wifi module done.");
}

/**
 * @brief 设置设备为开放热点
 * @param ap_ssid [热点的名称]
 */
void vesync_setup_wifi_open_ap(char *ap_ssid)
{
	vesync_hal_setup_wifi_ap(ap_ssid, NULL, HAL_WIFI_2_4G_CHANNEL1, HAL_WIFI_AUTH_OPEN);
	LOG_I(TAG, "WiFi setup open ap, ssid : %s", ap_ssid);
}

/**
 * @brief 设置设备为开放热点和客户端共存
 * @param ap_ssid [热点的名称]
 */
void vesync_setup_wifi_open_ap_and_sta(char *ap_ssid)
{
	vesync_hal_setup_wifi_ap_and_sta(ap_ssid, NULL, HAL_WIFI_2_4G_CHANNEL1, HAL_WIFI_AUTH_OPEN);
	LOG_I(TAG, "WiFi setup open ap and sta, ssid : %s", ap_ssid);
}

/**
 * @brief 设置设备的wifi模式
 * @param target_mode [目标模式]
 * @return int8_t 	  [设置结果，0为成功]
 */
int8_t vesync_set_wifi_mode(uint8_t target_mode)
{
	int8_t ret;
	ret	= vesync_hal_set_wifi_mode(target_mode);
	return ret;
}

/**
 * @brief 获取当前wifi模式
 * @return uint8_t [当前wifi模式]
 */
uint8_t vesync_get_wifi_mode(void)
{
	uint8_t mode;
	mode = vesync_hal_get_wifi_mode();
	return mode;
}

/**
 * @brief 设备连接WiFi
 * @param wifi_ssid	[WiFi名称]
 * @param wifi_key 	[WiFi密码]
 * @param callback 	[WiFi连接回调函数]
 */
void vesync_connect_wifi(char *wifi_ssid, char *wifi_password, vesync_wifi_cb callback)
{
	vesync_hal_connect_wifi(wifi_ssid, wifi_password, hal_connect_wifi_callback);
	LOG_I(TAG, "Connect to ap ssid : %s, password : %s ", wifi_ssid, wifi_password);
	
	if(NULL != callback)
	{
		s_vesync_wifi_callback = callback;
	}
}

/**
 * @brief 获取设备WiFi客户端模式下的mac地址字符串
 * @param mac_str_buffer [mac地址字符串缓存区，大小必须大于等于18字节]
 */
void vesync_get_wifi_sta_mac_string(char *mac_str_buffer)
{
	vesync_hal_get_mac_string(HAL_WIFI_INTERFACE_STA, mac_str_buffer);
}
