#ifndef _VESYNC_WIFI_H
#define _VESYNC_WIFI_H

#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "tcpip_adapter.h"
#include "esp_smartconfig.h"

#define CONFIGKEY_BUF_LEN				        16+4	//configKey缓存区大小

#define ERR_CONFIG_NET_SUCCESS 					0 		//配网时设备连接MQTT服务器成功
#define ERR_CONFIG_WIFI_SSID_MISSED				50		//配网数据的“wifiSSID”字段缺失
#define ERR_CONFIG_CONFIGKEY_MISSED 			51		//配网数据的“configKey”字段缺失
#define ERR_CONFIG_NO_AP_FOUND	 				52		//配网时设备找不到WiFi热点（信号差或者路由器不兼容时会出现该错误，设备会尝试重连直到超时）
#define ERR_CONFIG_WRONG_PASSWORD 				53		//配网时设备连接WiFi热点的密码错误
#define ERR_CONFIG_CONNECT_WIFI_FAIL			54		//配网时设备连接WiFi失败（信号差或者路由器不兼容时会出现该错误，设备会尝试重连直到超时）
#define ERR_CONFIG_CID_MISSED 					55		//配网时设备CID缺失（产测不合格但却误出货的设备才会出现，或者联调的设备未模拟产测去获取cid）
#define ERR_CONFIG_SERVER_IP_MISSED				56		//配网数据的“serverIP”字段缺失
#define ERR_CONFIG_URI_ERROR	 				57		//配网数据的uri不合法
#define ERR_CONFIG_PID_DO_NOT_MATCHED			58		//配网时的pid不匹配（用户配网选择的设备类型与实际设备类型不匹配）
#define ERR_CONFIG_NO_APP_LINK					59		//配网时设备没有监听到APP的tcp连接请求（APP端可能为找不到设备IP）
#define ERR_CONFIG_PARSE_JSON_FAIL				60		//配网时解析APP发送的配网数据失败（数据格式不符合json语法）
#define ERR_CONFIG_SMART_CONFIG_RESTART			61		//配网时设备端SMARTCONFIG发生了超时重启，超时时间现设置为240s
#define ERR_CONFIG_LINK_SERVER_FAILED			62		//配网时设备连接MQTT服务器失败
#define ERR_CONFIG_TIMEOUT						63		//配网超时，全程共5分钟

//设备WiFi参数配置
#pragma pack(1)
typedef struct{
	uint8_t wifiSSID[32 + 4];
	uint8_t wifiPassword[64 + 4];
	uint8_t wifiStaticIP[16 + 4];
	uint8_t wifiGateway[16 + 4];
	uint8_t wifiDNS[16 + 4];
}station_config_t;
#pragma pack()			//

//设备MQTT参数配置
#pragma pack(1)
typedef struct{
	uint32_t mqtt_port;
	uint32_t mqtt_keepalive;
	uint32_t security;
	uint8_t  configKey[CONFIGKEY_BUF_LEN];
	uint8_t  serverDN[128 + 4];
	uint8_t  serverIP[16 + 4];
	uint8_t  pid[16 + 4];
	uint32_t ip_link_error_count;
} mqtt_config_t;
#pragma pack()

typedef struct{
    station_config_t station_config;
    mqtt_config_t    mqtt_config;
}device_info_t;
extern device_info_t device_info;

extern const int WIFI_CONNECTED_BIT;
extern const int OTA_BIT;
extern EventGroupHandle_t user_event_group;

void vesync_wifi_init(void);





#endif