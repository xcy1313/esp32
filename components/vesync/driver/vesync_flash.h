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

#include "nvs.h"
#include "vesync_bt_hal.h"
#include "esp_err.h"
#include "esp_system.h"
#include "nvs_flash.h"

#define CONFIG_NAMESPACE "product"
#define CONFIG_CID_HOLDER_KEY "config_holder"
#define CONFIG_CID_CID_KEY "config_cid"
#define CONFIG_CID_AUTH_KEY "config_auth"
#define CONFIG_CID_PID_KEY "config_pid"

#define CONFIG_TOKEN_NAMESPACE "token"
#define CONFIG_TOKEN_KEY    "https_token"

#define INFO_NAMESPACE "vesync"
#define INFO_pid_KEY "vesync_pids"
#define INFO_config_KEY  "vesync_config"
#define INFO_DN_KEY  "vesync_server"
#define INFO_IP_KEY  "vesync_ip"
#define INFO_URL_KEY  "vesync_url"
#define INFO_ACCOUNT_KEY  "vesync_account"
#define INFO_SSID_KEY  "vesync_ssid"
#define INFO_PWD_KEY  "vesync_pwd"
#define INFO_static_IP_KEY  "vesync_ip"
#define INFO_gateWay_KEY  "vesync_gate_way"
#define INFO_DNS_KEY "vesync_dns"

/**
 * @brief 
 * @param part_name 
 * @return uint32_t 
 */
uint32_t vesync_flash_erase_partiton(const char *part_name);
/**
 * @brief 使用给定的名称擦除键值对
 * @param label_name 
 * @param key_name 
 * @return uint32_t 
 */
uint32_t vesync_flash_erase_key(const char *label_name,const char *key_name);

/**
 * @brief 擦除当前label_name区域对应的所有key_name存储内容
 * @param label_name 
 * @param key_name 
 */
uint32_t vesync_flash_erase_all_key(const char *label_name,const char *key_name);

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
 * @param label_name 
 * @param key_name 
 * @param data 
 * @param len 
 * @return uint32_t 返回操作错误码
 */
uint32_t vesync_flash_read(const char *label_name,const char *key_name,const void *data,uint16_t *len);

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

/**
 * @brief 写入配网参数信息
 * @param info 
 * @return uint32_t 
 */
uint32_t vesync_flash_write_product_config(product_config_t *info);

/**
 * @brief 读取配网参数信息
 * @param x_info 
 * @return true 
 * @return false 
 */
bool vesync_flash_read_product_config(product_config_t *x_info);

uint32_t vesync_flash_write_token_config(char *token);

uint32_t vesync_flash_read_token_config(char *token);

int32_t vesync_flash_erase_net_info(void);

#endif
