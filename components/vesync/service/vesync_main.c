/**
 * @file vesync_main.c
 * @brief vesync平台SDK入口文件
 * @author WatWu
 * @date 2018-08-14
 */

#include "vesync_main.h"
#include "vesync_developer.h"
#include "vesync_bt_driver.h"
#include "vesync_wifi.h"
#include "vesync_net_service.h"
#include "vesync_production.h"
#include "vesync_interface.h"
#include "vesync_sntp_service.h"
#include "nvs_flash.h"

static const char* TAG = "vesync_main";

//任务句柄定义
TaskHandle_t event_center_taskhd = NULL;

/**
 * @brief vesync事件处理中心
 * @param args [无]
 */
static void vesync_event_center_thread(void *args)
{
	BaseType_t notified_ret;
	uint32_t notified_value;

	//vesync_client_connect_wifi("R6100-2.4G", "12345678");
	// vesync_setup_wifi_open_ap_and_sta("ESP8266_FreeRTOS");

	vesync_developer_start();
	//vesync_enter_production_testmode(NULL);
	// vesync_config_cloud_mqtt_client("vesync_client", "192.168.16.25", 61613, "etekcity", "hardware");

	while(1)
	{
		notified_ret = xTaskNotifyWait(0x00000000, 0xFFFFFFFF, &notified_value, 10000 / portTICK_RATE_MS);
		if(pdPASS == notified_ret)
		{
			LOG_I(TAG, "Event center get new notified : %x.", notified_value);

			if(notified_value & NETWORK_CONNECTED)
			{
				// vesync_mqtt_client_connect_to_cloud();
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
	esp_err_t ret;
	/* Initialize NVS. */
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "nvs_flash_init failed (0x%x), erasing partition and retrying", ret);
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
	vesync_init_wifi_module();
	vesync_init_sntp_service();
	vesync_init_bt_module("esp_advertise",0x1,0x21,0x36,NULL,true);

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
	printf_os_task_manager();

		//仅需要在一个任务中使用较低的系统延时值就可以使整个系统保持较高频的任务调度
		//因为空闲任务和中断型任务函数无法即时触发调度，要依赖于系统的自动调度
		//而低频的任务调度会使类似xTaskNotifyFromISR等的中断型任务通知无法及时响应
		sleep(5);
	}
}
