#include "vesync_wifi.h"
#include "esp_log.h"

#include "esp_bt.h"
#include "esp_blufi_api.h"

static const char *TAG = "Vesync_WIFI";


EventGroupHandle_t user_event_group;

const int WIFI_CONNECTED_BIT = BIT0;
const int OTA_BIT = BIT1;

static bool gl_sta_connected = false;
static uint8_t gl_sta_bssid[6];
static uint8_t gl_sta_ssid[32];
static int gl_sta_ssid_len;

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
            ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&apCount, ap_list));
            esp_blufi_ap_record_t * blufi_ap_list = (esp_blufi_ap_record_t *)malloc(apCount * sizeof(esp_blufi_ap_record_t));
            if (!blufi_ap_list) {
                if (ap_list) {
                    free(ap_list);
                }
                ESP_LOGI(TAG, "malloc error, blufi_ap_list is NULL");
                break;
            }
            for (int i = 0; i < apCount; ++i)
            {
                blufi_ap_list[i].rssi = ap_list[i].rssi;
                memcpy(blufi_ap_list[i].ssid, ap_list[i].ssid, sizeof(ap_list[i].ssid));
            }
            esp_blufi_send_wifi_list(apCount, blufi_ap_list);
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

#define TEST_SSID "R6100-2.4G"
#define TEST_PWD  "12345678"

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
    ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    //ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );

    //ESP_ERROR_CHECK( esp_wifi_set_ps(WIFI_PS_MODEM));			//开启wifi省电模式;
    ESP_LOGI(TAG, "wifi init success");
}