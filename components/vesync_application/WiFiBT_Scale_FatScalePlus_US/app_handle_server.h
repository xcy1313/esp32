/**
 * @file app_handle_server.h
 * @brief 处理和网络对接相关
 * @author Jiven 
 * @date 2018-12-29
 */
#ifndef _APP_HANDLE_SERVCER_H
#define _APP_HANDLE_SERVCER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "vesync_device.h"

#define PRODUCT_TEST_START  0x1

#define USER_STORE_LEN		50

#define UPGRADE_NULL			0
#define UPGRADE_APP				1
#define UPGRADE_PRODUCTION	    2

//vesync事件处理中心任务的事件标志位定义，高8位用于存放自定义参数，即事件位共24位
#define STORE_WEIGHT_DATA_REQ			0x00000002
#define UPLOAD_WEIGHT_DATA_REQ			0x00000004
#define UPLOAD_ALL_USER_DATA_REQ		0x00000008

/**
 * @brief 设备配网状态
 */
typedef enum
{
	NET_CONFNET_NOT_CON = 0,			//未配网
	NET_CONFNET_ONLINE 	= 1,			//已连接上服务器
	NET_CONFNET_OFFLINE = 2,			//已配网但未连接上服务器
}device_net_status_t;

typedef enum{
	FACTORY_TEST_SYNC_BUTTON_BIT = 0x0001,
	FACTORY_TEST_SYNC_CHARGE_BIT = 0x0002,
	FACTORY_TEST_SYNC_WEIGHT_BIT = 0x0004,
	FACTORY_TEST_SYNC_BT_BIT 	 = 0x0008
}factory_test_bit_t;
extern factory_test_bit_t factory_test_bit;
/**
 * @brief 返回trace_id
 * @param time 
 */
char *vesync_get_time(void);

/**
 * @brief 主动发起升级结果
 * @param result 
 * @param trace_id 
 */
void app_handle_production_upgrade_response_result(char *trace_id,uint8_t result);

/**
 * @brief 应答服务器
 */
void app_handle_production_response_ack(char *trace_id,uint8_t test_item);

/**
 * @brief 初始化网络功能配置
 */
void app_hadle_server_create(void);

/**
 * @brief 上报产测时的按键测试次数
 * @param times [按键测试的次数]
 */
void app_handle_production_report_button(char *trace_id,int times);

/**
 * @brief 上报产测时的充电状态
 * @param charge_status [充电状态，1 - 表示正在充电；0 - 表示未充电]
 * @param power_percent [当前电量百分比数值]
 */
void app_handle_production_report_charge(char *trace_id,int charge_status, int power_percent);

/**
 * @brief 上报产测时的称重结果
 * @param weight [实时称重的结果，单位kg]
 * @param imped  [实时称重的阻抗，单位欧姆]
 */
void app_handle_production_report_weight(char *trace_id,int weight, int imped);

/**
 * @brief 响应回复服务器蓝牙的rssi值
 * @param rssi [蓝牙信号的rssi值]
 */
void app_handle_production_response_bt_rssi(char *trace_id,int rssi);

/**
 * @brief 注册回调接收Mqtt数据buff
 * @param data 
 */
void vesync_recv_json_data(char *data);

/**
 * @brief 获取当前配网状态
 */
device_net_status_t app_handle_get_net_status(void);

/**
 * @brief 设置配网新状态
 * @param new_status 
 */
void app_handle_set_net_status(device_net_status_t new_status);

/**
 * @brief 应用层网络服务通知
 * @param bit 
 */
void app_handle_net_service_task_notify_bit(uint32_t bit ,void *data,uint16_t len);

/**
 * @brief 设置升级来源
 * @param source 
 */
void app_set_upgrade_source(uint8_t source);

/**
 * @brief 获取升级来源
 */
uint8_t app_get_upgrade_source(void);

void device_status(device_status_e status);

void router_status(vesync_router_link_status_t status);

#endif