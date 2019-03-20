/**
 * @file vesync_user.c
 * @brief vesync平台应用层
 * @author WatWu
 * @date 2018-09-04
 */

#include <string.h>
#include <stdlib.h>

#include "vesync_build_cfg.h"

#include "vesync_log.h"
#include "vesync_api.h"

#include "vesync_wifi.h"
#include "vesync_flash.h"
#include "vesync_net_service.h"
#include "vesync_production.h"
#include "vesync_interface.h"
#include "vesync_sntp_service.h"
#include "vesync_developer.h"
#include "vesync_main.h"

#include "app_handle_phone.h"
#include "app_handle_scales.h"
#include "app_handle_server.h"
#include "app_public_events.h"

static const char* TAG = "vesync_user";

void app_sacle_pin_rst_delay(void)
{
	gpio_config_t io_conf;
	gpio_num_t num = SCALE_RST_PIN;
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << num);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;
	gpio_config(&io_conf);

    gpio_set_level(num, 0);
	vTaskDelay(50 / portTICK_PERIOD_MS);
	gpio_set_level(num, 1);
}

/**
 * @brief vesync平台应用层入口函数
 */
void vesync_user_entry(void *args)
{
	uint16_t len;

	utc_time_t	last_time;
	vesync_nvs_read_data(UNIX_TIME_NAMESPACE,UNIX_TIME_KEY,(uint8_t *)&last_time,&len);
	LOG_I(TAG, "read last power off time len:%d,zone:%d ,time:%d",len,last_time.zone,last_time.unix_time);
	info_str.user_utc_time.zone = last_time.zone;
	vesync_driver_wifi_stop();
	vesync_init_sntp_service("ntp.vesync.com");

	switch (esp_sleep_get_wakeup_cause()){	
		case ESP_SLEEP_WAKEUP_EXT1: 		//判断唤醒源是否按键或者称重唤醒
		case ESP_SLEEP_WAKEUP_EXT0:
			LOG_I(TAG, "wake up from EXT IO!!!");
			break;
		default:{
				LOG_I(TAG, "wake up from RST!!!");
				if(len == sizeof(utc_time_t)){
					vesync_set_time(last_time.unix_time,last_time.zone);
				}
				if(DEV_CONFIG_NET_RECORDS == vesync_get_device_status()){
					vesync_client_connect_wifi((char *)net_info.station_config.wifiSSID, (char *)net_info.station_config.wifiPassword);
				}
			}
			break;
	}
	app_sacle_wakeup_pin_init();
	vesync_regist_devstatus_cb(device_status);
	vesync_developer_start();
	vesync_set_production_status(PRODUCTION_EXIT);		//状态调整为未进入产测模式;
	LOG_I(TAG, "find product test cid ok[%s]",product_config.cid);
	LOG_E(TAG, "Application layer start with versiom[%s]",FIRM_VERSION);

	if(0 == vesync_flash_read_i8(UNIT_NAMESPACE,UNIT_KEY,&info_str.user_config_data.measu_unit)){	//上电读取默认单位;
        //ESP_LOGE(TAG, "read last unit is[%d]",info_str.user_config_data.measu_unit);
    }else{
		info_str.user_config_data.measu_unit = UNIT_LB;
		ESP_LOGE(TAG, "default unit is[%d]",info_str.user_config_data.measu_unit);
	}
	app_uart_start();
	app_ble_init();
	app_scales_start();
	app_hadle_server_create();
	app_public_events_task_create();
	app_scale_suspend_start();
	
	vTaskDelete(NULL);
}
