/**
 * @file vesync_net_service.h
 * @brief 网络基础业务
 * @author WatWu
 * @date 2018-12-05
 */

#ifndef VESYNC_NET_SERVICE_H
#define VESYNC_NET_SERVICE_H

#include "vesync_mqtt.h"
#include "cJSON.h"

// mqtt 主题定义
#define DEVICE_REQUEST_TOPIC            "mqtt/%s/v1/req"
#define DEVICE_RESPONSE_TOPIC           "mqtt/%s/v1/rsp"
#define CLOUD_BYPASS_TOPIC              "mqtt/%s/v1/bypass"
#define CLOUD_BYPASS_RSP_TOPIC          "mqtt/%s/v1/bypass/rsp"

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

/**
 * @brief 用户调用初始化wifi模块
 */
void vesync_clinet_wifi_module_init(bool power_save);

/**
 * @brief 设置https服务器地址
 * @param address 	[服务器域名或者IP地址]
 * @return int 		[设置结果，0为成功]
 */
int vesync_set_https_server_address(char *address);

/**
 * @brief vesync平台https客户端发起请求
 * @param method 		[请求的接口名]
 * @param body 			[请求接口的数据内容]
 * @param recv_buff 	[返回的数据内容缓存buffer]
 * @param recv_len 		[返回的数据内容长度指针，传入时为缓存buffer的长度，供内部判断buffer大小是否足够，足够时内部把返回的数据拷贝至buffer，并赋值该值为数据长度]
 * @param wait_time_ms 	[超时等待时间]
 * @return int 			[请求结果]
 */
int vesync_https_client_request(char *method, char *body, char *recv_buff, int *recv_len, int wait_time_ms);

/**
 * @brief 为cjson格式的协议方法添加固定的接口头部
 * @param method 	[原样返回时间]
 * @param method 	[接口方法名]
 * @param body 		[方法的数据体]
 * @return cJSON* 	[添加了固定头部后的cjson格式数据，使用完后必须调用cJSON_Delete进行删除！！！]
 */
cJSON* vesync_json_add_method_head(char *trace_id,char *method,cJSON *body);

#endif
