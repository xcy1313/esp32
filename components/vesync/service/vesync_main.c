/**
 * @file vesync_main.c
 * @brief vesync平台SDK入口文件
 * @author WatWu
 * @date 2018-08-14
 */

#include "vesync_main.h"
#include "vesync_developer.h"
#include "vesync_wifi.h"
#include "vesync_net_service.h"
#include "vesync_production.h"
#include "vesync_interface.h"
#include "vesync_sntp_service.h"
#include "vesync_flash.h"
#include "vesync_log.h"

#include "vesync_https.h"
#include "vesync_device.h"
#include "vesync_ota.h"

static const char* TAG = "vesync_main";

//任务句柄定义
TaskHandle_t event_center_taskhd = NULL;
TaskHandle_t https_task_handler = NULL;
vesync_application_cb_t vesync_application_cb = NULL;

/**
 * @brief vesync事件处理中心
 * @param args [无]
 */
static void vesync_event_center_thread(void *args)
{
	BaseType_t notified_ret;
	uint32_t notified_value;

	while(1)
	{
		notified_ret = xTaskNotifyWait(0x00000000, 0xFFFFFFFF, &notified_value, 10000 / portTICK_RATE_MS);
		if(pdPASS == notified_ret){
			LOG_I(TAG, "Event center get new notified : %x.", notified_value);

			if(notified_value & NETWORK_CONNECTED){
				if(strlen((char *)product_config.cid) !=0){
					if(vesync_get_device_status() == DEV_CONFIG_NET_READY){
						vesync_register_https_net();	//请求配网;
					}
				}
			}
			if(notified_value & HTTPS_NET_CONFIG_REGISTER){
				vesync_json_add_https_service_register(NETWORK_CONFIG_REQ);
			}
			if(notified_value & REFRESH_HTTPS_TROKEN){
				vesync_json_add_https_service_register(REFRESH_TOKEN_REQ);
			}
			if(notified_value & UPGRADE_REFRESH_DEVICE_ATTRIBUTE){
				vesync_json_add_https_service_register(UPGRADE_REFRESH_ATTRIBUTE_REQ);
			}
			if(notified_value & NETWORK_DISCONNECTED){
				
			}
			if(notified_value & RECEIVE_UART_DATA)
			{
			}

			if(notified_value & CONFIG_NET_MQTT_READY)
			{
			}

			if(notified_value & MQTT_CONNECTED)
			{
				if(vesync_get_production_status() != PRODUCTION_EXIT)		//连上的mqtt服务器为产测服务器
				{
					vesync_subscribe_production_topic();
					if(strlen((char *)product_config.cid) !=0){
						vesync_production_connected_report_to_server("reconn");
					}else{
						vesync_production_connected_report_to_server("first");
					}
				}
			}
		}
		else
		{
			// LOG_I(TAG, "No notified.");
		}
	}
}

/**
 * @brief 注册回调应用程接口
 * @param cb 
 */
void vesync_register_application_cb(vesync_application_cb_t cb)
{
	if(NULL != cb){
		vesync_application_cb = cb;
	}
}
/**
 * @brief vesync平台入口
 * @param args [无]
 */
void vesync_entry(void *args)
{
	if(pdPASS != xTaskCreate(vesync_event_center_thread,
	                         EVENT_TASK_NAME,
	                         EVENT_TASK_STACSIZE / sizeof(portSTACK_TYPE),
	                         NULL,
	                         EVENT_TASK_PRIO,
	                         &event_center_taskhd))
	{
		LOG_E(TAG, "Create event center task fail !");
	}
	vesync_clinet_wifi_module_init(true);
	vesync_init_sntp_service(1544410793,8,"ntp.vesync.com");
	vesync_init_https_module(vesync_https_ca_cert_pem);
	if(NULL != vesync_application_cb){
		vesync_application_cb();
	}
	vesync_flash_read_product_config(&product_config);
	if(vesync_flash_read_net_info(&net_info) == true){
		vesync_set_device_status(DEV_CONFIG_NET_RECORDS);		//已配网但未连接上服务器
		vesync_client_connect_wifi((char *)net_info.station_config.wifiSSID, (char *)net_info.station_config.wifiPassword);
	}else{
		LOG_E(TAG, "net config is NULL");
		vesync_set_device_status(DEV_CONFIG_NET_NULL);			//第一次使用，未配网
	}
	// uint8_t test_cid[] = "0LWPG6SG9xBPtnQaJbD8qCxVk2GKwMI1"; //Eric：0LZ8xknbQJC41fgVvG79w06tGLsA_jK1   0LWPG6SG9xBPtnQaJbD8qCxVk2GKwMI1
	// strcpy((char *)product_config.cid,(char *)test_cid);

	while(1){
		sleep(5);
	}
}
