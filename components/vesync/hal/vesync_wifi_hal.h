/**
 * @file vesync_wifi_hal.h
 * @brief vesync设备WiFi功能硬件抽象层
 * @author WatWu
 * @date 2018-11-17
 */

#ifndef VESYNC_WIFI_HAL_H
#define VESYNC_WIFI_HAL_H

#include "esp_wifi.h"
#include <stdbool.h>

//硬件抽象层WiFi接口定义
#define HAL_WIFI_INTERFACE_STA			ESP_IF_WIFI_STA
#define HAL_WIFI_INTERFACE_AP			ESP_IF_WIFI_AP

//硬件抽象层WiFi模式定义
#define HAL_WIFI_MODE_NULL				WIFI_MODE_NULL
#define HAL_WIFI_MODE_AP				WIFI_MODE_AP
#define HAL_WIFI_MODE_STA				WIFI_MODE_STA
#define HAL_WIFI_MODE_APSTA				WIFI_MODE_APSTA

//硬件抽象层WiFi加密方式定义
#define HAL_WIFI_AUTH_OPEN				WIFI_AUTH_OPEN
#define HAL_WIFI_AUTH_WEP				WIFI_AUTH_WEP
#define HAL_WIFI_AUTH_WPA_PSK			WIFI_AUTH_WPA_PSK
#define HAL_WIFI_AUTH_WPA2_PSK			WIFI_AUTH_WPA2_PSK
#define HAL_WIFI_AUTH_WPA_WPA2_PSK		WIFI_AUTH_WPA_WPA2_PSK
#define HAL_WIFI_AUTH_WPA2_ENTERPRISE	WIFI_AUTH_WPA2_ENTERPRISE

//硬件抽象层WiFi 2.4G信道定义
#define HAL_WIFI_2_4G_CHANNEL1			1
#define HAL_WIFI_2_4G_CHANNEL2			2
#define HAL_WIFI_2_4G_CHANNEL3			3
#define HAL_WIFI_2_4G_CHANNEL4			4
#define HAL_WIFI_2_4G_CHANNEL5			5
#define HAL_WIFI_2_4G_CHANNEL6			6
#define HAL_WIFI_2_4G_CHANNEL7			7
#define HAL_WIFI_2_4G_CHANNEL8			8
#define HAL_WIFI_2_4G_CHANNEL9			9
#define HAL_WIFI_2_4G_CHANNEL10			10
#define HAL_WIFI_2_4G_CHANNEL11			11
#define HAL_WIFI_2_4G_CHANNEL12			12
#define HAL_WIFI_2_4G_CHANNEL13			13
#define HAL_WIFI_2_4G_CHANNEL14			14

/**
 * @brief wifi连接状态枚举
 */
typedef enum
{
	VESYNC_WIFI_INIT =-1,
	VESYNC_WIFI_CONNECTING =0,
	VESYNC_WIFI_GOT_IP ,
	VESYNC_WIFI_LOST_IP,
	VESYNC_WIFI_WRONG_PASSWORD ,
	VESYNC_WIFI_NO_AP_FOUND ,
	VESYNC_WIFI_CONNECT_FAIL ,
	VESYNC_WIFI_SCAN_DONE ,
} vesync_wifi_status_e;

/**
* @brief 连接WiFi回调函数指针
*/
typedef void (*vesync_wifi_cb)(vesync_wifi_status_e wifi_status);

/**
 * @brief 硬件抽象层初始化wifi模块
 */
void vesync_hal_init_wifi_module(vesync_wifi_cb callback,bool power_save);

/**
 * @brief 硬件抽象层设置设备为WiFi热点
 * @param ap_ssid 	[ssid名称]
 * @param password 	[WiFi密码]
 * @param chan 		[WiFi信道]
 * @param auth_mode [加密方式]
 */
void vesync_hal_setup_wifi_ap(char *ap_ssid, char* password, uint8_t chan, uint8_t auth_mode);

/**
 * @brief 硬件抽象层设置设备为WiFi热点+客户端共存
 * @param ap_ssid 	[ssid名称]
 * @param password 	[WiFi密码]
 * @param chan 		[WiFi信道]
 * @param auth_mode [加密方式]
 */
void vesync_hal_setup_wifi_ap_and_sta(char *ap_ssid, char* password, uint8_t chan, uint8_t auth_mode);

/**
 * @brief 硬件抽象层设置设备的wifi模式
 * @param target_mode [目标模式]
 * @return int8_t 	  [设置结果，0为成功]
 */
int8_t vesync_hal_set_wifi_mode(uint8_t target_mode);

/**
 * @brief 硬件抽象层获取设备当前wifi模式
 * @return uint8_t [当前wifi模式]
 */
uint8_t vesync_hal_get_wifi_mode(void);

/**
 * @brief 硬件抽象层连接WiFi
 * @param wifi_ssid			[WiFi名称]
 * @param wifi_password 	[WiFi密码]
 * @param power_save 		[WiFi开启省电模式]
 */
void vesync_hal_connect_wifi(char *ssid ,char *pwd);

/**
 * @brief 硬件抽象层获取mac地址字符串
 * @param interface 		[WiFi接口，AP或STA]
 * @param mac_str_buffer 	[mac地址字符串缓存区，大小必须大于等于18字节]
 * @return int 				[获取结果]
 */
int vesync_hal_get_mac_string(int interface, char *mac_str_buffer);

/**
 * @brief 硬件抽象层开启扫描wifi列表
 * @return int 返回当前状态
 */
int vesync_hal_scan_wifi_list_start(void);

/**
 * @brief 硬件抽象层关闭wifi扫描
 */
int vesync_hal_scan_stop(void);

#endif
