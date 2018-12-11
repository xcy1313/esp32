
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h"
#include "cJSON.h"

#include "esp_blufi_api.h"
#include "esp_bt_defs.h"
#include "esp_gap_ble_api.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "vesync_blufi.h"
#include "vesync_bt.h"
#include "vesync_wifi.h"
#include "vesync_flash.h"

static const char *TAG = "Vesync_BLUFI";

#define BLUFI_INFO(fmt, ...)   ESP_LOGI(TAG, fmt, ##__VA_ARGS__) 
#define BLUFI_ERROR(fmt, ...)  ESP_LOGE(TAG, fmt, ##__VA_ARGS__) 

static void vesync_blufi_event_callback(esp_blufi_cb_event_t event, esp_blufi_cb_param_t *param);

#define BLUFI_DEVICE_NAME      "BLUFI_DEVICE"
static uint8_t vesync_blufi_service_uuid128[32] = {
    /* LSB <--------------------------------------------------------------------------------> MSB */
    //first uuid, 16bit, [12],[13] is the value
    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00,
};

//static uint8_t test_manufacturer[TEST_MANUFACTURER_DATA_LEN] =  {0x12, 0x23, 0x45, 0x56};
static esp_ble_adv_data_t vesync_blufi_adv_data = {
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = true,
    .min_interval = 0x100,
    .max_interval = 0x100,
    .appearance = 0x00,
    .manufacturer_len = 0,
    .p_manufacturer_data =  NULL,
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = 16,
    .p_service_uuid = vesync_blufi_service_uuid128,
    .flag = 0x6,
};

