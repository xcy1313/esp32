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

//==========实际应用项目的相关配置==========
//设备类型名称定义
#define DEVICE_TYPE						"WiFiBT_Hygrometer_SmartHygrometer_US"
#define FIRM_VERSION					"0.0.01"

#define PRODUCT_VER		                0x01
#define PRODUCT_TYPE                    0xC4
#define PRODUCT_NUM		                0xF0
#define PRODUCT_NAME	                "LV-SG1"

// #define PRODUCT_VER		                0x12
// #define PRODUCT_TYPE                    0xC0
// #define PRODUCT_NUM		                0xA4
// #define PRODUCT_NAME	                "ESF00+"

//产品PID
#define DEV_PID							"2b9lja6xd0yd40jc"

//产品authkey
#define DEV_AUTHKEY						"ww6zvzwtaa6hqjl991g0w1obkeo0sd44"

//定义有效则恢复出厂设置不重启设备，默认是重启
// #define RESTORE_DONOT_REBOOT

//产测系统配置，基线内部产测系统使用production名称，factory工厂模式留给应用层使用
#define PRODUCTION_WIFI_SSID			"VeSync_Pruduction_AP"
#define PRODUCTION_WIFI_KEY				"VeSync2018@etekcity.com.cn"
#define PRODUCTION_SERVER_ADDR			"inwallswitch-3ch.us.factory.vesync.com" //产测服务器地址

#endif
