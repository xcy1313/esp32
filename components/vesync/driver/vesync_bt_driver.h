/**
 * @file vesync_bt_driver.h
 * @brief 蓝牙接口驱动
 * @author Jiven 
 * @date 2018-12-14
 */
#ifndef _VESYNC_BT_DRIVER_H
#define _VESYNC_BT_DRIVER_H

#include <stdint.h>
#include <stdbool.h>

#include "etekcity_bt_prase.h"

#define FIRM_VERSION	"2.1.8"



/**
 * @brief true使能蓝牙配网功能
 * @param
 * @param version bit[7 : 4]:保留 ; bit[3 : 0]:数据格式版本号，当前版本为1；即当前该字节为：0x01
 * @param product_type 产品类型
 * @param product_num  产品编码
 * @param custom       自定义数据流
 * @param enable_blufi 是否使能蓝牙配网
 */
void vesync_init_bt_module(char *adver_name,uint8_t version,uint8_t product_type,uint8_t product_num,uint8_t *custom,bool enable_blufi);



int vesync_notify_send(frame_ctrl_t ctl,uint8_t *cnt,uint16_t cmd,const void *notify_data ,unsigned short len);




#endif
