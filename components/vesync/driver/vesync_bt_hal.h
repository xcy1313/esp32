/**
 * @file vesync_bt_hal.h
 * @brief 蓝牙hal配置ble服务属性接口
 * @author Jiven 
 * @date 2018-12-14
 */
#ifndef _VESYNC_BT_HAL_H
#define _VESYNC_BT_HAL_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "etekcity_bt_prase.h"

#define CID_LENGTH						32				//cid闀垮害

#define NET_CONFIG_TIME_OUT 20*1000
#define ADVER_TIME_OUT  10000     //广播超时1分钟
#define BLE_MAX_MTU     200

#define QUEUE_LEN                   100

// #define DEVICE_TYPE     "1"
// #define FIRM_VERSION    "2"

#define vesync_blufi_rev_custom_data_evt    0x1
#define vesync_ble_rev_data_evt      0x2


#define ERR_CONFIG_NET_SUCCESS 					0 		//配网时设备连接MQTT服务器成功
#define ERR_CONFIG_CMD_SUCCESS                  1       //收到命令应答
#define ERR_CONFIG_WIFI_SSID_MISSED				50		//路由器ssid出错
#define ERR_CONFIG_CONFIGKEY_MISSED 			51		//mqtt configkey出错
#define ERR_CONFIG_NO_AP_FOUND	 				52		//找不到热点
#define ERR_CONFIG_WRONG_PASSWORD 				53		//路由器密码出错
#define ERR_CONFIG_CONNECT_WIFI_FAIL			54		//连接路由器失败
#define ERR_CONFIG_CID_MISSED 					55		//MQTT CID缺失
#define ERR_CONFIG_SERVER_IP_MISSED				56		//MQTT服务器IP缺失
#define ERR_CONFIG_URI_ERROR	 				57		
#define ERR_CONFIG_PID_DO_NOT_MATCHED			58	    //pid与产测不匹配	
#define ERR_CONFIG_NO_APP_LINK					59		
#define ERR_CONFIG_PARSE_JSON_FAIL				60		//json解析出错
#define ERR_CONFIG_SMART_CONFIG_RESTART			61		//配网时设备端SMARTCONFIG发生了超时重启，超时时间现设置为240s
#define ERR_CONFIG_LINK_SERVER_FAILED			62		//配网时设备连接MQTT服务器失败
#define ERR_CONFIG_TIMEOUT						63		//配网超时，全程共5分钟
#define ERR_CONFIG_WIFI_DEIVER_INIT				64		//wifi驱动未初始化

#define ERR_CONFIG_SERVER_URL_MISSED            80      //https uri地址出错
#define ERR_CONFIG_ACCOUNT_ID_MISSED            81      //账户信息出错
#define ERR_WIFI_LINK_BUSY                      82      //wifi链路忙，不能进入扫描模式
#define ERR_NO_ALLOCATION_CID                   83      //未分配cid
#define ERR_CONNECT_MQTT_SERVER_FAIL            84      //连接MQTT服务器失败
#define ERR_CONNECT_HTTPS_SERVER_FAIL           85     //连接https服务器失败

#define ERR_TOTAL                               100     

/* Attributes State Machine */
enum{
    IDX_SVC,
    IDX_CHAR_A,
    IDX_CHAR_VAL_A,
    IDX_CHAR_CFG_A,

    IDX_CHAR_B,
    IDX_CHAR_VAL_B,

    HRS_IDX_NB,
};

/// device info Attributes Indexes
enum{
    IDX_DEV_SVC,

    IDX_DEV_FW_VERSION_CHAR,
    IDX_DEV_FW_VERSION_VAL,

    IDX_DEV_HW_VERSION_CHAR,
    IDX_DEV_HW_VERSION_VAL,

    IDX_DEV_SW_VERSION_CHAR,
    IDX_DEV_SW_VERSION_VAL,

    DEV_DEV_NB,
};

/**
 * @brief 蓝牙状态
 */
