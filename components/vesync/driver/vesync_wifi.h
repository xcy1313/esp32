/**
 * @file vesync_wifi.h
 * @brief vesync设备的WiFi连接等相关功能驱动
 * @author WatWu
 * @date 2018-11-17
 */

#ifndef VESYNC_WIFI_H
#define VESYNC_WIFI_H

#include "vesync_wifi_hal.h"

/**
 * @brief 阻塞等待网络连接成功
 * @param wait_time [等待时间，单位：毫秒]
 * @return int 		[等待结果，0 - 等待网络连接成功；-1 - 等待网络连接超时]
 */
int vesync_wait_network_connected(uint32_t	wait_time);

/**
 * @brief 初始化wifi模块
 */
void vesync_init_wifi_module(vesync_wifi_cb callback,bool power_save);

/**
 * @brief 设置设备为开放热点
 * @param ap_ssid [热点的名称]
 */
void vesync_setup_wifi_open_ap(char *ap_ssid);

/**
 * @brief 设置设备为开放热点和客户端共存
 * @param ap_ssid [热点的名称]
 */
void vesync_setup_wifi_open_ap_and_sta(char *ap_ssid);

/**
 * @brief 设置设备的wifi模式
 * @param target_mode [目标模式]
 * @return int8_t 	  [设置结果，0为成功]
 */
int8_t vesync_set_wifi_mode(uint8_t target_mode);

/**
 * @brief 获取当前wifi模式
 * @return uint8_t [当前wifi模式]
 */
uint8_t vesync_get_wifi_mode(void);

/**
 * @brief 设备连接WiFi
 * @param wifi_ssid	[WiFi名称]
 * @param wifi_key 	[WiFi密码]
 * @param callback 	[WiFi连接回调函数]
 */
void vesync_connect_wifi(char *wifi_ssid, char *wifi_password);

/**
 * @brief 获取设备WiFi客户端模式下的mac地址字符串
 * @param mac_str_buffer [mac地址字符串缓存区，大小必须大于等于18字节]
 */
void vesync_get_wifi_sta_mac_string(char *mac_str_buffer);

/**
 * @brief 获取当前模式下的wifi配置
 * @param mode 
 * @return uint8_t 
 */
uint8_t vesync_get_wifi_config(wifi_interface_t interface,wifi_config_t *cfg);

/**
 * @brief 开启wifi扫描
 * @param callback 	[WiFi连接回调函数]
 * @return int 
 */
int vesync_scan_wifi_list_start(void);

/**
 * @brief 停止wifi扫描
 * @return int 
 */
int vesync_scan_wifi_list_stop(void);

/**
 * @brief 阻塞等待网络断开成功
 * @param wait_time [等待时间，单位：毫秒]
 * @return int 		[等待结果，0 - 等待网络连接成功；-1 - 等待网络连接超时]
 */
int vesync_wait_network_disconnected(uint32_t	wait_time);

/**
 * @brief 阻塞等待网络连接成功
 * @param wait_time 
 * @return int 
 */
int vesync_wait_network_connected(uint32_t	wait_time);

/**
 * @brief 获取wifi状态
 * @return vesync_wifi_status_e 
 */
vesync_wifi_status_e vesync_wifi_get_status(void);

#endif
