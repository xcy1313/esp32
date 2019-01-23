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

#define xTCP_AES_ENABLE

//==========实际应用项目的相关配置==========
#define PRODUCT_TEST_TYPE 0x88
#define PRODUCT_TEST_NUM  0x88

#define PRODUCT_VER		                0x1
#define PRODUCT_TYPE                    0xC0
#define PRODUCT_NUM		                0xA4
#define PRODUCT_NAME	                "ESF00+"

//项目名称
#define PRODUCT_WIFI_NAME               "WiFiBT_Scale_FatScalePlus_US"
//==========实际应用项目的相关配置==========
#define FIRM_VERSION			        "0.0.11"
//设备类型名称定义
#define DEVICE_TYPE						"vesync_template"

//产品PID
#define DEV_PID							"v2zm0t0a4l6oylxs"

//产品authkey
#define DEV_AUTHKEY						"ww6zvzwtaa6hqjl991g0w1obkeo0sd44"

//定义有效则恢复出厂设置不重启设备，默认是重启
// #define RESTORE_DONOT_REBOOT

//产测系统配置，基线内部产测系统使用production名称，factory工厂模式留给应用层使用
#define PRODUCTION_WIFI_SSID		    "R6100-2.4G"	//"VeSync_Pruduction_AP"
#define PRODUCTION_WIFI_KEY				"12345678" //"VeSync2018@etekcity.com.cn"
#define PRODUCTION_SERVER_ADDR			"34.207.59.169" //产测服务器地址 esf00-fatscale.us.factory.vesync.com"//"192.168.100.67"

#endif
