#include "vesync_wifi.h"
#include "vesync_flash.h"
#include "esp_log.h"
#include "cJSON.h"

#include "esp_bt.h"
#include "esp_blufi_api.h"

static const char *TAG = "Vesync_WIFI";


EventGroupHandle_t user_event_group;

const int WIFI_CONNECTED_BIT = BIT0;
const int OTA_BIT = BIT1;
const int FLASH_WRITE_BIT = BIT2;

static bool gl_sta_connected = false;
static uint8_t gl_sta_bssid[6];
static uint8_t gl_sta_ssid[32];
static int gl_sta_ssid_len;
device_info_t device_info;

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
			cJSON_AddNumberToObject(root, "result", 0);
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

        esp_blufi_send_custom_data((uint8_t *)out, strlen(out));

        free(out);
	}
}

/**
 * @brief 
 * @param ctx 
 * @param event 
 * @return esp_err_t 
 */
static esp_err_t vesync_wifi_event_handler(void *ctx, system_event_t *event)
{
    wifi_mode_t mode;
#if 1
    switch (event->event_id) {
        case SYSTEM_EVENT_STA_START:
            ESP_LOGI(TAG, "connectting..............\n");
            esp_wifi_connect();
            break;
        case SYSTEM_EVENT_STA_GOT_IP: {
            esp_blufi_extra_info_t info;
            ESP_LOGI(TAG, "got ip:%s\n", ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
            xEventGroupSetBits(user_event_group, WIFI_CONNECTED_BIT);
            esp_wifi_get_mode(&mode);
    #if 1
            memset(&info, 0, sizeof(esp_blufi_extra_info_t));
            memcpy(info.sta_bssid, gl_sta_bssid, 6);
            info.sta_bssid_set = true;
            info.sta_ssid = gl_sta_ssid;
            info.sta_ssid_len = gl_sta_ssid_len;
            esp_blufi_send_wifi_conn_report(mode, ESP_BLUFI_STA_CONN_SUCCESS, 0, &info);
    #endif        
            break;
        }
        case SYSTEM_EVENT_STA_CONNECTED:
            gl_sta_connected = true;
            memcpy(gl_sta_bssid, event->event_info.connected.bssid, 6);
            memcpy(gl_sta_ssid, event->event_info.connected.ssid, event->event_info.connected.ssid_len);
            gl_sta_ssid_len = event->event_info.connected.ssid_len;
            break; 
        case SYSTEM_EVENT_STA_DISCONNECTED:
            /* This is a workaround as ESP32 WiFi libs don't currently
            auto-reassociate. */
            gl_sta_connected = false;
            memset(gl_sta_ssid, 0, 32);
            memset(gl_sta_bssid, 0, 6);
            gl_sta_ssid_len = 0;
            esp_wifi_connect();
            xEventGroupClearBits(user_event_group, WIFI_CONNECTED_BIT);
            break;
        case SYSTEM_EVENT_AP_START:
            esp_wifi_get_mode(&mode);

            /* TODO: get config or information of softap, then set to report extra_info */
            if (gl_sta_connected) {  
                esp_blufi_send_wifi_conn_report(mode, ESP_BLUFI_STA_CONN_SUCCESS, 0, NULL);
            } else {
                esp_blufi_send_wifi_conn_report(mode, ESP_BLUFI_STA_CONN_FAIL, 0, NULL);
            }
            break;
        case SYSTEM_EVENT_SCAN_DONE: {
            uint16_t apCount = 0;
            esp_wifi_scan_get_ap_num(&apCount);
            if (apCount == 0) {
                ESP_LOGI(TAG,"Nothing AP found");
                break;
            }
            wifi_ap_record_t *ap_list = (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * apCount);
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
            esp_blufi_send_wifi_list(apCount, blufi_ap_list);
            blufi_wifi_list_packet(apCount,blufi_ap_list);
            esp_wifi_scan_stop();
            free(ap_list);
            free(blufi_ap_list);
            break;
        }
        default:
            break;
    }
#endif    
    return ESP_OK;
}

#define TEST_SSID "R6100"
#define TEST_PWD  "123451231987"

void vesync_wifi_init(void)
{
    tcpip_adapter_init();
    user_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK( esp_event_loop_init(vesync_wifi_event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = TEST_SSID,
            .password = TEST_PWD,
        },
    };

    //device_info = vesync_flash_read_info();

    //printf("\r\n");
    //for(uint8_t i=0;i<sizeof(device_info.station_config.wifiPassword);i++){
    //    printf("%d",device_info.station_config.wifiPassword[i]);
    //}
    //printf("\r\n");

    //strcpy((char *)wifi_config.sta.ssid,(char *)device_info.station_config.wifiSSID);
    //strcpy((char *)wifi_config.sta.password,(char *)device_info.station_config.wifiPassword);
    //ESP_LOGI(TAG, "Setting WiFi configuration SSID %s,pwd %s",device_info.station_config.wifiSSID,wifi_config.sta.password);

    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    //ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );

    //xEventGroupWaitBits(user_event_group, WIFI_CONNECTED_BIT, false, true, portMAX_DELAY);
    //ESP_ERROR_CHECK( esp_wifi_set_ps(WIFI_PS_MODEM));			//开启wifi省电模式;
    ESP_LOGI(TAG, "wifi init success");
}