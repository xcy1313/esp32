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
	uint8_t unit;
	//app_sacle_pin_rst_delay();
	vesync_regist_devstatus_cb(device_status);
	vesync_developer_start();
	vesync_set_production_status(PRODUCTION_EXIT);		//状态调整为未进入产测模式;
	LOG_I(TAG, "find product test cid ok[%s]",product_config.cid);
	LOG_E(TAG, "Application layer start with versiom[%s]",FIRM_VERSION);

	if(0 == vesync_flash_read_i8(UNIT_NAMESPACE,UNIT_KEY,&info_str.user_config_data.measu_unit)){	//上电读取默认单位;
        ESP_LOGE(TAG, "read last unit is[%d]",info_str.user_config_data.measu_unit);
    }else{
		info_str.user_config_data.measu_unit = UNIT_LB;
		ESP_LOGE(TAG, "default unit is[%d]",info_str.user_config_data.measu_unit);
	}
	app_uart_start();
	app_ble_init();
	app_scales_start();
	app_hadle_server_create();
	
	vTaskDelete(NULL);
}
