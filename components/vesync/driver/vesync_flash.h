/**
 * @file vesync_flash.h
 * @brief vesync设备flash读写驱动
 * @author WatWu
 * @date 2018-11-20
 */

#ifndef VESYNC_FLASH_H
#define VESYNC_FLASH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "vesync_bt_hal.h"

#define INFO_NAMESPACE "vesync"
#define INFO_pid_KEY "vesync_pids"
#define INFO_config_KEY  "vesync_config"
#define INFO_DN_KEY  "vesync_server"
#define INFO_IP_KEY  "vesync_ip"
#define INFO_SSID_KEY  "vesync_ssid"
#define INFO_PWD_KEY  "vesync_pwd"
#define INFO_static_IP_KEY  "vesync_ip"
#define INFO_gateWay_KEY  "vesync_gate_way"
#define INFO_DNS_KEY "vesync_dns"

/**
 * @brief 擦除当前label_name区域对应的key_name存储内容
 * @param label_name 
 * @param key_name 
 */
void vesync_flash_erase(const char *label_name,const char *key_name);

/**
 * @brief 写入当前所在key_name的存储内容
 * @param label_name partition区名
 * @param key_name 键值名
 * @param data 数据写入指针
 * @param len 数据长度
 * @return true 写入结果成功
 * @return false 写入结果失败
 */
bool vesync_flash_write(const char *label_name,const char *key_name,const void *data,uint32_t len);

/**
 * @brief 读取当前所在key_name的存储内容
 * @param label_name partition区名
 * @param key_name 键值名
 * @param data 数据读取指针
 * @param len 读取长度
 */
void vesync_flash_read(const char *label_name,const char *key_name,const void *data,uint16_t *len);

/**
 * @brief 配置自定义partition区，操作前必须对此enable
 */
void vesync_flash_config(bool enable ,const char *part_name);

/**
 * @brief 
 * @param value 
 * @return uint32_t 
 */
uint32_t vesync_flash_write_i8(const char *label_name,const char *key_name,uint8_t value);

/**
 * @brief 
 * @param value 
 * @return uint32_t 
 */
uint32_t vesync_flash_read_i8(const char *label_name,const char *key_name,uint8_t *value);

/**
 * @brief 写入配网参数信息
 * @param info 
 * @return uint32_t 
 */
uint32_t vesync_flash_write_net_info(net_info_t *info);

/**
 * @brief 读取配网参数信息
 * @param x_info 
 * @return true 
 * @return false 
 */
bool vesync_flash_read_net_info(net_info_t *x_info);

#endif
