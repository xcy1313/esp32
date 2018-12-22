/**
 * @file vesync_net_service.c
 * @brief 网络基础业务
 * @author WatWu
 * @date 2018-12-05
 */

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "vesync_main.h"
#include "vesync_wifi.h"
#include "vesync_mqtt.h"

#include "vesync_net_service.h"

#include "vesync_ca_cert.h"

static const char* TAG = "mqtt_service";

static vesync_mqtt_client_t s_vesync_client = {0};
static vesync_mqtt_status_e s_vesync_mqtt_status = MQTT_OFFLINE;

/**
 * @brief 连接WiFi的结果回调
 * @param status [WiFi连接状态，包括连接成功和连接掉线，以及连接失败时的错误原因]
 */
static void vesync_connect_wifi_callback(vesync_wifi_status_e status)
{
	switch(status)
	{
		case VESYNC_WIFI_GOT_IP:
			xTaskNotify(event_center_taskhd, NETWORK_CONNECTED, eSetBits);			//通知事件处理中心任务
			break;
		case VESYNC_WIFI_LOST_IP:
			xTaskNotify(event_center_taskhd, NETWORK_DISCONNECTED, eSetBits);		//通知事件处理中心任务
			break;
		default:
			break;
	}
}

/**
 * @brief mqtt事件回调函数
 * @param event 		[mqtt事件]
 * @return esp_err_t 	[错误码]
 */
static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
	// esp_mqtt_client_handle_t client = event->client;
	// your_context_t *context = event->context;
	switch(event->event_id)
	{
		case MQTT_EVENT_CONNECTED:
			LOG_I(TAG, "MQTT_EVENT_CONNECTED");
			s_vesync_mqtt_status = MQTT_ONLINE;
			xTaskNotify(event_center_taskhd, MQTT_CONNECTED, eSetBits);			//通知事件处理中心任务
			break;

		case MQTT_EVENT_DISCONNECTED:
			LOG_I(TAG, "MQTT_EVENT_DISCONNECTED");
			s_vesync_mqtt_status = MQTT_OFFLINE;
			xTaskNotify(event_center_taskhd, MQTT_DISCONNECTED, eSetBits);		//通知事件处理中心任务
			break;

		case MQTT_EVENT_SUBSCRIBED:
			LOG_I(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
			break;

		case MQTT_EVENT_UNSUBSCRIBED:
			LOG_I(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
			break;

		case MQTT_EVENT_PUBLISHED:
			LOG_I(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
			break;

		case MQTT_EVENT_DATA:
			LOG_I(TAG, "MQTT_EVENT_DATA");
			LOG_I(TAG, "TOPIC=%.*s", event->topic_len, event->topic);
			LOG_I(TAG, "DATA=%.*s", event->data_len, event->data);
			break;

		case MQTT_EVENT_ERROR:
			LOG_I(TAG, "MQTT_EVENT_ERROR");
			break;
	}
	return ESP_OK;
}

/**
 * @brief 获取mqtt客户端在线状态
 * @return vesync_mqtt_status_e [mqtt客户端在线状态]
 */
vesync_mqtt_status_e vesync_get_mqtt_client_status(void)
{
	return s_vesync_mqtt_status;
}

/**
 * @brief 获取云平台mqtt客户端指针
 * @return vesync_mqtt_client_t* [vesync云平台mqtt客户端指针]
 */
vesync_mqtt_client_t* vesync_get_cloud_mqtt_client(void)
{
	return &s_vesync_client;
}

/**
 * @brief vesync客户端连接WiFi
 * @param wifi_ssid 	[WiFi账号]
 * @param wifi_password [WiFi密码]
 */
void vesync_client_connect_wifi(char *wifi_ssid, char *wifi_password)
{
	vesync_connect_wifi(wifi_ssid, wifi_password, true);
}

/**
 * @brief 配置vesync云平台的mqtt客户端
 * @param client_id 	[设备cid]
 * @param server_addr 	[mqtt服务器地址]
 * @param server_port 	[mqtt服务器端口]
 * @param username 		[mqtt用户名]
 * @param password 		[mqtt用户密码]
 */
void vesync_config_cloud_mqtt_client(char *client_id, char *server_addr, uint16_t server_port, char *username, char *password)
{
	if(vesync_get_mqtt_client_status() == MQTT_ONLINE)
	{
		vesync_mqtt_client_disconnet_from_cloud();
	}
	if(NULL != s_vesync_client.client_handle)
		vesync_delete_mqtt_client(&s_vesync_client);
	vesync_init_mqtt_client(&s_vesync_client, client_id, server_addr, server_port, username, password, mqtt_event_handler);
}

/**
 * @brief 配置vesync云平台的mqtt客户端
 * @param client_id 	[设备cid]
 * @param server_addr 	[mqtt服务器地址]
 * @param server_port 	[mqtt服务器端口]
 * @param username 		[mqtt用户名]
 * @param password 		[mqtt用户密码]
 */
void vesync_config_cloud_mqtt_client_with_tls(char *client_id, char *server_addr, uint16_t server_port, char *username, char *password)
{
	if(vesync_get_mqtt_client_status() == MQTT_ONLINE)
	{
		vesync_mqtt_client_disconnet_from_cloud();
	}
	if(NULL != s_vesync_client.client_handle)
		vesync_delete_mqtt_client(&s_vesync_client);
	vesync_init_mqtt_client_with_tls(&s_vesync_client, client_id, server_addr, server_port,
	                                 username, password, mqtt_event_handler,
	                                 vesync_ca_cert_pem, vesync_client_cert_pem, vesync_client_key_pem);
}

/**
 * @brief vesync平台mqtt客户端连接与服务器
 */
void vesync_mqtt_client_connect_to_cloud(void)
{
	vesync_mqtt_connect(&s_vesync_client);
}

/**
 * @brief vesync平台mqtt客户端断开与服务器的连接
 */
void vesync_mqtt_client_disconnet_from_cloud(void)
{
	vesync_mqtt_disconnect(&s_vesync_client);
}

void vesync_clinet_wifi_module_init(void)
{
	vesync_init_wifi_module(vesync_connect_wifi_callback);
}