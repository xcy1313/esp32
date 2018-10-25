#ifndef USER_COMMON_H
#define USER_COMMON_H

#include "user_log.h"

//公共头文件，用于放置共用的头文件、宏定义集合及任务事件定义

#define ON						1
#define OFF						0

#define HANDLE_FINISH			0
#define HANDLE_NONE				1

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

//----暂未启用----

#define ERR_CONFIG_TLS_HANDSHAKE_FAIL			70		//配网时设备连接服务器时tls握手失败
#define ERR_CONFIG_CONNECT_SERVER_FAIL			71		//配网时设备连接服务器失败
//---------------

//MQTT已占用任务优先级：2 ！
//设备功能任务优先级及消息类型定义===============================
#define DEVICE_TASK_PRIO        1		//设备功能任务优先级
#define DEVICE_TASK_QUEUE_SIZE  20		//任务队列长度

//消息类型定义
#define DEVICE_INIT_FINISH		0X01	//设备初始化完成
#define MQTT_CONNECTED			0X02	//MQTT连接上服务器
#define FIRST_NET_CONNET		0X03 	//smartconfig后首次连接网络消息，通知开启TCP服务器
#define MQTT_READY				0X04	//MQTT已准备好，即网络已连接且MQTT客户端已初始化
#define CLOSE_TCP_SERVER		0X05	//关闭TCP服务器
#define NETWORK_DISCONNETED		0X06	//网络连接断开
#define NETWORK_CONNETED		0X07	//网络连接成功
#define CONFIG_NETWORK			0X08 	//启动配网
#define MQTT_DISCONNECTED		0X09	//MQTT连接断开
#define MQTT_RECONNECT			0X0A	//MQTT重连事件
#define HANDLE_UART_BUFF		0X0B	//串口接收数据处理消息，解析MCU协议
#define CANCEL_CONFIG_NET		0X0C	//取消配网


//任务声明=======================================================

#endif
