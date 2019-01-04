/**
 * @file app_handle_phone.h
 * @brief 蓝牙接收事件功能
 * @author Jiven 
 * @date 2018-12-18
 */
#ifndef _APP_HANLDE_PHONE_H
#define _APP_HANLDE_PHONE_H

#include "etekcity_bt_prase.h"
#include "etekcity_comm.h"
#include <stdint.h>

#define PRODUCT_TEST_TYPE 0x88
#define PRODUCT_TEST_NUM  0x88

#define PRODUCT_VER		0x1
#define PRODUCT_TYPE    0xC0
#define PRODUCT_NUM		0xA0
#define PRODUCT_NAME	"ESF00+"

#define DEVICE_TYPE     "1"
#define FIRM_VERSION    "2"


#define APP_ADVERTISE_TIMEOUT   0
/**
 * @brief 定义与app交互命令集
 */
#define PROJECT_FOR_SCALES_BASE_ADDR	0x0000
#define CMD_REPORT_VESION       (PROJECT_FOR_SCALES_BASE_ADDR+0x0001)	
#define CMD_RESP_VERSION        (PROJECT_FOR_SCALES_BASE_ADDR+0x0002)
#define CMD_REPORT_CODING       (PROJECT_FOR_SCALES_BASE_ADDR+0x0003)
#define CMD_RESP_CODING         (PROJECT_FOR_SCALES_BASE_ADDR+0x0004)
#define CMD_REPORT_WEIGHT       (PROJECT_FOR_SCALES_BASE_ADDR+0x0005)
#define CMD_REPORT_ERRPR        (PROJECT_FOR_SCALES_BASE_ADDR+0x0006)
#define CMD_SET_BODY_FAT        (PROJECT_FOR_SCALES_BASE_ADDR+0x0008)
#define CMD_REPORT_POWER        (PROJECT_FOR_SCALES_BASE_ADDR+0x0009)
#define CMD_RESP_POWER          (PROJECT_FOR_SCALES_BASE_ADDR+0x000A)
#define CMD_SET_BT_STATUS       (PROJECT_FOR_SCALES_BASE_ADDR+0x000B)
#define CMD_SET_WIFI_STATUS     (PROJECT_FOR_SCALES_BASE_ADDR+0x000C)
#define CMD_SET_HW_UNIT    	    (PROJECT_FOR_SCALES_BASE_ADDR+0x000D)
#define CMD_SET_WEIGHT_UNIT		(PROJECT_FOR_SCALES_BASE_ADDR+0x001F)
#define CMD_SYNC_TIME			(PROJECT_FOR_SCALES_BASE_ADDR+0x0020)
#define CMD_CONFIG_ACCOUNT		(PROJECT_FOR_SCALES_BASE_ADDR+0x0021)
#define CMD_DELETE_ACCOUNT		(PROJECT_FOR_SCALES_BASE_ADDR+0x0022)
#define CMD_INQUIRY_HISTORY		(PROJECT_FOR_SCALES_BASE_ADDR+0x0023)
#define CMD_SET_FAT_CONFIG		(PROJECT_FOR_SCALES_BASE_ADDR+0x0024)
#define CMD_UPGRADE				(PROJECT_FOR_SCALES_BASE_ADDR+0x0025)
#define CMD_FACTORY_GET_BT_RSSI	(PROJECT_FOR_SCALES_BASE_ADDR+0x0026)

/**
 * @brief 
 */
void app_ble_init(void);

#endif

