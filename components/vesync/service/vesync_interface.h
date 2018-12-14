/**
 * @file vesync_interface.h
 * @brief vesync设备的独立业务接口
 * @author WatWu
 * @date 2018-11-21
 */

#ifndef VESYNC_INTERFACE_H
#define VESYNC_INTERFACE_H

#define VESYNC_VERSION					"1.0.0"		//标准vesync固件版本号定义

#include "cJSON.h"

/**
 * @brief 网络连接成功回调函数指针，此处指基线SDK内部的WiFi连接成功，已被分配了IP地址。
 */
typedef void (*network_connected_cb_t)(void);

/**
 * @brief 注册vesync网络连接成功回调函数，可以多次注册多个，在网络连接成功后会逐个调用，在需要网络连接成功后才能继续执行的应用可用该函数注册
 * @param cb   [网络连接成功回调函数，在网络连接成功后会被调用]
 * @return int [注册结果，0 - 注册失败；其他数值则是已经成功注册的回调函数数量]
 */
int vesync_regist_networkconnected_cb(network_connected_cb_t cb);

/**
 * [connected_server_cb_t 连接服务器成功回调函数指针]
 * @return  [无]
 */
typedef void (*connected_server_cb_t)(void);

/**
 * [vesync_regist_connectedserver_cb 注册连接服务器成功后的回调函数]
 * @param  cb [连接服务器成功回调函数，该函数由应用层定义，每次成功连接上服务器后会调用该函数，可用于上报设备上电数据信息]
 * @return    [无]
 */
void vesync_regist_connectedserver_cb(connected_server_cb_t cb);

/**
 * [json_recv_cb_t 接收到服务器发送的json数据回调函数指针]
 * @param  char*	 [json原始数据]
 * @return           [无]
 */
typedef void (*json_recv_cb_t)(char*);

/**
 * [vesync_regist_recvjson_cb 注册json接收回调函数]
 * @param  cb [json接收回调函数，该函数由应用层定义，mqtt收到json数据时会调用该函数]
 * @return    [无]
 */
void vesync_regist_recvjson_cb(json_recv_cb_t cb);

/**
 * [restore_device_cb_t 复位设备回调函数指针]
 * @return  [无]
 */
typedef void (*restore_device_cb_t)(void);

/**
 * [vesync_regist_restoredevice_cb 注册复位设备的回调函数]
 * @param  cb [复位设备回调函数，该函数由应用层定义，在APP端进行复位设备且设备响应后会调用该函数]
 * @return    [无]
 */
void vesync_regist_restoredevice_cb(restore_device_cb_t cb);

/**
 * [vesync_get_vesync_sdk_version 获取VeSync版本号]
 * @return  [版本号字符串]
 */
const char* vesync_get_vesync_sdk_version(void);

/**
 * @brief 上报客户端固件版本信息
 * @param devName		[设备名称]
 * @param devVersion 	[设备版本]
 * @param firmName 		[设备固件名称]
 * @param firmVersion 	[设备固件版本]
 */
void vesync_report_client_firmversion(const char *devName, const char *devVersion, const char *firmName, const char *firmVersion);

/**
 * @brief 直接按照标准缩进格式打印cjson数据，内部实现转换
 * @param json [cjson格式的数据]
 */
void vesync_printf_cjson(cJSON *json);

#endif
