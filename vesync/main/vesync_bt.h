
#ifndef _VESYNC_BT_H
#define _VESYNC_BT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "etekcity_bt_prase.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"

#include "etekcity_comm.h"

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

void vesync_bt_deinit(void);
void vesync_bt_init(bt_recv_cb_t cb);
uint32_t vesync_bt_notify(frame_ctrl_t ctl,uint8_t *cnt,uint16_t cmd,const void *notify_data ,unsigned short len);
void vesync_bt_gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);


void vesync_bt_advertise_start(uint32_t timeout);
void vesync_bt_advertise_stop(void);

bool vesync_bt_connected(void);
BT_STATUS_T vesync_get_bt_status(void);


#endif
