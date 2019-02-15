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
		LOG_I(TAG, "hal_call_wifi_connect_callback %d", wifi_status);
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
			hal_call_wifi_connect_callback(VESYNC_WIFI_CONNECTING);
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
		case SYSTEM_EVENT_SCAN_DONE: 
			hal_call_wifi_connect_callback(VESYNC_WIFI_SCAN_DONE);
			LOG_I(TAG, "blufi wifi scan done");
			break;
		default:
			LOG_I(TAG, "WiFi untreated event id : %d", event->event_id);
			break;
	}
	return ESP_OK;
}

/**
 * @brief WIFI Hal层注册wifi事件回调
 * @param callback 
 */
void vesync_hal_register_cb(vesync_wifi_cb callback)
{
	if(NULL != callback){
		s_wifi_connect_callback = callback;
	}else{
		LOG_E(TAG, "hal cb register fail");
	}
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
 * @brief 配置为station以备连接路由器
 * @param ssid 
 * @param pwd 
 */
void vesync_hal_connect_wifi(char *ssid ,char *pwd)
{
	static wifi_config_t wifi_config ={0};
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

	memset(&wifi_config,0,sizeof(wifi_config_t));
	if(pwd == NULL){
		LOG_E(TAG, "pwd is NULL");
		return;
	}
	ESP_ERROR_CHECK( esp_wifi_disconnect() );	//不加导致配网时发送扫描不到ap的bug;
	ESP_ERROR_CHECK( esp_wifi_stop() );

	strcpy((char *)wifi_config.sta.ssid,(char *)ssid);
	strcpy((char *)wifi_config.sta.password,(char *)pwd);
	ESP_LOGI(TAG, "Setting WiFi configuration SSID [%s],pwd [%s]",ssid,pwd);
	ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
	
	ESP_ERROR_CHECK( esp_wifi_start() );

	ESP_ERROR_CHECK( esp_wifi_connect() );
	ESP_LOGI(TAG, "wifi connect...........");
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
 * @brief 获取已连接的路由器的mac地址
 * @param router_mac 
 * @return int 
 */
int vesync_hal_get_router_mac_string(char *router_mac)
{
	/* AP information to which STA is connected */
	int ret;
	char mac_addr[6];
	wifi_ap_record_t ap_info;
	ret = esp_wifi_sta_get_ap_info(&ap_info);
	memcpy(mac_addr, (char *)ap_info.bssid, sizeof(ap_info.bssid));
	sprintf(router_mac, MACSTR, MAC2STR(mac_addr));
	return ret;
}

// int vesync_hal_get_rssi(int interface, char *mac_str_buffer)
// {
// 	wifi_config_t wifi_cfg;
// 	esp_wifi_get_config(interface,&wifi_cfg);
// }
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
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK( esp_wifi_stop() );

	ret = esp_wifi_deinit();
	if(ESP_OK != ret){
		LOG_E(TAG ,"blufi wifi deinit error :%d\n" ,ret);
		return ret;
	}
	ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
	ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
	ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );

	ESP_ERROR_CHECK( esp_wifi_set_ps(WIFI_PS_MODEM));			//开启wifi省电模式;
	ESP_ERROR_CHECK( esp_wifi_start() );
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
	LOG_I(TAG,"blufi wifi scan stop");
	
	return esp_wifi_scan_stop();
}

/**
 * @brief 硬件抽象层初始化wifi模块
 * @param callback 
 * @param power_save 是否使能低功耗模式
 */
void vesync_hal_init_wifi_module(vesync_wifi_cb callback,bool power_save)
{
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

	tcpip_adapter_init();
	vesync_hal_register_cb(callback);
	ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

	ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
	ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
	ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );

	if(power_save){
		ESP_ERROR_CHECK( esp_wifi_set_ps(WIFI_PS_MODEM));			//开启wifi省电模式;
	}
	ESP_ERROR_CHECK( esp_wifi_start() );
	LOG_I(TAG,"vesync wifi hal init\n");
}
