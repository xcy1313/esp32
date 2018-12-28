/**
 * @file vesync_wifi.c
 * @brief vesync设备的WiFi连接等相关功能
 * @author WatWu
 * @date 2018-11-09
 */

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "vesync_log.h"
#include "vesync_wifi.h"
#include "vesync_bt_hal.h"

#include "cJSON.h"

//WiFi状态、网络状态事件标志组位定义，置位代表连接成功
#define EVENT_BIT_WIFI_STATUS			0X000001
#define EVENT_BIT_NETWORK_STATUS		0X000002
#define EVENT_BIT_NET_WIFI_DISCONNECT	0x000004

static const char *TAG = "vesync_wifi";

//网络事件标志组，从右往左，0位表示wifi状态，1位表示网络状态，置位代表连接成功
static EventGroupHandle_t s_network_event_group = NULL;

static vesync_wifi_cb s_vesync_wifi_callback = NULL;		//wifi连接状态回调函数指针

static vesync_wifi_status_e wifi_status = VESYNC_WIFI_INIT;

/**
 * @brief 扫描AP热点的回调函数
 * @param arg 		[扫描获取到的AP信息指针]
 * @param status 	[扫描结果]
 */
static void  blufi_wifi_list_packet(uint16_t ap_count,void  *arg)
{
	if(ap_count != 0){
		wifi_ap_record_t  *bss_link = (wifi_ap_record_t *)arg;
		int cnt = 0;
    
		cJSON *root = NULL;
		cJSON *wifiList = NULL;
		root = cJSON_CreateObject();
        
		if(NULL != root){
			cJSON_AddStringToObject(root, "uri", "/replyWifiList");
			cJSON_AddNumberToObject(root, "err", 1);
			cJSON_AddItemToObject(root, "wifiList", wifiList =  cJSON_CreateArray());

			if(NULL != wifiList){
				do{
					cJSON *wifiList_root = cJSON_CreateObject();
					if(NULL != wifiList_root){
						cJSON_AddStringToObject(wifiList_root, "SSID", (char *)(bss_link[cnt].ssid));
						cJSON_AddNumberToObject(wifiList_root, "AUTH", bss_link[cnt].authmode);
						cJSON_AddNumberToObject(wifiList_root, "RSSI", bss_link[cnt].rssi);
						cJSON_AddItemToArray(wifiList, wifiList_root);
					}
					cnt++;
				}while(cnt < ap_count);
			}
		}
        
		char* out = cJSON_PrintUnformatted(root);	//不带缩进格式
		ESP_LOGI(TAG, "Send to app : %s\r\n", out);

		cJSON_Delete(root);

		ESP_LOGI(TAG, "There are %d ap.\n", ap_count);

        vesync_blufi_notify((uint8_t *)out, strlen(out));

        free(out);
	}
}

/**
 * @brief 设置wifi工作模式
 * @param status 
 */
static void vesync_set_wifi_status(vesync_wifi_status_e new_status)
{
    if(new_status != wifi_status){
        ESP_LOGI(TAG, "wifi set mode is %d",new_status);
        wifi_status = new_status;
    }
}

vesync_wifi_status_e vesync_wifi_get_status(void)
{
    ESP_LOGI(TAG, "wifi get mode is %d",wifi_status);
    return wifi_status;
}

/**
 * @brief 连接WiFi的底层回调
 * @param status [WiFi连接状态，包括连接成功和连接掉线，以及连接失败时的错误原因]
 */
static void hal_connect_wifi_callback(vesync_wifi_status_e status)
{
	static vesync_wifi_status_e ostatus = VESYNC_WIFI_INIT;
	switch(status)						//驱动层根据需要使用部分WiFi状态
	{
		case VESYNC_WIFI_CONNECTING:
			ostatus = VESYNC_WIFI_CONNECTING;
			break;
		case VESYNC_WIFI_GOT_IP:
			xEventGroupSetBits(s_network_event_group, EVENT_BIT_NETWORK_STATUS);
			xEventGroupClearBits(s_network_event_group, EVENT_BIT_NET_WIFI_DISCONNECT);
			ostatus = VESYNC_WIFI_GOT_IP;
			break;
		case VESYNC_WIFI_LOST_IP:
			xEventGroupClearBits(s_network_event_group, EVENT_BIT_NETWORK_STATUS);
			xEventGroupSetBits(s_network_event_group, EVENT_BIT_NET_WIFI_DISCONNECT);
			ostatus = VESYNC_WIFI_LOST_IP;
			break;
		case VESYNC_WIFI_SCAN_DONE:{
				uint16_t apCount = 0;
				esp_wifi_scan_get_ap_num(&apCount);
				if (apCount == 0) {
					ESP_LOGI(TAG,"Nothing AP found");
					break;
				}
				wifi_ap_record_t *ap_list = (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * apCount);
				memset(ap_list ,0,sizeof(wifi_ap_record_t) * apCount);
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
				blufi_wifi_list_packet(apCount,blufi_ap_list);
			}
			break;
		default:
			break;
	}
	vesync_set_wifi_status(ostatus);	//用于本文件处理
	if(NULL != s_vesync_wifi_callback)	//驱动层调用上一层注册的回调函数，WiFi状态传递给上一层继续处理
	{
		s_vesync_wifi_callback(status);
	}
}

/**
 * @brief 阻塞等待网络连接成功
 * @param wait_time [等待时间，单位：毫秒]
 * @return int 		[等待结果，0 - 等待网络连接成功；-1 - 等待网络连接超时]
 */
