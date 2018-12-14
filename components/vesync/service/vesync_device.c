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

#include "freertos/timers.h"

static const char* TAG = "vesync_device";

// static vesync_device_conf_t s_device_config;					//设备配置信息
// static vesync_product_conf_t s_product_config;					//设备产品信息
static device_status_e device_status = DEV_CONFNET_OFFLINE;		//设备配网状态，默认为离线状态

static device_status_cb_t dev_status_callback = NULL;			//定义配网状态回调函数指针

/**
 * [vesync_get_device_status 获取设备状态]
 * @return  [设备配网状态状态，详细定义见DeviceStatus_t]
 */
device_status_e vesync_get_device_status(void)
{
	return device_status;
}

/**
 * [vesync_set_device_status 设置设备状态]
 * @param  status [设备状态]
 * @return        [无]
 */
void vesync_set_device_status(uint8_t status)
{
	if(status == DEV_CONFNET_OFFLINE)				//如果要变更状态为OFFLINE
	{
		if(device_status != DEV_CONFNET_NOT_CON)	//则先判断原先状态是否为未配网，未配网时保持状态为未配网
		{
			device_status = status;
		}
	}
	else											//设置未配网、配网上线则立即生效
	{
		device_status = status;
	}
	if(dev_status_callback != NULL)
		dev_status_callback(device_status);
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
 * [vesync_device_init 设备初始化]
 * @param  dev_conf [设备配置信息结构体指针]
 * @return			[无]
 */
void vesync_device_init(vesync_device_conf_t* dev_conf)
{
	// if( get_production_status() != PRODUCTION_EXIT )	//设备被标记进入产测模式
	// {
	// 	LOG_I(TAG, "Enter production mode !");
	// }
	// else
	if(dev_conf->sta_holder != DEVCONF_HOLDER)		//属于首次使用，未进行过配网
	{
		LOG_I(TAG, "This is device's first used !");

		vesync_set_device_status(DEV_CONFNET_NOT_CON);	//设备未配网
		vesync_reset_device_conf(dev_conf);
	}
	else												//正常使用时启动
	{
		vesync_set_device_status(DEV_CONFNET_OFFLINE);	//设备已配网但未连接上服务器
		// connect_wifi_if_need_static_ip();				//判断是否需要使用静态IP连接WiFi
		// vesync_connect_wifi ( dev_conf->wifi_conf.ssid, dev_conf->wifi_conf.password, NULL );
		// vesync_init_mqtt_client(MQTT_INIT_FOR_NORMAL_USE);	//正常使用初始化MQTT客户端
	}

#ifdef TCP_DEBUG_ENABLE
	dev_conf->tcp_debug = TCP_DEBUG_ON;
#endif

	// vesync_regist_networkconnected_cb(connect_to_mqtt_server);  //注册WiFi连接成功后进行连接mqtt服务器
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
