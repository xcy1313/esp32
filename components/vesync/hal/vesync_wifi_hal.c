/**
 * @file vesync_wifi_hal.c
 * @brief vesync设备WiFi功能硬件抽象层
 * @author WatWu
 * @date 2018-11-17
 */

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_event_loop.h"

#include "vesync_wifi_hal.h"
#include "vesync_log.h"

static const char *TAG = "vesync_wifi_hal";

static vesync_wifi_cb s_wifi_connect_callback = NULL;		//wifi连接状态回调函数指针

/**
 * @brief 硬件抽象层调用上一层传递的WiFi连接回调函数
 * @param wifi_status [WiFi状态枚举]
 */
static void hal_call_wifi_connect_callback(vesync_wifi_status_e wifi_status)
{
	if(NULL != s_wifi_connect_callback)
	{
		s_wifi_connect_callback(wifi_status);
	}
}

/**
 * @brief wifi连接事件回调函数
 * @param ctx 			[保留未使用]
 * @param event 		[系统事件指针]
 * @return esp_err_t 	[错误码]
 */
static esp_err_t event_handler(void *ctx, system_event_t *event)
{
	switch(event->event_id)
	{
		case SYSTEM_EVENT_STA_START:
			esp_wifi_connect();
			break;
		case SYSTEM_EVENT_STA_GOT_IP:
			LOG_I(TAG, "Device got ip : %s", ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
			hal_call_wifi_connect_callback(VESYNC_WIFI_GOT_IP);
			break;
		case SYSTEM_EVENT_AP_STACONNECTED:
			LOG_I(TAG, "Station:"MACSTR" join, AID=%d",	MAC2STR(event->event_info.sta_connected.mac),
				  event->event_info.sta_connected.aid);
			break;
		case SYSTEM_EVENT_AP_STADISCONNECTED:
			LOG_I(TAG, "Station:"MACSTR"leave, AID=%d",	MAC2STR(event->event_info.sta_disconnected.mac),
				  event->event_info.sta_disconnected.aid);
			break;
		case SYSTEM_EVENT_STA_DISCONNECTED:
		{
			uint8_t reason = event->event_info.disconnected.reason;
			LOG_E(TAG, "Disconnect reason : %d", reason);
			switch(reason)
			{
				case WIFI_REASON_NO_AP_FOUND:
					hal_call_wifi_connect_callback(VESYNC_WIFI_NO_AP_FOUND);
					break;
				case WIFI_REASON_AUTH_FAIL:
					hal_call_wifi_connect_callback(VESYNC_WIFI_WRONG_PASSWORD);
					break;
				case WIFI_REASON_ASSOC_FAIL:
					hal_call_wifi_connect_callback(VESYNC_WIFI_CONNECT_FAIL);
					break;
				default:
					hal_call_wifi_connect_callback(VESYNC_WIFI_LOST_IP);
					break;
			}
			esp_wifi_connect();
			break;
		}
		case SYSTEM_EVENT_SCAN_DONE: {
            uint16_t apCount = 0;
            esp_wifi_scan_get_ap_num(&apCount);
            if (apCount == 0) {
                ESP_LOGI(TAG,"Nothing AP found");
                break;
            }
            wifi_ap_record_t *ap_list = (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * apCount);
			memset(ap_list ,NULL,sizeof(wifi_ap_record_t) * apCount);
            if (!ap_list) {
                ESP_LOGI(TAG,"malloc error, ap_list is NULL");
                break;
            }
            wifi_ap_record_t *blufi_ap_list = (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * apCount);
            if (!blufi_ap_list) {
                ESP_LOGI(TAG,"malloc error, blufi_ap_list is NULL");
                break;
            }
            ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&apCount, ap_list));

            for (int i = 0; i < apCount; ++i){
                blufi_ap_list[i].authmode = ap_list[i].authmode; 
                blufi_ap_list[i].rssi = ap_list[i].rssi;
                memcpy(blufi_ap_list[i].ssid, ap_list[i].ssid, sizeof(ap_list[i].ssid));
            }
            free(ap_list);
            free(blufi_ap_list);
			vesync_hal_scan_stop();
			hal_call_wifi_connect_callback(VESYNC_WIFI_SCAN_DONE);
            break;
        } 
		default:
			LOG_I(TAG, "WiFi untreated event id : %d", event->event_id);
			break;
	}
	return ESP_OK;
}

/**
 * @brief 硬件抽象层初始化wifi模块
 */
void vesync_hal_init_wifi_module(void)
{
	tcpip_adapter_init();
	ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));		//默认启动时为客户端模式，必须设置！
}

