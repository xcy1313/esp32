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
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_main.h"
#include "esp_bt.h"
#include "esp_gatt_common_api.h"
#include "esp_blufi_api.h"
#include "blufi_security.h"

#define ADVER_TIME_OUT  0     //广播超时1分钟
#define BLE_MAX_MTU     500

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
    BT_DISCONNTED
}BT_STATUS_T;

/**
 * [uart_recv_cb_t 蓝牙数据接收回调函数指针]
 * @param  char 	[蓝牙链路状态]
 * @param  char* 	[蓝牙数据]
 * @param  int  	[蓝牙接收数据长度]
 * @return       	[无]
 */
typedef void (*bt_recv_cb_t)(const void*,const void*, unsigned short);

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
int32_t vesync_hal_bt_client_init(char *adver_name,uint8_t version,uint8_t product_type,uint8_t product_num,uint8_t *custom,bool enable_blufi,bt_recv_cb_t cb);

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
 * @brief 蓝牙是否连接成功
 * @return true 已连接
 * @return false 已断开
 */
 bool vesync_bt_connected(void);

/**
 * @brief 获取蓝牙当前状态
 * @return BT_STATUS_T 
 */
BT_STATUS_T vesync_get_bt_status(void);

/**
 * @brief 使能notify通知特征发送数据至server端
 * @param ctl 控制码
 * @param cnt 发送包计数
 * @param cmd 命令码
 * @param notify_data 数据载荷
 * @param len 载荷数据长度
 * @return uint32_t 为0发送数据成功
 */
uint32_t vesync_bt_notify(frame_ctrl_t ctl,uint8_t *cnt,uint16_t cmd,const void *notify_data ,unsigned short len);

#endif
