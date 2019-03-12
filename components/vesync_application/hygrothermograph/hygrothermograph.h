/**
 * @file hygrothermograph.h
 * @brief 温湿度计设备功能接口
 * @author WatWu
 * @date 2019-03-11
 */

#ifndef HYGROTHERMOGRAPH_H
#define HYGROTHERMOGRAPH_H

#include "vesync_api.h"

//蓝牙通信命令定义
#define REPORT_TEMP_HUMI                        0x0301      //设备上报温湿度值
#define QUERY_HISTORY                           0x0302      //查询温湿度历史记录
#define CLEAR_HISTORY                           0x0303      //清空温湿度历史记录
#define QUERY_PREWARNING                        0x0304      //查询温湿度预警值
#define SET_PREWARNING                          0x0305      //设置温湿度预警值

#define BLE_CONNECTED                           1
#define BLE_DISCONNECTED                        0

#define HISTORY_LIST_MAX                        (2*24*7)    //历史记录最大值，每半小时一次，最大保存7天

#define HUMITURE_FLASH_LABEL                    "userdata"  //温湿度计的的flash保存label
#define HUMITURE_HISTORY_KEY                    "humiture_his"  //温湿度计的历史记录

typedef struct                                              //温湿度数据结构，含时间戳
{
    int16_t temp;
    int16_t humi;
    uint32_t timestamp;
} temp_humi_data_t;

typedef struct
{
    uint32_t history_amount;                                //当前已保存的历史记录总数
    temp_humi_data_t history_list[HISTORY_LIST_MAX];        //历史记录列表
}temp_humi_history_t;

typedef struct
{
    uint16_t temp_warn_upper_limit;
    uint16_t temp_warn_lower_limit;
    uint8_t  temp_warn_switch;
    uint16_t humi_warn_upper_limit;
    uint16_t humi_warn_lower_limit;
    uint8_t  humi_warn_switch;
}hygrother_warning_t;

/**
 * @brief 更新温湿度值到APP，通过ble蓝牙
 */
void update_temp_humi_to_app(void);

/**
 * @brief 蓝牙数据接收回调函数
 * @param data  [接收的蓝牙数据]
 * @param len   [数据长度]
 */
void ble_recv_data_callback(const unsigned char *data, unsigned char len);

/**
 * @brief 蓝牙状态回调
 * @param bt_status [当前实时蓝牙状态]
 */
void ble_status_callback(BT_STATUS_T bt_status);

/**
 * @brief 温湿度计设备初始化
 */
void hygrothermograph_init(void);

#endif
