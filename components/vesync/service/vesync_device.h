/**
 * @file vesync_device.h
 * @brief 设备初始化、配置属性及操作等，描述内部SDK对象
 * @author WatWu
 * @date 2018-11-20
 */

#ifndef VESYNC_DEVICE_H
#define VESYNC_DEVICE_H

#include <stdint.h>

#define CID_LENGTH						32				//cid长度
#define CID_HOLDER 						0XCCDD			//CID保存标志
#define DEVCONF_HOLDER					0XA5A5			//设备配置初始化保存标志
#define CONFIGKEY_BUF_LEN				16+4			//configKey缓存区大小
#define IP_DIRECT_LINK_TRY_TIMES		3				//IP直连尝试次数

#define NVDM_GROUP_VESYNC				"vesync"
#define NVDM_ITEM_DEVICE_CONFIG			"device_config"
#define NVDM_ITEM_DEVICE_CID			"device_cid"

/**
 * @brief 设备配网状态
 */
typedef enum
{
	DEV_CONFNET_NOT_CON = 0,			//未配网
	DEV_CONFNET_ONLINE 	= 1,			//已连接上服务器
	DEV_CONFNET_OFFLINE = 2,			//已配网但未连接上服务器
} device_status_e;

/**
 * @brief TCP调试端口开关状态
 */
typedef enum
{
	TCP_DEBUG_OFF,
	TCP_DEBUG_ON
} tcp_debug_status_e;

/**
 * @brief vesync设备wifi参数配置
 */
typedef struct
{
	char ssid[32 + 4];
	char password[64 + 4];
	char staticip[16 + 4];
	char gateway[16 + 4];
	char dns[16 + 4];
} vesync_wifi_conf_t;

/**
 * @brief vesync设备mqtt参数配置
 */
typedef struct
{
	uint32_t server_port;
	uint32_t keepalive;
	uint32_t security;
	char configkey[CONFIGKEY_BUF_LEN];
	char server_dn[128 + 4];
	char server_ip[16 + 4];
	char pid[16 + 4];
	uint32_t ip_link_error_count;
} vesync_mqtt_conf_t;

/**
 * @brief vesync设备产品相关参数配置
 */
typedef struct
{
	//CID记录标志，等于CID_HOLDER则代表已保存CID，永久使用
	uint32_t cid_holder;

	char cid[32 + 4];
	char authkey[32 + 4];
	char pid[16 + 4];
} vesync_product_conf_t;

/**
 * @brief vesync设备整体配置
 */
typedef struct
{
	//数据初始化标志，等于DEVCONF_HOLDER则代表已初始化保存过数据
	uint32_t sta_holder;

	//WIFI相关
	vesync_wifi_conf_t wifi_conf;

	//MQTT相关
	vesync_mqtt_conf_t mqtt_conf;

	//tcp调试端口开关
	tcp_debug_status_e tcp_debug;
} vesync_device_conf_t;


/**
 * [device_status_cb_t 配网状态回调函数指针]
 * @param  device_status_e [配网状态]
 * @return [无]
 */
typedef void (*device_status_cb_t)(device_status_e);

/**
 * [vesync_get_device_status 获取设备状态]
 * @return  [设备配网状态状态，详细定义见DeviceStatus_t]
 */
device_status_e vesync_get_device_status(void);
/**
 * [vesync_set_device_status 设置设备状态]
 * @param  status [设备状态]
 * @return        [无]
 */
void vesync_set_device_status(uint8_t status);

/**
 * [vesync_regist_devstatus_cb 注册实时报告设备配网状态变化的回调函数]
 * @param  cb [配网状态回调函数，该函数由应用层定义，设备配网状态每次的改变都会调用该函数]
 * @return    [无]
 */
void vesync_regist_devstatus_cb(device_status_cb_t cb);

/**
 * [vesync_reset_device_conf 设备配置清空复位]
 * @param  dev_conf [配置信息结构体指针]
 * @return         [无]
 */
void vesync_reset_device_conf(vesync_device_conf_t* dev_conf);

/**
 * @brief 获取设备CID。CID由设备产测通过的最后一个步骤写入FLASH，永久使用
 * @param cid_buf 	[需要获取CID的数据保存区]
 * @return int 		[SUCCESS - 成功读取到CID；FAIL - CID不存在]
 */
int vesync_get_device_cid(char *cid_buf);

/**
 * @brief 获取设备pid。pid由应用层通过宏定义传入，标记不同产品
 * @param pid_buf 	[需要获取pid的数据保存区]
 */
void vesync_get_device_pid(char *pid_buf);

/**
 * @brief 获取设备AuthKey。AuthKey由应用层通过宏定义传入，标记不同厂商
 * @param authkey_buf 	[需要获取AuthKey的数据保存区]
 */
void vesync_get_device_authkey(char *authkey_buf);

/**
 * @brief 设置设备产品信息，把设备产测通过后保存在FLASH里的信息读取到产品信息全局变量
 * @param product_conf 	[设备产品信息结构体]
 * @return 				[无]
 */
void vesync_get_product_config(vesync_product_conf_t *product_conf);

#endif
