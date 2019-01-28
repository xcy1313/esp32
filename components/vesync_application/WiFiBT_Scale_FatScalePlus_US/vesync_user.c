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
	app_sacle_pin_rst_delay();
	vesync_regist_devstatus_cb(device_status);
	// uint8_t test_cid[] = "-LBjKhfYG-U1i3TvOrshQNN5StRl3uT1";
	// strcpy((char *)product_config.cid,(char *)test_cid);
	LOG_I(TAG, "find product test cid ok[%s]",product_config.cid);
	vesync_developer_start();
	vesync_set_production_status(PRODUCTION_EXIT);		//状态调整为未进入产测模式;
	LOG_E(TAG, "Application layer start with versiom[%s]",FIRM_VERSION);
	app_ble_init();
	app_uart_start();
	app_scales_start();
	app_hadle_server_create();
	
	vTaskDelete(NULL);
}
