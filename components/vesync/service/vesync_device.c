/**
 * @file vesync_device.h
 * @brief 设备初始化、配置属性及操作等，描述内部SDK对象
 * @author WatWu
 * @date 2018-11-20
 */

#include "vesync_device.h"
#include "vesync_wifi.h"
#include "vesync_build_cfg.h"
#include "vesync_main.h"
#include "vesync_tcp_server.h"
#include "vesync_wifi_led.h"
#include "vesync_net_service.h"

#include "freertos/timers.h"

// static const char* TAG = "vesync_device";

// static vesync_device_conf_t s_device_config;					//设备配置信息
// static vesync_product_conf_t s_product_config;				//设备产品信息
static device_status_e device_status = DEV_CONFIG_NET_NULL;		//设备配网状态，默认为离线状态
static vesync_router_link_status_t router_status = DEV_ROUTER_LINK_INIT;		//设备配网状态，默认为离线状态

static device_status_cb_t dev_status_callback = NULL;			//定义配网状态回调函数指针
static vesync_router_link_status_cb_t router_link_status_callback = NULL;

/**
 * [vesync_get_device_status 获取设备状态]
 * @return  [设备配网状态状态，详细定义见DeviceStatus_t]
 */
device_status_e vesync_get_device_status(void)
{
	LOG_I(TAG, "vesync_get_device_status : %d.",device_status);
	return device_status;
}

/**
 * [vesync_set_device_status 设置设备状态]
 * @param  status [设备状态]
 * @return        [无]
 */
void vesync_set_device_status(uint8_t status)
{
	if(device_status != status){
		device_status = status;
		if(dev_status_callback != NULL)
			dev_status_callback(device_status);
	}
}

/**
 * [vesync_regist_devstatus_cb 注册实时报告设备配网状态变化的回调函数]
 * @param  cb [配网状态回调函数，该函数由应用层定义，设备配网状态每次的改变都会调用该函数]
 * @return    [无]
 */
void vesync_regist_devstatus_cb(device_status_cb_t cb)
{
	dev_status_callback = cb;
}

vesync_router_link_status_t vesync_get_router_link_status(void)
{
	return router_status;
}
/**
 * @brief 设置路由器连接状态
 * @param status 
 */
void vesync_set_router_link_status(vesync_router_link_status_t status)
{
	LOG_I(TAG, "vesync_set_router_link_status status:%d ,router_status:%d", status,router_status);
	if(router_status != status){
		router_status = status;
		if(router_link_status_callback != NULL)
			router_link_status_callback(status);
	}
}

/**
 * @brief 应用层注册路由器连接状态
 * @param cb 
 */
void vesync_regist_router_link_status_cb(vesync_router_link_status_cb_t cb)
{
	router_link_status_callback = cb;
}


/**
 * [vesync_reset_device_conf 设备配置清空复位]
 * @param  dev_conf [配置信息结构体指针]
 * @return         [无]
 */
void vesync_reset_device_conf(vesync_device_conf_t* dev_conf)
{
	memset(dev_conf, 0, sizeof(vesync_device_conf_t));		//配置信息全部直接清零
}

/**
 * @brief 获取设备CID。CID由设备产测通过的最后一个步骤写入FLASH，永久使用
 * @param cid_buf 	[需要获取CID的数据保存区]
 * @return int 		[SUCCESS - 成功读取到CID；FAIL - CID不存在]
 */
int vesync_get_device_cid(char *cid_buf)
{
	//开发调试使用的CID，暂时使用写死的方式保存CID供调试
	strcpy(cid_buf, "0LIUYTLBBdCZndZslg_9HXoRHdHQ2v22");

	// if(s_product_config.cid_holder == CID_HOLDER)		//已通过产测，被分配了CID
	// {
	// 	strcpy(cid_buf, s_product_config.cid);
	// }
	// else
	// {
	// 	//没有cid标志，清空处理
	// 	memset(&s_product_config, 0, sizeof(s_product_config));
	// 	return FAIL;
	// }

	return 0;
}

/**
 * @brief 获取设备pid。pid由应用层通过宏定义传入，标记不同产品
 * @param pid_buf 	[需要获取pid的数据保存区]
 */
void vesync_get_device_pid(char *pid_buf)
{
	strcpy(pid_buf, DEV_PID);
}

/**
 * @brief 获取设备AuthKey。AuthKey由应用层通过宏定义传入，标记不同厂商
 * @param authkey_buf 	[需要获取AuthKey的数据保存区]
 */
void vesync_get_device_authkey(char *authkey_buf)
{
	strcpy(authkey_buf, DEV_AUTHKEY);
}

/**
 * @brief 设置设备产品信息，把设备产测通过后保存在FLASH里的信息读取到产品信息全局变量
 * @param product_conf 	[设备产品信息结构体]
 * @return 				[无]
 */
void vesync_get_product_config(vesync_product_conf_t *product_conf)
{
	vesync_get_device_cid(product_conf->cid);
	vesync_get_device_authkey(product_conf->authkey);
	vesync_get_device_pid(product_conf->pid);
}

/**
 * [vesync_init_device_client 初始化vesync云平台设备客户端，使用标准固件必须先调用该初始化函数]
 * @return  [无]
 */
// void vesync_init_device_client(void)
// {
// 	//获取设备产品信息，包括cid、pid、authkey
// 	vesync_get_product_config(&s_product_config);

// 	// vesync_user_entry();

// 	//加载保存的配置信息
// 	// int ret = vesync_configuration_read_flash(NVDM_ITEM_DEVICE_CONFIG, &s_device_config, sizeof(s_device_config));
// 	// if(NVDM_STATUS_OK == ret)
// 	// {
// 	// 	vesync_device_init(&s_device_config);
// 	// }
// 	// else
// 	// 	LOG_E(TAG, "Read device configuration error : %d.",ret);
// }
