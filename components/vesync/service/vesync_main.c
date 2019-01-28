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

			}
			if(notified_value & HTTPS_NET_CONFIG_REGISTER){
				vesync_json_add_https_service_register(NETWORK_CONFIG_REQ);
			}
			if(notified_value & REFRESH_HTTPS_TROKEN){
				vesync_json_add_https_service_register(REFRESH_TOKEN_REQ);
			}
			if(notified_value & NETWORK_DISCONNECTED)
			{
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
					vesync_production_connected_report_to_server();
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
		vesync_set_device_status(DEV_CONFNET_OFFLINE);		//已配网但未连接上服务器
		vesync_client_connect_wifi((char *)net_info.station_config.wifiSSID, (char *)net_info.station_config.wifiPassword);
	}else{
		vesync_set_device_status(DEV_CONFNET_NOT_CON);	//第一次使用，未配网
	}
	uint8_t test_cid[] = "0LWPG6SG9xBPtnQaJbD8qCxVk2GKwMI1";
// 0LWPJNML3eqV3fLKZo7zTAOJyJpbZq71
// 0LWPG6SG9xBPtnQaJbD8qCxVk2GKwMI1
	strcpy((char *)product_config.cid,(char *)test_cid);
	LOG_I(TAG, "device status : %d\n" ,vesync_get_device_status());

	while(1){
		// LOG_I(TAG, "ESP8266 FreeRTOS printf !");
		// printf_os_task_manager();

		//仅需要在一个任务中使用较低的系统延时值就可以使整个系统保持较高频的任务调度
		//因为空闲任务和中断型任务函数无法即时触发调度，要依赖于系统的自动调度
		//而低频的任务调度会使类似xTaskNotifyFromISR等的中断型任务通知无法及时响应
		sleep(5);
	}
}
