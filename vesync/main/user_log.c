/**
 * Copyright © Etekcity Technologies Co., Ltd. 2018-2018. All rights reserved.
 * @file user_log.c
 * @brief 用户打印日志
 * @author Blue.Qi
 * @date 2018-05-25
 */

#include <stdarg.h>
#include "user_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//打印缓冲区大小
#define PRINT_BUFF_MAX	1024

//日志级别
UserLogLevel_E g_logLevel = USER_LOG_DEBUG;

//日志是否开启 0:关闭 非0:开启
unsigned char g_isLogEnable = 1;

/**
 * @brief 设置用户日志打印级别
 * @param level	[日志级别]
 */
void ICACHE_FLASH_ATTR UserSetLogLevel(UserLogLevel_E level)
{
	g_logLevel = level;
}

/**
 * @brief 开启用户日志打印功能
 */
void ICACHE_FLASH_ATTR UserLogEnable(void)
{
	g_isLogEnable = 1;
}

/**
 * @brief 关闭用户日志打印功能
 */
void ICACHE_FLASH_ATTR UserLogDisable(void)
{
	g_isLogEnable = 0;
}

/**
 * @brief 	用户日志打印初始化
 * @param 	level	[打印级别]
 */
void ICACHE_FLASH_ATTR UserLogInit(UserLogLevel_E level)
{
	UserSetLogLevel(level);
	UserLogEnable();
}

/**
 * @brief vesync内部调试打印信息输出
 * @param level		[打印级别]
 * @param format 	[格式化参数]
 * @param ... 		[可变参数]
 */
void ICACHE_FLASH_ATTR VesyncDebugPrintf(UserLogLevel_E level, const char * format, ...)
{
	if(g_isLogEnable && level >= g_logLevel)
	{
		va_list arg_ptr;
		char str[PRINT_BUFF_MAX];							//保存格式化后的字符串

		//format参数为flash中的变量
		char fmt[PRINT_BUFF_MAX];
		memcpy(fmt, format, PRINT_BUFF_MAX);				//使用内存中的数组fmt拷贝定义在flash中的数组format
		va_start(arg_ptr, format);							//获取函数可变参列表的起始位置，注意使用的是format，非fmt
		vsnprintf(str, PRINT_BUFF_MAX, fmt, arg_ptr);	//格式化字符串，注意使用的是fmt，非format，ets_vsnprintf不能直接读取FLASH
		va_end(arg_ptr);

		//format参数为内存中的变量，此段注释保留作为对比
		// va_start(arg_ptr, format);
		// ets_vsnprintf(str, PRINT_BUFF_MAX, format, arg_ptr);
		// va_end(arg_ptr);

		switch(level)
		{
			case USER_LOG_DEBUG :
				printf("[D]<Vesync>%s", str);//DEBUG
				break;
			case USER_LOG_INFO :
				printf("[I]<Vesync>%s", str);//INFO
				break;
			case USER_LOG_WARN :
				printf("[W]<Vesync>%s", str);//WARN
				break;
			case USER_LOG_ERROR :
				printf("[E]<Vesync>%s", str);//ERROR
				break;
			default:
				break;
		}
	}
}

/**
 * @brief 应用层调试信息打印输出
 * @param level		[打印级别]
 * @param format 	[格式化参数]
 * @param ... 		[可变参数]
 */
void ICACHE_FLASH_ATTR DebugPrintf(UserLogLevel_E level, const char * format, ...)
{
	if(g_isLogEnable && level >= g_logLevel)
	{
		va_list arg_ptr;
		char str[PRINT_BUFF_MAX];							//保存格式化后的字符串

		//format参数为flash中的变量
		char fmt[PRINT_BUFF_MAX];
		memcpy(fmt, format, PRINT_BUFF_MAX);				//使用内存中的数组fmt拷贝定义在flash中的数组format
		va_start(arg_ptr, format);							//获取函数可变参列表的起始位置，注意使用的是format，非fmt
		vsnprintf(str, PRINT_BUFF_MAX, fmt, arg_ptr);	//格式化字符串，注意使用的是fmt，非format，ets_vsnprintf不能直接读取FLASH
		va_end(arg_ptr);

		//format参数为内存中的变量，此段注释保留作为对比
		// va_start(arg_ptr, format);
		// ets_vsnprintf(str, PRINT_BUFF_MAX, format, arg_ptr);
		// va_end(arg_ptr);

		switch(level)
		{
			case USER_LOG_DEBUG :
				printf("[D]%s", str);//DEBUG
				break;
			case USER_LOG_INFO :
				printf("[I]%s", str);//INFO
				break;
			case USER_LOG_WARN :
				printf("[W]%s", str);//WARN
				break;
			case USER_LOG_ERROR :
				printf("[E]%s", str);//ERROR
				break;
			default:
				break;
		}
	}
}