typedef enum{
    BT_INIT = -1,
    BT_CREATE_SERVICE,
    BT_ADVERTISE_START,
    BT_ADVERTISE_STOP,
    BT_CONNTED,
    BT_DISCONNTED,
    BT_CONFIG_NET_START,
    BT_CONFIG_NET_TIMEOUT,
    BT_CONFIG_NET_SUCCEED,
    BT_CONFIG_NET_CANCEL,
}BT_STATUS_T;

//设备WiFi参数配置
#pragma pack(1)
typedef struct{
    uint8_t server_url[64+ 4];
    uint8_t account_id[4+ 4];
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
    uint8_t  pid[16 + 4];
	uint8_t  configKey[20];
	uint8_t  serverDN[64 + 4];
	uint8_t  serverIP[16 + 4];
} mqtt_config_t;
#pragma pack()

//设备产测配置
#pragma pack(1)
typedef struct{
	uint8_t  cid[32 + 4];
}product_config_t;
#pragma pack()
extern product_config_t product_config;

//设备配网参数配置
typedef struct{
    mqtt_config_t    mqtt_config;
    station_config_t station_config;
}net_info_t;
extern net_info_t net_info;

typedef void (*vesync_bt_status_cb_t)(BT_STATUS_T bt_status);

/**
 * [uart_recv_cb_t 蓝牙数据接收回调函数指针]
 * @param  char 	[蓝牙链路状态]
 * @param  char* 	[蓝牙数据]
 * @param  int  	[蓝牙接收数据长度]
 * @return       	[无]
 */
typedef void (*bt_recv_cb_t)(const unsigned char*, unsigned char);

/**
 * @brief 关闭蓝牙协议栈
 */
void vesync_hal_bt_client_deinit(void);

/**
 * @brief 初始化蓝牙协议栈功能
 * @param adver_name   广播名称
 * @param product_type 版本号
 * @param product_type 产品类型
 * @param product_num  产品编码
 * @param custom       自定义数据流
 * @param enable_blufi 是否使能蓝牙配网
 * @param cb           蓝牙接收数据回调
 * @return int32_t 
 */
int32_t vesync_bt_client_init(char *adver_name,uint8_t version,uint8_t product_type,uint8_t product_num,uint8_t *custom,bool enable_blufi,
                                    vesync_bt_status_cb_t bt_status_cb, bt_recv_cb_t cb);

/**
 * @brief 开启蓝牙广播，timeout为0表示一直广播
 * @param timeout 
 */
void vesync_bt_advertise_start(uint32_t timeout);
/**
 * @brief 停止蓝牙广播
 */
void vesync_bt_advertise_stop(void);
/**
 * @brief 使能notify通知特征发送数据至server端
 * @param ctl 控制码
 * @param cnt 发送包计数
 * @param cmd 命令码
 * @param notify_data 数据载荷
 * @param len 载荷数据长度
 * @return uint32_t 为0发送数据成功
 */
uint32_t vesync_bt_notify(frame_ctrl_t ctl,uint8_t *cnt,uint16_t cmd,const unsigned char *notify_data ,unsigned short len);

/**
 * @brief 蓝牙是否连接成功
 * @return true 
 * @return false 
 */
bool vesync_bt_connected(void);
/**
 * @brief 蓝牙配网发送自定义数据流
 * @param data 
 * @param data_len 
 * @return uint32_t 
 */
uint32_t vesync_blufi_notify(uint8_t *data, uint32_t data_len);

/**
 * @brief 动态修改广播名称
 * @param name 
 * @return uint32_t 
 */
uint32_t vesync_bt_dynamic_set_ble_advertise_name(char *name);

/**
 * @brief 动态修改广播参数
 * @param product_type 
 * @param product_num 
 * @return uint32_t 
 */
uint32_t vesync_bt_dynamic_ble_advertise_para(uint8_t product_type,uint8_t product_num);

#endif
