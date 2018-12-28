/**
 * @file vesync_user.c
 * @brief vesync平台应用层
 * @author WatWu
 * @date 2018-09-04
 */

#include <string.h>
#include <stdlib.h>

#include "vesync_log.h"
#include "vesync_api.h"

#include "app_handle_phone.h"
#include "app_handle_scales.h"
#include "vesync_production.h"

static const char* TAG = "vesync_user";

/**
 * @brief vesync平台应用层入口函数
 */
void vesync_user_entry(void *args)
{
	LOG_I(TAG, "Application layer start !");
	if(PRODUCTION_EXIT == vesync_get_production_status()){
		app_ble_init();
		app_scales_start();
    }

	vTaskDelete(NULL);
}
