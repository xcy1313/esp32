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

#include "vesync_device.h"
#include "vesync_ota.h"

#include "vesync_https.h"

static const char* TAG = "vesync_main";

//任务句柄定义
TaskHandle_t event_center_taskhd = NULL;
TaskHandle_t https_task_handler = NULL;

static void vesync_https_req_task(void *pvParameters)
{
	int ret;
	char recv_buff[1024];
	int buff_len = sizeof(recv_buff);
	while(1)
	ret = vesync_https_client_request("deviceRegister", "hello", recv_buff, &buff_len, 2 * 1000);
	if(buff_len > 0 && ret == 0){
		LOG_I(TAG, "Https recv %d byte data : \n%s", buff_len, recv_buff);
	}
}
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

			if(notified_value & NETWORK_CONNECTED)
			{
				// vesync_mqtt_client_connect_to_cloud();
				xTaskCreate(&vesync_https_req_task, "vesync_https_req_task", 8192, NULL, 5, https_task_handler);
			}

			if(notified_value & NETWORK_DISCONNECTED)
			{
				vTaskDelete(https_task_handler);
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
	vesync_clinet_wifi_module_init(true);
	vesync_init_sntp_service(1544410793,8,"ntp.vesync.com");
	vesync_init_https_module(vesync_https_ca_cert_pem);
	if(pdPASS != xTaskCreate(vesync_event_center_thread,
	                         EVENT_TASK_NAME,
	                         EVENT_TASK_STACSIZE / sizeof(portSTACK_TYPE),
	                         NULL,
	                         EVENT_TASK_PRIO,
	                         &event_center_taskhd))
	{
		LOG_E(TAG, "Create event center task fail !");
	}

	while(1)
	{
		// LOG_I(TAG, "ESP8266 FreeRTOS printf !");
		// printf_os_task_manager();

		//仅需要在一个任务中使用较低的系统延时值就可以使整个系统保持较高频的任务调度
		//因为空闲任务和中断型任务函数无法即时触发调度，要依赖于系统的自动调度
		//而低频的任务调度会使类似xTaskNotifyFromISR等的中断型任务通知无法及时响应
		sleep(5);
	}
}