/**
 * @brief 硬件抽象层设置设备为WiFi热点
 * @param ap_ssid 	[ssid名称]
 * @param password 	[WiFi密码]
 * @param chan 		[WiFi信道]
 * @param auth_mode [加密方式]
 */
void vesync_hal_setup_wifi_ap(char *ap_ssid, char* password, uint8_t chan, uint8_t auth_mode)
{
	wifi_config_t wifi_config;
	memset(&wifi_config, 0, sizeof(wifi_config_t));
	strcpy((char *)&(wifi_config.ap.ssid), ap_ssid);	
	strcpy((char *)&(wifi_config.ap.password), password);
	wifi_config.ap.ssid_len = strlen(ap_ssid);	
	wifi_config.ap.channel = chan;
	wifi_config.ap.authmode = auth_mode;
	wifi_config.ap.max_connection = 4;

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
	ESP_ERROR_CHECK(esp_wifi_start());
}

/**
 * @brief 硬件抽象层设置设备为WiFi热点+客户端共存
 * @param ap_ssid 	[ssid名称]
 * @param password 	[WiFi密码]
 * @param chan 		[WiFi信道]
 * @param auth_mode [加密方式]
 */
void vesync_hal_setup_wifi_ap_and_sta(char *ap_ssid, char* password, uint8_t chan, uint8_t auth_mode)
{
	wifi_config_t wifi_config;
	memset(&wifi_config, 0, sizeof(wifi_config_t));
	strcpy((char *)&(wifi_config.ap.ssid), ap_ssid);	
	strcpy((char *)&(wifi_config.ap.password), password);
	wifi_config.ap.ssid_len = strlen(ap_ssid);	
	wifi_config.ap.channel = chan;
	wifi_config.ap.authmode = auth_mode;
	wifi_config.ap.max_connection = 4;

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
	ESP_ERROR_CHECK(esp_wifi_start());
}

/**
 * @brief 硬件抽象层设置设备的wifi模式
 * @param target_mode [目标模式]
 * @return int8_t 	  [设置结果，0为成功]
 */
int8_t vesync_hal_set_wifi_mode(uint8_t target_mode)
{
	int8_t ret;
	ret	= esp_wifi_set_mode(target_mode);
	return ret;
}

/**
 * @brief 硬件抽象层获取设备当前wifi模式
 * @return uint8_t [当前wifi模式]
 */
uint8_t vesync_hal_get_wifi_mode(void)
{
	uint8_t mode;
	ESP_ERROR_CHECK(esp_wifi_get_mode((wifi_mode_t *)&mode));
	return mode;
}

/**
 * @brief 硬件抽象层连接WiFi
 * @param wifi_ssid			[WiFi名称]
 * @param wifi_password 	[WiFi密码]
 * @param callback 			[WiFi连接回调函数]
 */
void vesync_hal_connect_wifi(char *wifi_ssid, char *wifi_password, vesync_wifi_cb callback)
{
	wifi_config_t wifi_config;
	memset(&wifi_config, 0, sizeof(wifi_config_t));
	strcpy((char *) & (wifi_config.sta.ssid), wifi_ssid);
	strcpy((char *) & (wifi_config.sta.password), wifi_password);

	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
	ESP_ERROR_CHECK(esp_wifi_start());

	if(NULL != callback)
	{
		s_wifi_connect_callback = callback;
	}
}

/**
 * @brief 硬件抽象层获取mac地址字符串
 * @param interface 		[WiFi接口，AP或STA]
 * @param mac_str_buffer 	[mac地址字符串缓存区，大小必须大于等于18字节]
 * @return int 				[获取结果]
 */
int vesync_hal_get_mac_string(int interface, char *mac_str_buffer)
{
	uint8_t mac_addr[6];
	esp_wifi_get_mac(interface, mac_addr);
	sprintf(mac_str_buffer, MACSTR, MAC2STR(mac_addr));
	return 0;
}

/**
 * @brief 启动扫描WiFi列表
 * @return int [扫描结果]
 */
int vesync_hal_scan_wifi_list_start(void)
{
    int ret;
    wifi_scan_config_t scanConf = {
                .ssid = NULL,
                .bssid = NULL,
                .channel = 0,
                .show_hidden = false
            };
    LOG_I(TAG,"blufi wifi scan start\n");

    ret = esp_wifi_scan_start(&scanConf, true);

    if(ESP_OK != ret){
        LOG_E(TAG ,"blufi wifi scan error :%d\n" ,ret);
    }
	return ret;
}

/**
 * @brief 停止扫描WiFi列表
 * @return int [扫描结果]
 */
int vesync_hal_scan_stop(void)
{
    LOG_I(TAG,"blufi wifi scan stop\n");
    return esp_wifi_scan_stop();
}