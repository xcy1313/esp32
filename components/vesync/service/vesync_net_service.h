/**
 * @file vesync_net_service.h
 * @brief 网络基础业务
 * @author WatWu
 * @date 2018-12-05
 */

#ifndef VESYNC_NET_SERVICE_H
#define VESYNC_NET_SERVICE_H

#include "vesync_mqtt.h"

typedef enum
{
	MQTT_ONLINE = 0,
	MQTT_OFFLINE = 1
}vesync_mqtt_status_e;

/**
 * @brief 获取mqtt客户端在线状态
 * @return vesync_mqtt_status_e [mqtt客户端在线状态]
 */
vesync_mqtt_status_e vesync_get_mqtt_client_status(void);

/**
 * @brief 获取云平台mqtt客户端指针
 * @return vesync_mqtt_client_t* [vesync云平台mqtt客户端指针]
 */
vesync_mqtt_client_t* vesync_get_cloud_mqtt_client(void);

/**
 * @brief vesync客户端连接WiFi
 * @param wifi_ssid 	[WiFi账号]
 * @param wifi_password [WiFi密码]
 */
void vesync_client_connect_wifi(char *wifi_ssid, char *wifi_password);

/**
 * @brief 配置vesync云平台的mqtt客户端
 * @param client_id 	[设备cid]
 * @param server_addr 	[mqtt服务器地址]
 * @param server_port 	[mqtt服务器端口]
 * @param username 		[mqtt用户名]
 * @param password 		[mqtt用户密码]
 */
void vesync_config_cloud_mqtt_client(char *client_id, char *server_addr, uint16_t server_port, char *username, char *password);

/**
 * @brief 配置vesync云平台的mqtt客户端
 * @param client_id 	[设备cid]
 * @param server_addr 	[mqtt服务器地址]
 * @param server_port 	[mqtt服务器端口]
 * @param username 		[mqtt用户名]
 * @param password 		[mqtt用户密码]
 */
void vesync_config_cloud_mqtt_client_with_tls(char *client_id, char *server_addr, uint16_t server_port, char *username, char *password);

/**
 * @brief vesync平台mqtt客户端连接与服务器
 */
void vesync_mqtt_client_connect_to_cloud(void);

/**
 * @brief vesync平台mqtt客户端断开与服务器的连接
 */
void vesync_mqtt_client_disconnet_from_cloud(void);

#endif