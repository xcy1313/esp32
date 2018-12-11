/**
 * @brief mqtt客户端
 */

#ifndef VESYNC_MQTT_H
#define VESYNC_MQTT_H

#include "mqtt_client.h"

typedef enum
{
	MQTT_QOS0 = 0,
	MQTT_QOS1 = 1,
	MQTT_QOS2 = 2
} mqtt_qos_e;

/**
 * @brief vesync平台mqtt客户端
 */
typedef struct
{
	esp_mqtt_client_config_t client_config;
	esp_mqtt_client_handle_t client_handle;
} vesync_mqtt_client_t;

/**
 * @brief 初始化mqtt客户端
 * @param vesync_client [vesync mqtt客户端对象指针]
 * @param client_id 	[设备的cid]
 * @param server_addr 	[mqtt服务器地址]
 * @param server_port 	[mqtt服务器端口]
 * @param username 		[登陆用户名]
 * @param password 		[登陆密码]
 * @param event_cb		[mqtt事件回调函数]
 * @return int 			[初始化结果]
 */
int vesync_init_mqtt_client(vesync_mqtt_client_t *vesync_client, char *client_id, char *server_addr, uint16_t server_port,
                            char *username, char *password, mqtt_event_callback_t event_cb);

/**
 * @brief 初始化mqtt客户端
 * @param vesync_client [vesync mqtt客户端对象指针]
 * @param client_id 	[设备的cid]
 * @param server_addr 	[mqtt服务器地址]
 * @param server_port 	[mqtt服务器端口]
 * @param username 		[登陆用户名]
 * @param password 		[登陆密码]
 * @param event_cb		[mqtt事件回调函数]
 * @param ca_cert		[ca证书]
 * @param client_cert	[客户端证书]
 * @param client_key	[客户端秘钥]
 * @return int 			[初始化结果]
 */
int vesync_init_mqtt_client_with_tls(vesync_mqtt_client_t *vesync_client, char *client_id, char *server_addr, uint16_t server_port,
                                     char *username, char *password, mqtt_event_callback_t event_cb,
                                     const char * ca_cert, const char *client_cert, const char *client_key);

/**
 * @brief 删除mqtt客户端
 * @param vesync_client [vesync mqtt客户端对象指针]
 * @return int 			[删除结果]
 */
int vesync_delete_mqtt_client(vesync_mqtt_client_t *vesync_client);

/**
 * @brief 设置mqtt的tls加密证书密钥
 * @param vesync_client	[vesync mqtt客户端对象指针]
 * @param ca_cert		[ca证书]
 * @param client_cert	[客户端证书]
 * @param client_key	[客户端秘钥]
 */
void vesync_set_mqtt_tls_cert_and_key(vesync_mqtt_client_t *vesync_client, const char * ca_cert, const char *client_cert, const char *client_key);

/**
 * @brief mqtt发起连接
 * @param vesync_client [vesync mqtt客户端对象指针]
 * @return int 			[操作结果，0为成功]
 */
int vesync_mqtt_connect(vesync_mqtt_client_t *vesync_client);

/**
 * @brief mqtt断开连接
 * @param vesync_client [vesync mqtt客户端对象指针]
 * @return int 			[操作结果，0为成功]
 */
int vesync_mqtt_disconnect(vesync_mqtt_client_t *vesync_client);

/**
 * @brief mqtt订阅消息主题
 * @param vesync_client	[vesync mqtt客户端对象指针]
 * @param topic 		[消息主题]
 * @param qos 			[QoS消息质量]
 * @return int 			[订阅结果]
 */
int vesync_mqtt_subscribe_topic(vesync_mqtt_client_t *vesync_client, char *topic, int qos);

/**
 * @brief mqtt取消订阅消息主题
 * @param vesync_client [vesync mqtt客户端对象指针]
 * @param topic 		[消息主题]
 * @return int 			[取消订阅结果]
 */
int vesync_mqtt_unsubscribe_topic(vesync_mqtt_client_t *vesync_client, char *topic);

/**
 * @brief 发布原始格式的数据到指定主题
 * @param vesync_client [vesync mqtt客户端对象指针]
 * @param topic 		[发布的主题]
 * @param data 			[原始通用格式的数据]
 * @param datalen 		[数据长度]
 * @param qos 			[发布消息的qos]
 * @param retain 		[发布消息的retain]
 * @return int	 		[发布结果，0 - 成功存入mqtt消息发布队列]
 */
int vesync_mqtt_publish_rawdata_to_topic(vesync_mqtt_client_t *vesync_client, char* topic, char* data, int datalen, int qos, int retain);

#endif
