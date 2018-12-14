/**
 * @file vesync_flash.c
 * @brief vesync设备flash读写驱动
 * @author WatWu
 * @date 2018-11-20
 */

#include <stdint.h>
#include "vesync_flash.h"

/**
 * [vesync_configuration_write_flash 自定义配置信息写入flash]
 * @param  item   	 [配置信息名称，用于区分不同的配置数据]
 * @param  config    [配置信息指针]
 * @param  length    [配置信息长度]
 * @return           [写入结果]
 */
int vesync_configuration_write_flash(const char *item, void* config, uint32_t length)
{
	int ret = 0;

	return ret;
}

/**
 * [vesync_configuration_read_flash 读取自定义配置信息]
 * @param  item  	[配置信息名称，用于区分不同的配置数据]
 * @param  config   [配置信息指针]
 * @param  length   [配置信息长度]
 * @return          [读取结果]
 */
int vesync_configuration_read_flash(const char *item, void* config, uint32_t length)
{
	int ret = 0;

	return ret;
}