static esp_ble_adv_params_t vesync_blufi_adv_data_adv_params = {
    .adv_int_min        = 0x100,
    .adv_int_max        = 0x100,
    .adv_type           = ADV_TYPE_IND,
    .own_addr_type      = BLE_ADDR_TYPE_PUBLIC,
    //.peer_addr            =
    //.peer_addr_type       =
    .channel_map        = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

static wifi_config_t sta_config;
static wifi_config_t ap_config;

/* store the station info for send back to phone */
static bool gl_sta_connected = false;
static uint8_t gl_sta_bssid[6];
static uint8_t gl_sta_ssid[32];
static int gl_sta_ssid_len;

/* connect infor*/
static uint8_t server_if;
static uint16_t conn_id;

vesync_blufi_param_t vesync_blufi_param;

static esp_blufi_callbacks_t vesync_blufi_callbacks = {
    .event_cb = vesync_blufi_event_callback,
    .negotiate_data_handler = blufi_dh_negotiate_data_handler,
    .encrypt_func = blufi_aes_encrypt,
    .decrypt_func = blufi_aes_decrypt,
    .checksum_func = blufi_crc_checksum,
};


/**
 * [Handle_ConfigNetJson  解析处理APP通过ble发来的配网json数据]
 * @param  json [配网json数据]
 * @return    	[无]
 */
static void Handle_ConfigNetJson(device_info_t *info,cJSON *json)
{
	cJSON *root = json;
    uint32_t mask =0;

	cJSON *pid = cJSON_GetObjectItemCaseSensitive(root, "pid");
	if(true == cJSON_IsString(pid)){
		strcpy((char*)info->mqtt_config.pid, pid->valuestring);
		BLUFI_INFO("pid : %s\r\n", info->mqtt_config.pid);
        mask |=0x1;
	}

	cJSON *configKey = cJSON_GetObjectItemCaseSensitive(root, "configKey");
	if(true == cJSON_IsString(configKey)){
        strcpy((char*)info->mqtt_config.configKey, configKey->valuestring);
		BLUFI_INFO("configKey : %s\r\n", info->mqtt_config.configKey);
        mask |=0x2;
	}

	cJSON *serverDN = cJSON_GetObjectItemCaseSensitive(root, "serverDN");
	if(true == cJSON_IsString(serverDN)){
		char *str = strstr(serverDN->valuestring, ":");
		if(NULL != str){
			strncpy((char*)info->mqtt_config.serverDN, serverDN->valuestring, str - serverDN->valuestring);
			info->mqtt_config.serverDN[str - serverDN->valuestring] = '\0';

			int port = atoi(str + 1);   
			info->mqtt_config.mqtt_port = port;
			BLUFI_INFO("mqtt port : %d\r\n", info->mqtt_config.mqtt_port);
		}else{
			strcpy((char*)info->mqtt_config.serverDN, serverDN->valuestring);
		}
        mask |=0x4;
		BLUFI_INFO("serverDN : %s\r\n", info->mqtt_config.serverDN);
	}

	cJSON *serverIP = cJSON_GetObjectItemCaseSensitive(root, "serverIP");
	if(true == cJSON_IsString(serverIP)){
		char *str = strstr(serverIP->valuestring, ":");
		if(NULL != str){
			strncpy((char*)info->mqtt_config.serverIP, serverIP->valuestring, str - serverIP->valuestring);
		}else{
			strcpy((char*)info->mqtt_config.serverIP, serverIP->valuestring);
		}
        mask |=0x8;
		BLUFI_INFO("serverIP : %s\r\n", (char*)info->mqtt_config.serverIP);
	}

	cJSON *wifiSSID = cJSON_GetObjectItemCaseSensitive(root, "wifiSSID");
	if(true == cJSON_IsString(wifiSSID)){
		strcpy((char*)info->station_config.wifiSSID, wifiSSID->valuestring);
		BLUFI_INFO("wifiSSID : %s\r\n", (char*)info->station_config.wifiSSID);
        mask |=0x10;
	}

	cJSON *wifiPassword = cJSON_GetObjectItemCaseSensitive(root, "wifiPassword");
	if(true == cJSON_IsString(wifiPassword)){
		strcpy((char*)info->station_config.wifiPassword, wifiPassword->valuestring);
		BLUFI_INFO("wifiPassword : %s\r\n", (char*)info->station_config.wifiPassword);
        mask |=0x20;
	}

	cJSON *wifiStaticIP = cJSON_GetObjectItemCaseSensitive(root, "wifiStaticIP");
	if(true == cJSON_IsString(wifiStaticIP)){
		strcpy((char*)info->station_config.wifiStaticIP, wifiStaticIP->valuestring);
		BLUFI_INFO("wifiStaticIP : %s\r\n", (char*)info->station_config.wifiStaticIP);
        mask |=0x40;
	}

	cJSON *wifiGateway = cJSON_GetObjectItemCaseSensitive(root, "wifiGateway");
	if(true == cJSON_IsString(wifiGateway)){
		strcpy((char*)info->station_config.wifiGateway, wifiGateway->valuestring);
		BLUFI_INFO("wifiGateway : %s\r\n", (char*)info->station_config.wifiGateway);
        mask |=0x80;
	}

	cJSON *wifiDNS = cJSON_GetObjectItemCaseSensitive(root, "wifiDNS");
	if(true == cJSON_IsString(wifiDNS)){
		strcpy((char*)info->station_config.wifiDNS, wifiDNS->valuestring);
		BLUFI_INFO("wifiDNS : %s\r\n", (char*)info->station_config.wifiDNS);
        mask |=0x100;
	}
    if(mask & 0x30){
        vesync_refresh_wifi(info);
    }
    if((mask & 0x1ff) == 0x1ff){
        vesync_flash_write_info(info); 
    }
    //vesync_flash_read_info();
}

#define DEVICE_TYPE     "1"
#define FIRM_VERSION    "2"
/**
 * @brief 回复APP设备的固件信息
 */
static void vesync_reply_firmware_information(void)
{
	uint8_t buffer[128];
	uint8_t buflen = 0;
	buflen = sprintf((char *)buffer, "{\"uri\":\"/replyFirmware\",\"deviceType\":\"%s\",\"firmVersion\":\"%s\"}", DEVICE_TYPE, FIRM_VERSION);

	if(esp_blufi_send_custom_data(buffer, buflen) != ESP_OK){
		BLUFI_INFO("Send wifi list to app failed !\n");
	}
	BLUFI_INFO("TCP send to APP : %s \r\n", buffer);
}


/**
 * @brief 启动扫描WiFi列表
 * @return int [扫描结果]
 */
static void vesync_scan_wifi_list(void)
{
    int ret;
    wifi_scan_config_t scanConf = {
                .ssid = NULL,
                .bssid = NULL,
                .channel = 0,
                .show_hidden = false
            };
    BLUFI_INFO("blufi wifi scan start\n");

    ret = esp_wifi_scan_start(&scanConf, true);
    if(ESP_OK != ret){
        ESP_LOGE(TAG ,"blufi wifi scan error :%d\n" ,ret);
        uint8_t buffer[64];
        sprintf((char *)buffer, "{\"uri\":\"/replyWifiList\",\"result\":-1}");
        if(esp_blufi_send_custom_data(buffer, sizeof(buffer)-1) != ESP_OK){
            BLUFI_INFO("Send wifi list reply to app failed !\n");
        }
    }
}

static void vesync_scan_stop(void)
{
    BLUFI_INFO("blufi wifi scan stop\n");
    ESP_ERROR_CHECK(esp_wifi_scan_stop());
}

/**
 * [handle_tcp_message  解析APP端通过本地网络接口直接发送过来的数据]
 * @param  data   [网络数据]
 * @param  length [数据长度]
 * @return        [无]
 */
void handle_blufi_custom_message(device_info_t *info, const char *data, int length)
{
	cJSON *root = cJSON_Parse(data);
	if(NULL == root){
		BLUFI_INFO("Parse cjson error !\r\n");
		return;
	}

	//vesync_printf_cjson(root);				//json标准格式，带缩进
	BLUFI_INFO("Handler custom message !\r\n");

	//获取uri，判断是否为配网的json数据
	cJSON *uri = cJSON_GetObjectItemCaseSensitive(root, "uri");

	if(true == cJSON_IsString(uri)){
		BLUFI_INFO("Found uri !");
		if(!strcmp(uri->valuestring, "/beginConfigRequest")){		//开始配网
			//确认是配网信息
			Handle_ConfigNetJson(info,root); 								//解析处理json数据
		}else if(!strcmp(uri->valuestring, "/cancelConfig")){			//取消配网
			vesync_scan_stop();
		}else if(!strcmp(uri->valuestring, "/queryFirmware")){
			vesync_reply_firmware_information();
		}else if(!strcmp(uri->valuestring, "/queryWifiList")){
			vesync_scan_wifi_list();
		}else{
			//VESYNC_LOG(USER_LOG_ERROR, "Config parameter error !\r\n");
			//confignet_upload_realtime_log(ERR_CONFIG_URI_ERROR, "CONFIG_URI_ERROR");
		}
	}

	cJSON_Delete(root);									//务必记得释放资源！
}

static void vesync_blufi_event_callback(esp_blufi_cb_event_t event, esp_blufi_cb_param_t *param)
{
    switch (event){
        case ESP_BLUFI_EVENT_INIT_FINISH:
            BLUFI_INFO("BLUFI init finish\n");
        #ifdef ENABLE_BLUFI_ADVERTISE
            esp_ble_gap_set_device_name(BLUFI_DEVICE_NAME);   //受app限制 需使用此广播名用来调试blufi 
            esp_ble_gap_config_adv_data(&vesync_blufi_adv_data);
        #endif
            break;
        case ESP_BLUFI_EVENT_DEINIT_FINISH:
            BLUFI_INFO("BLUFI deinit finish\n");
            break;
        case ESP_BLUFI_EVENT_BLE_CONNECT:
            BLUFI_INFO("BLUFI ble connect\n");
            server_if = param->connect.server_if;
            conn_id = param->connect.conn_id;
            blufi_security_init();
            break;
        case ESP_BLUFI_EVENT_BLE_DISCONNECT:
            BLUFI_INFO("BLUFI ble disconnect\n");
            blufi_security_deinit();
            break;
        case ESP_BLUFI_EVENT_SET_WIFI_OPMODE:
            BLUFI_INFO("BLUFI Set WIFI opmode %d\n", param->wifi_mode.op_mode);
            ESP_ERROR_CHECK( esp_wifi_set_mode(param->wifi_mode.op_mode) );
            break;
        case ESP_BLUFI_EVENT_REQ_CONNECT_TO_AP:
            BLUFI_INFO("BLUFI requset wifi connect to AP\n");
            /* there is no wifi callback when the device has already connected to this wifi
            so disconnect wifi before connection.
            */
            esp_wifi_disconnect();
            esp_wifi_connect();
            break;
        case ESP_BLUFI_EVENT_REQ_DISCONNECT_FROM_AP:
            BLUFI_INFO("BLUFI requset wifi disconnect from AP\n");
            esp_wifi_disconnect();
            break;
        case ESP_BLUFI_EVENT_REPORT_ERROR:
            BLUFI_ERROR("BLUFI report error, error code %d\n", param->report_error.state);
            esp_blufi_send_error_info(param->report_error.state);
            break;
        case ESP_BLUFI_EVENT_GET_WIFI_STATUS: {
            wifi_mode_t mode;
            esp_blufi_extra_info_t info;

            esp_wifi_get_mode(&mode);

            if (gl_sta_connected ){  
                memset(&info, 0, sizeof(esp_blufi_extra_info_t));
                memcpy(info.sta_bssid, gl_sta_bssid, 6);
                info.sta_bssid_set = true;
                info.sta_ssid = gl_sta_ssid;
                info.sta_ssid_len = gl_sta_ssid_len;
                esp_blufi_send_wifi_conn_report(mode, ESP_BLUFI_STA_CONN_SUCCESS, 0, &info);
            } else {
                esp_blufi_send_wifi_conn_report(mode, ESP_BLUFI_STA_CONN_FAIL, 0, NULL);
            }
            BLUFI_INFO("BLUFI get wifi status from AP\n");

            break;
        }
        case ESP_BLUFI_EVENT_RECV_SLAVE_DISCONNECT_BLE:
            BLUFI_INFO("blufi close a gatt connection");
            esp_blufi_close(server_if, conn_id);
            break;
        case ESP_BLUFI_EVENT_DEAUTHENTICATE_STA:
            /* TODO */
            break;
        case ESP_BLUFI_EVENT_RECV_STA_BSSID:
            memcpy(sta_config.sta.bssid, param->sta_bssid.bssid, 6);
            sta_config.sta.bssid_set = 1;
            esp_wifi_set_config(WIFI_IF_STA, &sta_config);
            BLUFI_INFO("Recv STA BSSID %s\n", sta_config.sta.ssid);
            break;
        case ESP_BLUFI_EVENT_RECV_STA_SSID:
            strncpy((char *)sta_config.sta.ssid, (char *)param->sta_ssid.ssid, param->sta_ssid.ssid_len);
            sta_config.sta.ssid[param->sta_ssid.ssid_len] = '\0';
            esp_wifi_set_config(WIFI_IF_STA, &sta_config);
            BLUFI_INFO("Recv STA SSID %s\n", sta_config.sta.ssid);
            break;
        case ESP_BLUFI_EVENT_RECV_STA_PASSWD:
            strncpy((char *)sta_config.sta.password, (char *)param->sta_passwd.passwd, param->sta_passwd.passwd_len);
            sta_config.sta.password[param->sta_passwd.passwd_len] = '\0';
            esp_wifi_set_config(WIFI_IF_STA, &sta_config);
            BLUFI_INFO("Recv STA PASSWORD %s\n", sta_config.sta.password);
            break;
        case ESP_BLUFI_EVENT_RECV_SOFTAP_SSID:
            strncpy((char *)ap_config.ap.ssid, (char *)param->softap_ssid.ssid, param->softap_ssid.ssid_len);
            ap_config.ap.ssid[param->softap_ssid.ssid_len] = '\0';
            ap_config.ap.ssid_len = param->softap_ssid.ssid_len;
            esp_wifi_set_config(WIFI_IF_AP, &ap_config);
            BLUFI_INFO("Recv SOFTAP SSID %s, ssid len %d\n", ap_config.ap.ssid, ap_config.ap.ssid_len);
            break;
        case ESP_BLUFI_EVENT_RECV_SOFTAP_PASSWD:
            strncpy((char *)ap_config.ap.password, (char *)param->softap_passwd.passwd, param->softap_passwd.passwd_len);
            ap_config.ap.password[param->softap_passwd.passwd_len] = '\0';
            esp_wifi_set_config(WIFI_IF_AP, &ap_config);
            BLUFI_INFO("Recv SOFTAP PASSWORD %s len = %d\n", ap_config.ap.password, param->softap_passwd.passwd_len);
            break;
        case ESP_BLUFI_EVENT_RECV_SOFTAP_MAX_CONN_NUM:
            if (param->softap_max_conn_num.max_conn_num > 4) {
                return;
            }
            ap_config.ap.max_connection = param->softap_max_conn_num.max_conn_num;
            esp_wifi_set_config(WIFI_IF_AP, &ap_config);
            BLUFI_INFO("Recv SOFTAP MAX CONN NUM %d\n", ap_config.ap.max_connection);
            break;
        case ESP_BLUFI_EVENT_RECV_SOFTAP_AUTH_MODE:
            if (param->softap_auth_mode.auth_mode >= WIFI_AUTH_MAX) {
                return;
            }
            ap_config.ap.authmode = param->softap_auth_mode.auth_mode;
            esp_wifi_set_config(WIFI_IF_AP, &ap_config);
            BLUFI_INFO("Recv SOFTAP AUTH MODE %d\n", ap_config.ap.authmode);
            break;
        case ESP_BLUFI_EVENT_RECV_SOFTAP_CHANNEL:
            if (param->softap_channel.channel > 13) {
                return;
            }
            ap_config.ap.channel = param->softap_channel.channel;
            esp_wifi_set_config(WIFI_IF_AP, &ap_config);
            BLUFI_INFO("Recv SOFTAP CHANNEL %d\n", ap_config.ap.channel);
            break;
        case ESP_BLUFI_EVENT_GET_WIFI_LIST:{
            vesync_scan_wifi_list();
#if 0            
            wifi_scan_config_t scanConf = {
                .ssid = NULL,
                .bssid = NULL,
                .channel = 0,
                .show_hidden = false
            };
            BLUFI_INFO("blufi wifi scan start\n");
            ESP_ERROR_CHECK(esp_wifi_scan_start(&scanConf, true));
#endif            
            break;
        }
        case ESP_BLUFI_EVENT_RECV_CUSTOM_DATA:  //手机下发custom data 
            handle_blufi_custom_message(&device_info,(char *)param->custom_data.data, param->custom_data.data_len);
            BLUFI_INFO("Recv Custom Data %d\n", param->custom_data.data_len);
            //esp_log_buffer_hex("Custom Data", param->custom_data.data, param->custom_data.data_len);
            esp_blufi_send_custom_data(param->custom_data.data, param->custom_data.data_len);   //设备转发数据表示应答
            break;
        case ESP_BLUFI_EVENT_RECV_USERNAME:
            /* Not handle currently */
            break;
        case ESP_BLUFI_EVENT_RECV_CA_CERT:
            /* Not handle currently */
            break;
        case ESP_BLUFI_EVENT_RECV_CLIENT_CERT:
            /* Not handle currently */
            break;
        case ESP_BLUFI_EVENT_RECV_SERVER_CERT:
            /* Not handle currently */
            break;
        case ESP_BLUFI_EVENT_RECV_CLIENT_PRIV_KEY:
            /* Not handle currently */
            break;;
        case ESP_BLUFI_EVENT_RECV_SERVER_PRIV_KEY:
            /* Not handle currently */
            break;
        default:
            break;
    }
}

void vesync_blufi_init(void)
{
    esp_err_t ret;
    esp_bt_controller_status_t bt_status;
    bt_status = esp_bt_controller_get_status();

    BLUFI_INFO("BLUFI status %d\n", bt_status);

    if( bt_status == ESP_BT_CONTROLLER_STATUS_IDLE){    //判断蓝牙当前状态
        ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

        esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
        ret = esp_bt_controller_init(&bt_cfg);
        if (ret) {
            BLUFI_ERROR("%s initialize bt controller failed: %s\n", __func__, esp_err_to_name(ret));
        }

        ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
        if (ret) {
            BLUFI_ERROR("%s enable bt controller failed: %s\n", __func__, esp_err_to_name(ret));
            return;
        }

        ret = esp_bluedroid_init();
        if (ret) {
            BLUFI_ERROR("%s init bluedroid failed: %s\n", __func__, esp_err_to_name(ret));
            return;
        }

        ret = esp_bluedroid_enable();
        if (ret) {
            BLUFI_ERROR("%s init bluedroid failed: %s\n", __func__, esp_err_to_name(ret));
            return;
        }

        BLUFI_INFO("BD ADDR: "ESP_BD_ADDR_STR"\n", ESP_BD_ADDR_HEX(esp_bt_dev_get_address()));

        BLUFI_INFO("BLUFI VERSION %04x\n", esp_blufi_get_version());

        ret = esp_ble_gap_register_callback(vesync_bt_gap_event_handler);
        if(ret){
            BLUFI_ERROR("%s gap register failed, error code = %x\n", __func__, ret);
            return;
        }
    }else if(bt_status == ESP_BT_CONTROLLER_STATUS_ENABLED ){
        ret = esp_blufi_register_callbacks(&vesync_blufi_callbacks);
        if(ret){
            BLUFI_ERROR("%s blufi register failed, error code = %x\n", __func__, ret);
            return;
        }
        esp_blufi_profile_init();
    }
}
