/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#ifndef _VESYNC_BT_H
#define _VESYNC_BT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"

#include "etekcity_comm.h"

#define BLE_MAX_MTU     500
/* Attributes State Machine */
enum
{
    IDX_SVC,
    IDX_CHAR_A,
    IDX_CHAR_VAL_A,
    IDX_CHAR_CFG_A,

    IDX_CHAR_B,
    IDX_CHAR_VAL_B,

    IDX_CHAR_C,
    IDX_CHAR_VAL_C,

    HRS_IDX_NB,
};
#endif

/**
 * [uart_recv_cb_t 蓝牙数据接收回调函数指针]
 * @param  char* 	[蓝牙数据]
 * @param  int  	[蓝牙接收数据长度]
 * @return       	[无]
 */
typedef void (*bt_recv_cb_t)(const void*, unsigned short);

typedef struct{
    bt_recv_cb_t        m_bt_handler;
}BTSTRUCT;

void vesync_bt_init(bt_recv_cb_t cb);
void vesync_bt_notify(void *notify_data ,unsigned short len);
void vesync_bt_gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
