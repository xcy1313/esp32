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
// #include "vesync_uart.h"

#define VESYNC_WIFI_LED_GPIO			28				//WIFI LED指示灯引脚定义

static const char* TAG = "vesync_user";

/**
 * @brief WiFi指示灯状态回调函数，实时返回WiFi指示灯的亮灭状态
 * @param led_status [WiFi LED灯的实时亮灭状态]
 */
static void wifiled_status_callback(int led_status)
{
	// if(led_status == ON)
	// 	hal_gpio_set_output(VESYNC_WIFI_LED_GPIO, HAL_GPIO_DATA_HIGH);
	// else
	// 	hal_gpio_set_output(VESYNC_WIFI_LED_GPIO, HAL_GPIO_DATA_LOW);
}

/**
 * @brief 初始化LED的GPIO口
 */
static void init_wifi_led_gpio(void)
{
	// hal_gpio_init(VESYNC_WIFI_LED_GPIO);
	// hal_pinmux_set_function(VESYNC_WIFI_LED_GPIO, HAL_GPIO_28_GPIO28); 		//将引脚设置为在GPIO模式下运行。
	// hal_gpio_set_direction(VESYNC_WIFI_LED_GPIO, HAL_GPIO_DIRECTION_OUTPUT);
	// hal_gpio_set_output(VESYNC_WIFI_LED_GPIO, HAL_GPIO_DATA_LOW);
}

/**
 * @brief 应用层串口接收回调函数
 * @param recv_data [串口接收的数据]
 * @param recv_len 	[接收数据长度]
 */
// static void user_uart_recv_callback(uint8_t *recv_data, int recv_len)
// {
// 	if(strstr((char*)recv_data, "AT+5S") != NULL)					//smartconfig模式配网
// 	{
// 		LOG_I(TAG, "Smartconfig mode to configure network !");

// 		vesync_configure_network(SMART_CONFIG, NULL);
// 	}
// 	else if(strstr((char*)recv_data, "AT+10S") != NULL)			//AP模式配网
// 	{
// 		LOG_I(TAG, "AP mode to configure network !");

// 		vesync_configure_network(AP_CONFIG, NULL);
// 	}
// 	else if(strstr((char*)recv_data, "AT+15S") != NULL)			//回复出厂设置
// 	{
// 		LOG_I(TAG, "Restore Factory Defaults !");

// 		// vesync_publish_rawdata("Publish test", os_strlen("Publish test"), 2, 0);
// 		// reset_device_report();

// 		vesync_configure_network(RESET_NETWORK, NULL);
// 	}
// 	else if(strstr((char*)recv_data, "AT+LEDTEST=") != NULL)		//LED闪烁行为测试
// 	{
// 		char *str = strstr((char*)recv_data, "=");
// 		int behavior = atoi(str + 1);
// 		LOG_I(TAG, "Get LED test mode : %d",behavior);

// 		if(behavior <= 6)
// 			vesync_set_wifiled_behavior(behavior);
// 	}
// 	else if(strstr((char*)recv_data, "AT+PRODUCTION") != NULL)		//产测模式
// 	{
// 		LOG_I(TAG, "Enter production mode !");
// 		enter_production_testmode(NULL);
// 	}
// }

/**
 * @brief vesync平台应用层入口函数
 */
void vesync_user_entry(void *args)
{
	LOG_I(TAG, "Application layer start !");
	vTaskDelete(NULL);
}
