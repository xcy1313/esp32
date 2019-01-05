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
	app_uart_start();
	app_button_start();
	//vesync_init_sntp_service(1544410822,8,"ntp.vesync.com");

	if(vesync_flash_read_product_config(&product_config)){
		LOG_I(TAG, "find product test cid ok[%s]",product_config.cid);
		vesync_set_device_status(DEV_CONFNET_OFFLINE);		//已配网但未连接上服务器
		vesync_set_production_status(PRODUCTION_EXIT);		//状态调整为未进入产测模式;
  		if(vesync_flash_read_net_info(&net_info) == true){
	 		vesync_client_connect_wifi((char *)net_info.station_config.wifiSSID, (char *)net_info.station_config.wifiPassword);
		}else{
			LOG_I(TAG, "first time use!!!!");
			vesync_set_device_status(DEV_CONFNET_NOT_CON);	//第一次使用，未配网
		}
	}else{
		LOG_E(TAG, "enter product test mode[%s]",product_config.cid);
		vesync_regist_recvjson_cb(vesync_recv_json_data);
	 	vesync_enter_production_testmode(NULL);
		resend_cmd_bit &= ~RESEND_CMD_ALL_BIT;
		resend_cmd_bit |= RESEND_CMD_FACTORY_START_BIT;
		app_uart_encode_send(MASTER_SET,CMD_FACTORY_SYNC_START,0,0,true);
	}
	
	LOG_I(TAG, "Application layer start !");
	if(PRODUCTION_EXIT == vesync_get_production_status()){
		app_ble_init();
		app_scales_start();
		app_hadle_server_create();
    }
	vTaskDelete(NULL);
}
