#include "vesync_wifi.h"
#include "vesync_flash.h"
#include "vesync_public.h"
#include "esp_log.h"
#include "cJSON.h"

#include "esp_bt.h"
#include "esp_blufi_api.h"

static const char *TAG = "Vesync_WIFI";

static bool gl_sta_connected = false;
static uint8_t gl_sta_bssid[6];
static uint8_t gl_sta_ssid[32];
static int gl_sta_ssid_len;
device_info_t device_info;
wifi_status_t wifi_status;
static bool reconnect = true;
static uint8_t reconnect_cnt = 0;

static void vesync_set_wifi_status(wifi_status_t status);

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
 * @return esp_err_t esp_wifi_types.h
 */
static esp_err_t vesync_system_event_handler(void *ctx, system_event_t *event)
{
    wifi_mode_t mode;
    static wifi_status_t status = STATION_IDLE;
#if 1
    switch (event->event_id) {
        case SYSTEM_EVENT_STA_START:
            ESP_LOGI(TAG, "connectting..............\n");
            esp_wifi_connect();
            status = STATION_CONNECTING;
            break;
        case SYSTEM_EVENT_STA_GOT_IP: {
            esp_blufi_extra_info_t info;
            ESP_LOGI(TAG, "got ip:%s\n", ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
            xEventGroupSetBits(user_event_group, WIFI_CONNECTED_BIT);
            xEventGroupClearBits(user_event_group, DISCONNECTED_BIT);
            esp_wifi_get_mode(&mode);
            status = STATION_GOT_IP;
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
            if (reconnect){    //必须加重连超时机制
                if(reconnect_cnt++ >=5){
                    reconnect_cnt =0;
                    reconnect = false;
                    ESP_ERROR_CHECK(esp_wifi_disconnect());
                    ESP_LOGI(TAG, "reconnect fail !!!!");
                }else{
                    ESP_LOGI(TAG, "sta disconnect, reconnect...%d" ,reconnect_cnt);
                    esp_wifi_connect();
                }
            }else{
                reconnect_cnt = 0;
            }
            ESP_LOGI(TAG, "disconnect reason:%d",event->event_info.disconnected.reason);
            
            gl_sta_connected = false;
            memset(gl_sta_ssid, 0, 32);
            memset(gl_sta_bssid, 0, 6);
            gl_sta_ssid_len = 0;
            xEventGroupClearBits(user_event_group, WIFI_CONNECTED_BIT);
            xEventGroupSetBits(user_event_group, DISCONNECTED_BIT);
            status = STATION_DIS_CONNECTED;
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_DISCONNECTED");
            break;
        case SYSTEM_EVENT_STA_WPS_ER_PIN:
            ESP_LOGE(TAG, "SYSTEM_EVENT_STA_WPS_ER_PIN!!!!!!!!");
            break;
        case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_AUTHMODE_CHANGE");
            break;
        case SYSTEM_EVENT_STA_LOST_IP:
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_LOST_IP");
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
    vesync_set_wifi_status(status);    

    return ESP_OK;
}

/**
 * @brief 设置wifi工作模式
 * @param status 
 */
static void vesync_set_wifi_status(wifi_status_t new_status)
{
    uint8_t wifi_conn;

    if(new_status != wifi_status){
        ESP_LOGI(TAG, "wifi set mode is %d",new_status);
        switch(new_status){
            case STATION_GOT_IP:
                    wifi_conn = 2;
                    resend_cmd_bit |= RESEND_CMD_WIFI_STATUS_BIT;
                    uart_encode_send(MASTER_SET,CMD_WIFI_STATUS,(char *)&wifi_conn,sizeof(uint8_t),true);
                break;
            case STATION_CONNECTING:
                    wifi_conn = 1;
                    resend_cmd_bit |= RESEND_CMD_WIFI_STATUS_BIT;
                    uart_encode_send(MASTER_SET,CMD_WIFI_STATUS,(char *)&wifi_conn,sizeof(uint8_t),true);
                break;
            case STATION_DIS_CONNECTED:
                    wifi_conn = 0;
                    resend_cmd_bit |= RESEND_CMD_WIFI_STATUS_BIT;
                    uart_encode_send(MASTER_SET,CMD_WIFI_STATUS,(char *)&wifi_conn,sizeof(uint8_t),true);
                break;
            default:
                break;
        }
        wifi_status = new_status;
    }
}

wifi_status_t vesync_wifi_get_status(void)
{
    ESP_LOGI(TAG, "wifi get mode is %d",wifi_status);
    return wifi_status;
}


/**
 * @brief 配置station接口以备连接路由器
 * @param ssid 
 * @param pwd 
 */
void vesync_wifi_config(char *ssid ,char *pwd)
{
    static wifi_config_t wifi_config ={0};
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    memset(&wifi_config,NULL,sizeof(wifi_config_t));
    if(pwd == NULL){
        ESP_LOGE(TAG, "pwd is NULL");
        return;
    }    
    
    strcpy((char *)wifi_config.sta.ssid,(char *)ssid);
    strcpy((char *)wifi_config.sta.password,(char *)pwd);
    ESP_LOGI(TAG, "Setting WiFi configuration SSID [%s],pwd [%s]",ssid,pwd);

    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );

    ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
    ESP_ERROR_CHECK( esp_wifi_set_ps(WIFI_PS_MODEM));			//开启wifi省电模式;
}

void vesync_refresh_wifi(device_info_t *info){
    esp_err_t ret;
    wifi_config_t station_config;

    memset(&station_config,NULL,sizeof(wifi_config_t));
    uint8_t ostatus = vesync_wifi_get_status();
    strcpy((char*) station_config.sta.ssid, (char*)info->station_config.wifiSSID);
    strcpy((char*) station_config.sta.password, (char*)info->station_config.wifiPassword);

    ESP_LOGI(TAG, "ssid[%s] " ,station_config.sta.ssid);
    ESP_LOGI(TAG, "pwd[%s] "  ,station_config.sta.password);

    if(STATION_GOT_IP == ostatus){   //如果wifi之前已经连接
        reconnect = false;
        ESP_LOGI(TAG, "wifi disconnect......");
        ESP_ERROR_CHECK(esp_wifi_disconnect());
        xEventGroupWaitBits(user_event_group, DISCONNECTED_BIT, false, false, portTICK_RATE_MS);
    }
    ESP_LOGI(TAG, "wifi re-config");
    vesync_wifi_config((char*)station_config.sta.ssid,(char*)station_config.sta.password);

    ESP_LOGI(TAG, "wifi connect......");
    ESP_ERROR_CHECK( esp_wifi_connect() );
    reconnect = true;
    reconnect_cnt =0;
}
/**
 * @brief 
 */
void vesync_wifi_deinit(void)
{
    ESP_ERROR_CHECK(esp_wifi_stop());
}

/**
 * @brief 初始化wifi配置，启动tcpip适配器
 */
void vesync_wifi_init(void)
{
    memset(&device_info,NULL,sizeof(device_info_t));
    tcpip_adapter_init();
    ESP_ERROR_CHECK( esp_event_loop_init(vesync_system_event_handler, NULL) );

    if(vesync_flash_read_info(&device_info)){
        ESP_LOGI(TAG, "read flash config ok!!!!");
        vesync_wifi_config((char *)device_info.station_config.wifiSSID,(char *)device_info.station_config.wifiPassword);
    }else{
        ESP_LOGI(TAG, "first time use!!!!");
    }
    ESP_LOGI(TAG, "wifi init success");
}