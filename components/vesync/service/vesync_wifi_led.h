/**
 * @file vesync_wifi_led.h
 * @brief vesync设备统一的WiFi指示灯行为
 * @author WatWu
 * @date 2018-11-19
 */

#ifndef VESYNC_WIFI_LED_H
#define VESYNC_WIFI_LED_H

/**
 * @brief wifi指示灯状态
 */
typedef enum
{
	WIFI_LED_OFF = 0,
	WIFI_LED_ON = 1,
	WIFI_LED_BLINK = 2,
	WIFI_LED_DOUBLE_BLINK = 3
} wifi_led_status_e;

/**
 * @brief wifi指示灯行为
 */
typedef enum
{
	WIFI_LED_LOGIN_SUCCESS = 0,			//连接服务器成功
	WIFI_LED_NOT_CONFIG = 1,			//设备未配网、配网超时
	WIFI_LED_WIFI_LINK_FAIL = 2,		//WiFi连接失败
	WIFI_LED_SERVER_LINK_FAIL = 3,		//WiFi连接成功，服务器连接失败
	WIFI_LED_SMART_CONFIG = 4,			//smartconfig模式配网
	WIFI_LED_APN_CONFIG = 5,			//APN模式配网
	WIFI_LED_RESET_DEVICE = 6,			//复位设备
	WIFI_LED_NET_LINKING = 7,			//连接中
} wifi_led_behavior_e;

/**
 * @brief 状态回调函数指针
 * @param int [wifi指示灯当前状态，ON - 亮；OFF - 灭]
 */
typedef void (*wifi_led_turn_cb_t)(int);

/**
 * [vesync_set_wifiled_behavior 设置WIFI指示灯闪烁模式]
 * @param  ledBehavior [WiFi指示灯闪烁行为]
 * @return      	   [无]
 */
void vesync_set_wifiled_behavior(wifi_led_behavior_e ledBehavior);

/**
 * @brief 注册WiFi LED指示灯状态变化回调函数
 * @param cb [WiFi指示灯变化回调函数，每次指示灯进行亮灭变化时都会调用该函数]
 */
void vesync_regist_wifiled_cb(wifi_led_turn_cb_t cb);

#endif
