/**
 * @file vesync_build_cfg.h
 * @brief vesync设备相关的配置和宏定义，在此处统一进行对具体产品固件进行配置
 * @author WatWu
 * @date 2018-11-16
 */

#ifndef VESYNC_BUILD_CONFIG_H
#define VESYNC_BUILD_CONFIG_H

//固件开启开发者模式， 方便固件调试，不受配网影响，产品发布时关闭。
//#define TCP_DEBUG_ENABLE

#define TCP_AES_ENABLE

//项目名称
#define PRODUCT_WIFI_NAME               "WiFiBT_Scale_FatScalePlus_US"
//==========实际应用项目的相关配置==========
#define PRODUCT_FIRM_VERSION			"0.0.01"

//产品PID
#define DEV_PID							"v2zm0t0a4l6oylxs"

//产品authkey
#define DEV_AUTHKEY						"ww6zvzwtaa6hqjl991g0w1obkeo0sd44"

//定义有效则恢复出厂设置不重启设备，默认是重启
// #define RESTORE_DONOT_REBOOT

//产测系统配置，基线内部产测系统使用production名称，factory工厂模式留给应用层使用
#define PRODUCTION_WIFI_SSID			"R6100-2.4G"//"VeSync_Pruduction_AP"
#define PRODUCTION_WIFI_KEY				"12345678"//"VeSync2018@etekcity.com.cn"
#define PRODUCTION_SERVER_ADDR			"192.168.100.67"//"inwallswitch-3ch.us.factory.vesync.com" //产测服务器地址
#endif