int vesync_wait_network_connected(uint32_t	wait_time)
{
	int ret;
	//阻塞等待网络事件标志组网络连接标志置位
	ret = xEventGroupWaitBits(s_network_event_group,
	                          EVENT_BIT_NETWORK_STATUS,
	                          pdFALSE,
	                          pdTRUE,
	                          wait_time / portTICK_RATE_MS);
	if(ret & EVENT_BIT_NETWORK_STATUS)
		return 0;
	else
		return -1;
}

/**
 * @brief 阻塞等待网络断开成功
 * @param wait_time [等待时间，单位：毫秒]
 * @return int 		[等待结果，0 - 等待网络连接成功；-1 - 等待网络连接超时]
 */
int vesync_wait_network_disconnected(uint32_t	wait_time)
{
	int ret;
	//阻塞等待网络事件标志组网络连接标志置位
	ret = xEventGroupWaitBits(s_network_event_group,
	                          EVENT_BIT_NET_WIFI_DISCONNECT,
	                          pdFALSE,
	                          pdFALSE,
	                          wait_time / portTICK_RATE_MS);
	if(ret & EVENT_BIT_NET_WIFI_DISCONNECT)
		return 0;
	else
		return -1;
}

/**
 * @brief WIFI Drive层注册wifi事件回调
 * @param callback 
 */
static void vesync_driver_register_cb(vesync_wifi_cb callback)
{
	if(NULL != callback){
		s_vesync_wifi_callback = callback;
	}else{
		LOG_E(TAG, "driver cb register fail");
	}
}

/**
 * @brief 初始化wifi模块
 */
void vesync_init_wifi_module(vesync_wifi_cb callback)
{
	vesync_driver_register_cb(callback);
	vesync_hal_init_wifi_module(hal_connect_wifi_callback);

	s_network_event_group = xEventGroupCreate();
	
	if(NULL != callback){
		s_vesync_wifi_callback = callback;
	}
	LOG_I(TAG, "Init wifi module done.");
}

/**
 * @brief 设置设备为开放热点
 * @param ap_ssid [热点的名称]
 */
void vesync_setup_wifi_open_ap(char *ap_ssid)
{
	vesync_hal_setup_wifi_ap(ap_ssid, NULL, HAL_WIFI_2_4G_CHANNEL1, HAL_WIFI_AUTH_OPEN);
	LOG_I(TAG, "WiFi setup open ap, ssid : %s", ap_ssid);
}

/**
 * @brief 设置设备为开放热点和客户端共存
 * @param ap_ssid [热点的名称]
 */
void vesync_setup_wifi_open_ap_and_sta(char *ap_ssid)
{
	vesync_hal_setup_wifi_ap_and_sta(ap_ssid, NULL, HAL_WIFI_2_4G_CHANNEL1, HAL_WIFI_AUTH_OPEN);
	LOG_I(TAG, "WiFi setup open ap and sta, ssid : %s", ap_ssid);
}

/**
 * @brief 设置设备的wifi模式
 * @param target_mode [目标模式]
 * @return int8_t 	  [设置结果，0为成功]
 */
int8_t vesync_set_wifi_mode(uint8_t target_mode)
{
	int8_t ret;
	ret	= vesync_hal_set_wifi_mode(target_mode);
	return ret;
}

/**
 * @brief 获取当前wifi模式
 * @return uint8_t [当前wifi模式]
 */
uint8_t vesync_get_wifi_mode(void)
{
	uint8_t mode;
	mode = vesync_hal_get_wifi_mode();
	return mode;
}

/**
 * @brief 获取当前模式下的wifi配置
 * @param mode 
 * @return uint8_t 
 */
uint8_t vesync_get_wifi_config(wifi_interface_t interface,wifi_config_t *cfg){
	ESP_ERROR_CHECK(esp_wifi_get_config(interface, cfg));

	return 0;
}

/**
 * @brief 设备连接WiFi
 * @param wifi_ssid	[WiFi名称]
 * @param wifi_key 	[WiFi密码]
 * @param callback 	[WiFi连接回调函数]
 */
void vesync_connect_wifi(char *wifi_ssid, char *wifi_password, bool power_save)
{
	if(VESYNC_WIFI_GOT_IP == vesync_wifi_get_status()){	//如果之前设备是连接状态 需要主动断开
		ESP_LOGI(TAG, "wifi disconnect......");
		ESP_ERROR_CHECK(esp_wifi_disconnect());
		vesync_wait_network_disconnected(3000);
	}
	vesync_hal_connect_wifi(wifi_ssid, wifi_password, power_save);
	LOG_I(TAG, "Connect to ap ssid : %s, password : %s ", wifi_ssid, wifi_password);
}

/**
 * @brief 获取设备WiFi客户端模式下的mac地址字符串
 * @param mac_str_buffer [mac地址字符串缓存区，大小必须大于等于18字节]
 */
void vesync_get_wifi_sta_mac_string(char *mac_str_buffer)
{
	vesync_hal_get_mac_string(ESP_MAC_WIFI_STA, mac_str_buffer);
}

/**
 * @brief 开启wifi扫描
 * @return int 
 */
int vesync_scan_wifi_list_start(void)
{
	return vesync_hal_scan_wifi_list_start();
}

/**
 * @brief 停止wifi扫描
 * @return int 
 */
int vesync_scan_wifi_list_stop(void)
{
	return vesync_hal_scan_stop();
}
