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

#include "vesync_wifi.h"
#include "vesync_flash.h"
#include "vesync_net_service.h"
#include "vesync_production.h"
#include "vesync_interface.h"
#include "vesync_sntp_service.h"

#include "app_handle_phone.h"
#include "app_handle_scales.h"
#include "app_handle_server.h"

static const char* TAG = "vesync_user";

/**
 * @brief vesync平台应用层入口函数
 */
void vesync_user_entry(void *args)
{
	vesync_set_production_status(PRODUCTION_EXIT);		//状态调整为未进入产测模式;
	app_ble_init();
	app_uart_start();
	app_scales_start();

	LOG_I(TAG, "find product test cid ok[%s]",product_config.cid);
	if(vesync_flash_read_net_info(&net_info) == true){
		app_handle_set_net_status(NET_CONFNET_OFFLINE);		//已配网但未连接上服务器
		//vesync_client_connect_wifi((char *)net_info.station_config.wifiSSID, (char *)net_info.station_config.wifiPassword);
	}else{
		LOG_I(TAG, "first time use!!!!");
		app_handle_set_net_status(NET_CONFNET_NOT_CON);	//第一次使用，未配网
	}
	LOG_I(TAG, "Application layer start !");
	//vesync_client_connect_wifi("R6100-2.4G", "12345678");	// wifi driver初始化，否则无法获取mac地址
	//vesync_init_sntp_service(1544410822,8,"ntp.vesync.com");
	app_hadle_server_create();
	
	vTaskDelete(NULL);
}
