/**
 * @file vesync_mqtt.c
 * @brief MQTT客户端
 * @author WatWu
 * @date 2018-08-06
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "vesync_mqtt.h"
#include "vesync_log.h"

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
                            char *username, char *password, mqtt_event_callback_t event_cb)
{
	memset(vesync_client, 0, sizeof(vesync_mqtt_client_t));

	vesync_client->client_config.client_id = pvPortMalloc(strlen(client_id) + 1);
	strcpy((char*)vesync_client->client_config.client_id, client_id);
	vesync_client->client_config.host = pvPortMalloc(strlen(server_addr) + 1);
	strcpy((char*)vesync_client->client_config.host, server_addr);

	vesync_client->client_config.port = server_port;

	vesync_client->client_config.username = pvPortMalloc(strlen(username) + 1);
	strcpy((char*)vesync_client->client_config.username, username);
	vesync_client->client_config.password = pvPortMalloc(strlen(password) + 1);
	strcpy((char*)vesync_client->client_config.password, password);

	vesync_client->client_config.event_handle = event_cb;

	vesync_client->client_handle = esp_mqtt_client_init(&(vesync_client->client_config));

	return 0;
}

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
                                     const char * ca_cert, const char *client_cert, const char *client_key)
{
	memset(vesync_client, 0, sizeof(vesync_mqtt_client_t));

	vesync_client->client_config.client_id = pvPortMalloc(strlen(client_id) + 1);
	strcpy((char*)vesync_client->client_config.client_id, client_id);
	vesync_client->client_config.host = pvPortMalloc(strlen(server_addr) + 1);
	strcpy((char*)vesync_client->client_config.host, server_addr);

	vesync_client->client_config.port = server_port;

	vesync_client->client_config.username = pvPortMalloc(strlen(username) + 1);
	strcpy((char*)vesync_client->client_config.username, username);
	vesync_client->client_config.password = pvPortMalloc(strlen(password) + 1);
	strcpy((char*)vesync_client->client_config.password, password);

	vesync_client->client_config.event_handle = event_cb;

	vesync_client->client_config.transport = MQTT_TRANSPORT_OVER_SSL;

	if(NULL != vesync_client->client_config.cert_pem)
		free((char*)vesync_client->client_config.cert_pem);
	if(NULL != vesync_client->client_config.client_cert_pem)
		free((char*)vesync_client->client_config.client_cert_pem);
	if(NULL != vesync_client->client_config.client_key_pem)
		free((char*)vesync_client->client_config.client_key_pem);

	//此处不检验CA证书，否则会出现tls握手失败
	// vesync_client->client_config.cert_pem = pvPortMalloc(strlen(ca_cert) + 1);
	// if(NULL != vesync_client->client_config.cert_pem)
	// 	strcpy((char*)vesync_client->client_config.cert_pem, ca_cert);

	vesync_client->client_config.client_cert_pem = pvPortMalloc(strlen(client_cert) + 1);
	if(NULL != vesync_client->client_config.client_cert_pem)
		strcpy((char*)vesync_client->client_config.client_cert_pem, client_cert);
	vesync_client->client_config.client_key_pem = pvPortMalloc(strlen(client_key) + 1);
	if(NULL != vesync_client->client_config.client_key_pem)
		strcpy((char*)vesync_client->client_config.client_key_pem, client_key);

	vesync_client->client_handle = esp_mqtt_client_init(&(vesync_client->client_config));

	return 0;
}

/**
 * @brief 删除mqtt客户端
 * @param vesync_client [vesync mqtt客户端对象指针]
 * @return int 			[删除结果]
 */
int vesync_delete_mqtt_client(vesync_mqtt_client_t *vesync_client)
{
	int ret;
	ret = esp_mqtt_client_destroy(vesync_client->client_handle);
	return ret;
}

/**
 * @brief mqtt发起连接
 * @param vesync_client [vesync mqtt客户端对象指针]
 * @return int 			[操作结果，0为成功]
 */
int vesync_mqtt_connect(vesync_mqtt_client_t *vesync_client)
{
	int ret;
	ret = esp_mqtt_client_start(vesync_client->client_handle);
	return ret;
}

/**
 * @brief mqtt断开连接
 * @param vesync_client [vesync mqtt客户端对象指针]
 * @return int 			[操作结果，0为成功]
 */
int vesync_mqtt_disconnect(vesync_mqtt_client_t *vesync_client)
{
	int ret;
	ret = esp_mqtt_client_stop(vesync_client->client_handle);
	return ret;
}

/**
 * @brief mqtt订阅消息主题
 * @param vesync_client	[vesync mqtt客户端对象指针]
 * @param topic 		[消息主题]
 * @param qos 			[QoS消息质量]
 * @return int 			[订阅结果]
 */
int vesync_mqtt_subscribe_topic(vesync_mqtt_client_t *vesync_client, char *topic, int qos)
{
	int ret;
	ret = esp_mqtt_client_subscribe(vesync_client->client_handle, topic, qos);
	return ret;
}

/**
 * @brief mqtt取消订阅消息主题
 * @param vesync_client [vesync mqtt客户端对象指针]
 * @param topic 		[消息主题]
 * @return int 			[取消订阅结果]
 */
int vesync_mqtt_unsubscribe_topic(vesync_mqtt_client_t *vesync_client, char *topic)
{
	int ret;
	ret = esp_mqtt_client_unsubscribe(vesync_client->client_handle, topic);
	return ret;
}

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
int vesync_mqtt_publish_rawdata_to_topic(vesync_mqtt_client_t *vesync_client, char* topic, char* data, int datalen, int qos, int retain)
{
	int ret;
	ret = esp_mqtt_client_publish(vesync_client->client_handle, topic, data, datalen, qos, retain);
	return ret;
}
