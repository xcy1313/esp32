/**
 * Copyright © Etekcity Technologies Co., Ltd. 2018-2018. All rights reserved.
 * @file user_log.h
 * @brief 用户打印日志头文件
 * @author Blue.Qi
 * @date 2018-05-25
 */
#ifndef USER_LOG_H
#define USER_LOG_H

#include "c_types.h"

//对于标准级别它们关系如下：ALL < DEBUG < INFO < WARN < ERROR < FATAL < OFF
//应用层日志级别
typedef enum UserLogLevel_E
{
	USER_LOG_DEBUG, 	//指定细粒度信息事件是最有用的应用程序调试
	USER_LOG_INFO,		//指定能够突出在粗粒度级别的应用程序运行情况的信息的消息
	USER_LOG_WARN,		//指定具有潜在危害的情况
	USER_LOG_ERROR,		//错误事件可能仍然允许应用程序继续运行
} UserLogLevel_E;

extern UserLogLevel_E g_logLevel;
extern unsigned char g_isLogEnable;

/**
 * @brief 设置用户日志打印级别
 * @param level	[日志级别]
 */
void ICACHE_FLASH_ATTR UserSetLogLevel(UserLogLevel_E level);

/**
 * @brief 开启用户日志打印功能
 */
void ICACHE_FLASH_ATTR UserLogEnable(void);

/**
 * @brief 关闭用户日志打印功能
 */
void ICACHE_FLASH_ATTR UserLogDisable(void);

/**
 * @brief 	用户日志打印初始化
 * @param 	level	[打印级别]
 */
void ICACHE_FLASH_ATTR UserLogInit(UserLogLevel_E level);

/**
 * @brief vesync内部调试打印信息输出
 * @param level		[打印级别]
 * @param format 	[格式化参数]
 * @param ... 		[可变参数]
 */
void ICACHE_FLASH_ATTR VesyncDebugPrintf(UserLogLevel_E level, const char * format, ...);

/**
 * @brief 应用层调试信息打印输出
 * @param level		[打印级别]
 * @param format 	[格式化参数]
 * @param ... 		[可变参数]
 */
void ICACHE_FLASH_ATTR DebugPrintf(UserLogLevel_E level, const char * format, ...);

//level 日志级别
//format 日志内容,不定参
#define USER_LOG(level, format,...) \
    do \
    {\
		static const char flash_str[] ICACHE_RODATA_ATTR STORE_ATTR = format;	\
        DebugPrintf(level, flash_str,##__VA_ARGS__); \
    }while(0)

//vesync sdk 日志打印
//level 日志级别
//format 日志内容,不定参
#define VESYNC_LOG(level, format, ...) \
	do \
	{\
		static const char flash_str[] ICACHE_RODATA_ATTR STORE_ATTR = format;	\
		VesyncDebugPrintf(level, flash_str,##__VA_ARGS__); \
	}while(0)

#endif
