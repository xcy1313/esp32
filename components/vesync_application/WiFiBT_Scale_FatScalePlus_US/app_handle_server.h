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

/**
 * @brief 初始化网络功能配置
 */
void app_hadle_server_create(void);

/**
 * @brief 上报产测时的按键测试次数
 * @param times [按键测试的次数]
 */
void app_handle_production_report_button(int times);

/**
 * @brief 上报产测时的充电状态
 * @param charge_status [充电状态，1 - 表示正在充电；0 - 表示未充电]
 * @param power_percent [当前电量百分比数值]
 */
void app_handle_production_report_charge(int charge_status, int power_percent);

/**
 * @brief 上报产测时的称重结果
 * @param weight [实时称重的结果，单位kg]
 * @param imped  [实时称重的阻抗，单位欧姆]
 */
void app_handle_production_report_weight(int weight, int imped);

/**
 * @brief 响应回复服务器蓝牙的rssi值
 * @param rssi [蓝牙信号的rssi值]
 */
void app_handle_production_response_bt_rssi(int rssi);

#endif