/**
 * @file vesync_bt_hal.c
 * @brief 蓝牙hal配置ble服务属性接口
 * @author Jiven 
 * @date 2018-12-14
 */
#include "vesync_bt_hal.h"
#include "vesync_wifi.h"
#include "vesync_flash.h"
#include "vesync_freertos_timer.h"

#include "etekcity_bt_prase.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_main.h"
#include "esp_bt.h"
#include "esp_gatt_common_api.h"
#include "esp_blufi_api.h"
#include "blufi_security.h"
#include "vesync_build_cfg.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp_log.h"
#include "cJSON.h"

#include "vesync_build_cfg.h"

#define GATTS_TABLE_TAG "Vesync_BT"

#define APP_ADV_INTERVAL_MIN            0x20                          /**< The advertising interval (in units of 0.625 ms. This value corresponds to 300 ms). */
#define APP_ADV_INTERVAL_MAX            0x40                          /**< The advertising interval (in units of 0.625 ms. This value corresponds to 300 ms). */
#define TX_POWER                        ESP_PWR_LVL_N0                /* +3dbm*/
#define APP_ADV_TIMEOUT_IN_SECONDS      ADVER_TIME_OUT                /**< The advertising timeout (in units of seconds). */

#define PROFILE_NUM                 1
#define PROFILE_APP_IDX             0
#define ESP_APP_ID                  0x55
#define SVC_INST_ID                 0

#define GATTS_DEMO_CHAR_VAL_LEN_MAX 500
#define PREPARE_BUF_MAX_SIZE        1024
#define CHAR_DECLARATION_SIZE       (sizeof(uint8_t))

static vesync_bt_status_cb_t  vesync_bt_connect_status_callback = NULL;		//蓝牙连接状态回调函数指针
static void vesync_set_bt_status(BT_STATUS_T new_status);
static BT_STATUS_T vesync_get_bt_status(void);
bt_recv_cb_t m_bt_handler;

uint16_t heart_rate_handle_table[HRS_IDX_NB];

static uint16_t ble_conn_id = 0xffff;
static esp_gatt_if_t ble_gatts_if;

static BT_STATUS_T bt_status = BT_INIT;

static TimerHandle_t advertise_timer = NULL;
static TimerHandle_t net_config_timer = NULL;

static xQueueHandle hal_bt_receive_queue = NULL;

net_info_t net_info;    //配网信息
product_config_t product_config;//产测配置

typedef struct{
    uint8_t  len;
    uint8_t  buff[QUEUE_LEN];
}rx_frame_t;

typedef struct{
    uint8_t                 *prepare_buf;
    int                     prepare_len;
} prepare_type_env_t;

static prepare_type_env_t prepare_write_env;

static void gatts_profile_event_handler(esp_gatts_cb_event_t event,esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
static void vesync_blufi_event_handler(esp_blufi_cb_event_t event, esp_blufi_cb_param_t *param);

esp_bd_addr_t remote_device_addr;

static char advertise_name[30] = {0};
static uint8_t adver_manufacturer[] ={0xd0,0x6,0x1,0x36,0x35,0x34,0x33,0x32,0x31,0xe0,0xa0};

/* The length of adv data must be less than 31 bytes */
static esp_ble_adv_data_t adv_data = {
    .set_scan_rsp        = false,
    .include_name        = false,
    .include_txpower     = false,
    .appearance          = 0x00,
    .manufacturer_len    = sizeof(adver_manufacturer),
    .p_manufacturer_data = adver_manufacturer,
    .service_data_len    = 0,
    .p_service_data      = NULL,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

// scan response data
static esp_ble_adv_data_t scan_rsp_data = {
    .set_scan_rsp        = true,
    .include_name        = true,
};

static esp_ble_adv_params_t adv_params = {
    .adv_int_min         = APP_ADV_INTERVAL_MIN,
    .adv_int_max         = APP_ADV_INTERVAL_MAX,
    .adv_type            = ADV_TYPE_IND,
    .own_addr_type       = BLE_ADDR_TYPE_PUBLIC,
    .channel_map         = ADV_CHNL_ALL,
    .adv_filter_policy   = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

struct gatts_profile_inst{
    esp_gatts_cb_t gatts_cb;
    uint16_t gatts_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t service_handle;
    esp_gatt_srvc_id_t service_id;
    uint16_t char_handle;
    esp_bt_uuid_t char_uuid;
    esp_gatt_perm_t perm;
    esp_gatt_char_prop_t property;
    uint16_t descr_handle;
    esp_bt_uuid_t descr_uuid;
};

static esp_blufi_callbacks_t vesync_blufi_callbacks = {
    .event_cb = vesync_blufi_event_handler,
    .negotiate_data_handler = blufi_dh_negotiate_data_handler,
    .encrypt_func = blufi_aes_encrypt,
    .decrypt_func = blufi_aes_decrypt,
    .checksum_func = blufi_crc_checksum,
};

static struct gatts_profile_inst heart_rate_profile_tab[PROFILE_NUM] = {
    [PROFILE_APP_IDX] = {
        .gatts_cb = gatts_profile_event_handler,
        .gatts_if = ESP_GATT_IF_NONE,       /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
    },
};

/* user Service characteristic*/
static const uint16_t GATTS_SERVICE_PRIMARY_UUID   = 0xFFF0;
static const uint16_t GATTS_CHAR_NOTIFY_UUID       = 0xFFF1;
static const uint16_t GATTS_CHAR_WRITE_UUID        = 0xFFF2;

/* device information characteristic */
static const uint16_t GATTS_SYSTEM_ID_UUID         = ESP_GATT_UUID_SYSTEM_ID; 
static const uint16_t GATTS_MODEL_NUMBER_UUID      = ESP_GATT_UUID_MODEL_NUMBER_STR;
static const uint16_t GATTS_SERIAL_NUMBER_UUID     = ESP_GATT_UUID_SERIAL_NUMBER_STR;
static const uint16_t GATTS_FW_VERSION_UUID        = ESP_GATT_UUID_FW_VERSION_STR;
static const uint16_t GATTS_HW_VERSION_UUID        = ESP_GATT_UUID_HW_VERSION_STR;
static const uint16_t GATTS_SW_VERSION_UUID        = ESP_GATT_UUID_SW_VERSION_STR;
static const uint16_t GATTS_MANU_NAME_UUID         = ESP_GATT_UUID_MANU_NAME;
static const uint16_t GATTS_IEEE_DATA_UUID         = ESP_GATT_UUID_IEEE_DATA;
static const uint16_t GATTS_PNP_ID_UUID            = ESP_GATT_UUID_PNP_ID;

static const uint16_t primary_service_uuid         = ESP_GATT_UUID_PRI_SERVICE; 
static const uint16_t character_declaration_uuid   = ESP_GATT_UUID_CHAR_DECLARE;
static const uint16_t character_client_config_uuid = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
static const uint8_t char_prop_write               = ESP_GATT_CHAR_PROP_BIT_WRITE|ESP_GATT_CHAR_PROP_BIT_WRITE_NR;
static const uint8_t char_prop_notify              = ESP_GATT_CHAR_PROP_BIT_NOTIFY;
static const uint8_t char_prop_read                = ESP_GATT_CHAR_PROP_BIT_READ;
static const uint8_t char_prop_read_notify         = ESP_GATT_CHAR_PROP_BIT_READ|ESP_GATT_CHAR_PROP_BIT_NOTIFY;
static const uint8_t heart_measurement_ccc[2]      = {0x00, 0x00};
static const uint8_t char_value[4]                 = {0x06, 0xd0};
static const uint8_t systemId_value[2]             = {0x01, 0x10};

///device Service
static const uint16_t device_svc = ESP_GATT_UUID_DEVICE_INFO_SVC;
static const uint16_t bat_lev_uuid = ESP_GATT_UUID_BATTERY_LEVEL;
static const uint8_t   bat_lev_ccc[2] ={ 0x00, 0x00};
static const uint16_t char_format_uuid = ESP_GATT_UUID_CHAR_PRESENT_FORMAT;
static uint8_t fw_version[5] = {0x30,0x2e,0x30,0x2e,0x31};
static uint8_t hw_version[5] = {0x30,0x2e,0x30,0x2e,0x31};
static uint8_t sw_version[] = "R0000V0001";//{0x31,0x2e,0x31,0x2e,0x31};

static const esp_gatts_attr_db_t device_info_att_db[DEV_DEV_NB] =
{
    // Battary Service Declaration
    [IDX_DEV_SVC]               =  {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&primary_service_uuid, ESP_GATT_PERM_READ,
                                            sizeof(uint16_t), sizeof(device_svc), (uint8_t *)&device_svc}},

    [IDX_DEV_FW_VERSION_CHAR]   = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
                                                   CHAR_DECLARATION_SIZE,CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read}},
    [IDX_DEV_FW_VERSION_VAL]    = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_FW_VERSION_UUID, ESP_GATT_PERM_READ,
                                                                sizeof(fw_version),sizeof(fw_version), fw_version}},

    // hw version Characteristic Declaration
    [IDX_DEV_HW_VERSION_CHAR]   = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
                                                   CHAR_DECLARATION_SIZE,CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read}},
    // hw version Characteristic Value
    [IDX_DEV_HW_VERSION_VAL]    = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_HW_VERSION_UUID, ESP_GATT_PERM_READ,
                                                                sizeof(hw_version),sizeof(hw_version), hw_version}},
    
    // sw version Characteristic Declaration
    [IDX_DEV_SW_VERSION_CHAR]   = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
                                                   CHAR_DECLARATION_SIZE,CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read}},
    // sw version Characteristic Value
    [IDX_DEV_SW_VERSION_VAL]    = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_SW_VERSION_UUID, ESP_GATT_PERM_READ,
                                                                sizeof(sw_version)-1,sizeof(sw_version)-1, sw_version}},

};

/* Full Database Description - Used to add attributes into the database */
static const esp_gatts_attr_db_t gatt_db[HRS_IDX_NB] =
{
    // Service Declaration
    [IDX_SVC]        =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&primary_service_uuid, ESP_GATT_PERM_READ,
      sizeof(uint16_t), sizeof(GATTS_SERVICE_PRIMARY_UUID), (uint8_t *)&GATTS_SERVICE_PRIMARY_UUID}},

    /* Characteristic Declaration */
    [IDX_CHAR_A]     =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_notify}},

    /* Characteristic Value */
    [IDX_CHAR_VAL_A] =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_NOTIFY_UUID, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(char_value), (uint8_t *)char_value}},

    /* Client Characteristic Configuration Descriptor */
    [IDX_CHAR_CFG_A]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_client_config_uuid, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      sizeof(uint16_t), sizeof(heart_measurement_ccc), (uint8_t *)heart_measurement_ccc}},

    /* Characteristic Declaration */
    [IDX_CHAR_B]      =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_write}},

    /* Characteristic Value */
    [IDX_CHAR_VAL_B]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_WRITE_UUID, ESP_GATT_PERM_WRITE,
      GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(char_value), (uint8_t *)char_value}},
};

static void vesync_hal_gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event) {
        case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
            ESP_LOGI(GATTS_TABLE_TAG, "ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT");
            vesync_set_bt_status(BT_ADVERTISE_START);
            break;
        case ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT:
            ESP_LOGI(GATTS_TABLE_TAG, "ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT");
            vesync_set_bt_status(BT_ADVERTISE_START);
            break;
        case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
            if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
                ESP_LOGE(GATTS_TABLE_TAG, "advertising start failed");
            }else{
                ESP_LOGI(GATTS_TABLE_TAG, "advertising start successfully");
            }
            break;
        case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
            if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS) {
                ESP_LOGE(GATTS_TABLE_TAG, "Advertising stop failed");
            }
            else {
                ESP_LOGI(GATTS_TABLE_TAG, "Stop adv successfully\n");
            }
            break;
        case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
            ESP_LOGI(GATTS_TABLE_TAG, "update connection params status = %d, min_int = %d, max_int = %d,conn_int = %d,latency = %d, timeout = %d",
                param->update_conn_params.status,
                param->update_conn_params.min_int,
                param->update_conn_params.max_int,
                param->update_conn_params.conn_int,
                param->update_conn_params.latency,
                param->update_conn_params.timeout);
            break;
        default:
            break;
    }
}

void example_prepare_write_event_env(esp_gatt_if_t gatts_if, prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param)
{
    ESP_LOGI(GATTS_TABLE_TAG, "prepare write, handle = %d, value len = %d", param->write.handle, param->write.len);
    esp_gatt_status_t status = ESP_GATT_OK;
    if (prepare_write_env->prepare_buf == NULL) {
        prepare_write_env->prepare_buf = (uint8_t *)malloc(PREPARE_BUF_MAX_SIZE * sizeof(uint8_t));
        prepare_write_env->prepare_len = 0;
        if (prepare_write_env->prepare_buf == NULL) {
            ESP_LOGE(GATTS_TABLE_TAG, "%s, Gatt_server prep no mem", __func__);
            status = ESP_GATT_NO_RESOURCES;
        }
    } else {
        if(param->write.offset > PREPARE_BUF_MAX_SIZE) {
            status = ESP_GATT_INVALID_OFFSET;
        } else if ((param->write.offset + param->write.len) > PREPARE_BUF_MAX_SIZE) {
            status = ESP_GATT_INVALID_ATTR_LEN;
        }
    }
    /*send response when param->write.need_rsp is true */
    if (param->write.need_rsp){
        esp_gatt_rsp_t *gatt_rsp = (esp_gatt_rsp_t *)malloc(sizeof(esp_gatt_rsp_t));
        if (gatt_rsp != NULL){
            gatt_rsp->attr_value.len = param->write.len;
            gatt_rsp->attr_value.handle = param->write.handle;
            gatt_rsp->attr_value.offset = param->write.offset;
            gatt_rsp->attr_value.auth_req = ESP_GATT_AUTH_REQ_NONE;
            memcpy(gatt_rsp->attr_value.value, param->write.value, param->write.len);
            esp_err_t response_err = esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, status, gatt_rsp);
            if (response_err != ESP_OK){
               ESP_LOGE(GATTS_TABLE_TAG, "Send response error");
            }
            free(gatt_rsp);
        }else{
            ESP_LOGE(GATTS_TABLE_TAG, "%s, malloc failed", __func__);
        }
    }
    if (status != ESP_GATT_OK){
        return;
    }
    memcpy(prepare_write_env->prepare_buf + param->write.offset,
           param->write.value,
           param->write.len);
    prepare_write_env->prepare_len += param->write.len;

}

void example_exec_write_event_env(prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param){
    if (param->exec_write.exec_write_flag == ESP_GATT_PREP_WRITE_EXEC && prepare_write_env->prepare_buf){
        esp_log_buffer_hex(GATTS_TABLE_TAG, prepare_write_env->prepare_buf, prepare_write_env->prepare_len);
    }else{
        ESP_LOGI(GATTS_TABLE_TAG,"ESP_GATT_PREP_WRITE_CANCEL");
    }
    if (prepare_write_env->prepare_buf) {
        free(prepare_write_env->prepare_buf);
        prepare_write_env->prepare_buf = NULL;
    }
    prepare_write_env->prepare_len = 0;
}

/**
 * @brief 蓝牙配网发送自定义数据流
 * @param data 
 * @param data_len 
 * @return uint32_t 
 */
uint32_t vesync_blufi_notify(uint8_t *data, uint32_t data_len)
{
    return esp_blufi_send_custom_data(data, data_len);
}
/**
 * @brief 
 * @param notify_data 
 * @param len 为了跟串口统一 len包含命令+载荷长度 
 */
uint32_t vesync_bt_notify(frame_ctrl_t ctl,uint8_t *cnt,uint16_t cmd,const unsigned char *notify_data ,unsigned short len)
{
    uint32_t ret = 0;
    uint8_t sendbuf[300] ={0};
    uint8_t sendlen =0;

    if(vesync_get_bt_status() != BT_CONNTED)    return 1;
    sendlen = bt_data_frame_encode(ctl,cnt,cmd,notify_data,len,sendbuf);

    esp_log_buffer_hex(GATTS_TABLE_TAG, sendbuf, sendlen);
    ret = esp_ble_gatts_send_indicate(ble_gatts_if, ble_conn_id, heart_rate_handle_table[IDX_CHAR_VAL_A],
                                        (uint16_t)sendlen, (uint8_t *)sendbuf, false);

    return ret;                                 
}
/**
 * @brief 更新连接参数请求 满足ios系统需求
 * @return uint32_t 
 */
static uint32_t vesync_update_connect_interval(uint16_t min_interval,uint16_t max_interval,uint16_t time_out)
{
    uint32_t ret =1;
    esp_ble_conn_update_params_t conn_params = {0};
    //memcpy(conn_params.bda, param->connect.remote_bda, sizeof(esp_bd_addr_t));

    conn_params.latency = 0;
    conn_params.min_int = min_interval;    // min_int = 0x10*1.25ms = 20ms
    conn_params.max_int = max_interval;    // max_int = 0x20*1.25ms = 40ms
    conn_params.timeout = time_out;        // timeout = 400*10ms = 4000ms

    ret = esp_ble_gap_update_conn_params(&conn_params);
    return ret;
}
/**
 * @brief 查询当前uuid对应的char描述
 * @param handle 
 * @return uint8_t 
 */
static uint8_t find_char_and_desr_index(uint16_t handle)
{
    uint8_t error = 0xff;

    for(int i = 0; i < HRS_IDX_NB ; i++){
        if(handle == heart_rate_handle_table[i]){
            return i;
        }
    }

    return error;
}

/**
 * @brief 回复APP设备的配网信息
 */
static void vesync_reply_response(char *url,int err_code,char *err_describe)
{
	uint8_t buffer[200] ={0};
	uint8_t buflen = 0;

    if(err_code == 1){
        buflen = sprintf((char *)buffer, "{\"uri\":\"%s\",\"err\":%d}",url,err_code);
    }else{
        if(strcmp(url,"/beginConfigRequest")==0){
            uint8_t mode;
            int32_t ret ;
            wifi_config_t cfg;
            char upload_buf[256] ={0};
            mode = vesync_get_wifi_mode();
            if(WIFI_MODE_AP == mode){
                vesync_get_wifi_config(WIFI_IF_AP, &cfg);
                BLUFI_INFO("AP mode, %s %s", cfg.ap.ssid, cfg.ap.password);
            }else if(WIFI_MODE_STA == mode){
                char ap_mac_addr[6 * 3];
                vesync_get_wifi_config(WIFI_IF_STA, &cfg);
                BLUFI_INFO("sta mode, connected %s", cfg.sta.bssid);
                sprintf(ap_mac_addr, MACSTR, MAC2STR(cfg.sta.bssid));
                if(strlen(ap_mac_addr) < 12){
                    sprintf(ap_mac_addr, "null");	
                }
                sprintf(upload_buf, "{\"uri\":\"/beginConfigReply\",\"err\":\"%d\",\"description\":\"%s\",\"routerMac\":\"%s\",\"deviceRSSI\":\"%d\",\"firmVersion\":\"%s\"}",
                    err_code, err_describe, ap_mac_addr, /*wifi_station_get_rssi()*/12, FIRM_VERSION);

                ret = vesync_blufi_notify((uint8_t *)upload_buf, strlen(upload_buf));
            }
        }else{
            BLUFI_ERROR("blufi send error %s",url);
            buflen = sprintf((char *)buffer, "{\"uri\":\"%s\",\"err\":%d}",url,err_code);
        }
    }

	if(esp_blufi_send_custom_data(buffer, buflen) != ESP_OK){
		BLUFI_INFO("Send wifi list to app failed !\n");
	}
	BLUFI_INFO("blufi send to APP : %s \r\n", buffer);
}
/**
 * [Handle_ConfigNetJson  解析处理APP通过ble发来的配网json数据]
 * @param  json [配网json数据]
 * @return    	[无]
 */
static void vesync_Handle_ConfigNetJson(net_info_t *info,cJSON *json)
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
    cJSON *serverUrl = cJSON_GetObjectItemCaseSensitive(root, "serverUrl");
    if(true == cJSON_IsString(serverUrl)){
		char *str = strstr(serverUrl->valuestring, ":");
		if(NULL != str){
			strncpy((char*)info->station_config.server_url, serverUrl->valuestring, str - serverUrl->valuestring);
			info->station_config.server_url[str - serverUrl->valuestring] = '\0';
		}else{
			strcpy((char*)info->station_config.server_url, serverUrl->valuestring);
		}
        mask |=0x10;
		BLUFI_INFO("server_url : %s\r\n", (char*)info->station_config.server_url);
	}
    cJSON *account_id = cJSON_GetObjectItemCaseSensitive(root, "accountID");
    if(true == cJSON_IsString(account_id)){
		strcpy((char*)info->station_config.account_id, account_id->valuestring);
		BLUFI_INFO("account : %s\r\n", (char*)info->station_config.account_id);
        mask |=0x20;
	}
	cJSON *wifiSSID = cJSON_GetObjectItemCaseSensitive(root, "wifiSSID");
	if(true == cJSON_IsString(wifiSSID)){
		strcpy((char*)info->station_config.wifiSSID, wifiSSID->valuestring);
		BLUFI_INFO("wifiSSID : %s\r\n", (char*)info->station_config.wifiSSID);
        mask |=0x40;
	}

	cJSON *wifiPassword = cJSON_GetObjectItemCaseSensitive(root, "wifiPassword");
	if(true == cJSON_IsString(wifiPassword)){
		strcpy((char*)info->station_config.wifiPassword, wifiPassword->valuestring);
		BLUFI_INFO("wifiPassword : %s\r\n", (char*)info->station_config.wifiPassword);
        mask |=0x80;
	}

	cJSON *wifiStaticIP = cJSON_GetObjectItemCaseSensitive(root, "wifiStaticIP");
	if(true == cJSON_IsString(wifiStaticIP)){
		strcpy((char*)info->station_config.wifiStaticIP, wifiStaticIP->valuestring);
		BLUFI_INFO("wifiStaticIP : %s\r\n", (char*)info->station_config.wifiStaticIP);
        mask |=0x100;
	}

	cJSON *wifiGateway = cJSON_GetObjectItemCaseSensitive(root, "wifiGateway");
	if(true == cJSON_IsString(wifiGateway)){
		strcpy((char*)info->station_config.wifiGateway, wifiGateway->valuestring);
		BLUFI_INFO("wifiGateway : %s\r\n", (char*)info->station_config.wifiGateway);
        mask |=0x200;
	}

	cJSON *wifiDNS = cJSON_GetObjectItemCaseSensitive(root, "wifiDNS");
	if(true == cJSON_IsString(wifiDNS)){
		strcpy((char*)info->station_config.wifiDNS, wifiDNS->valuestring);
		BLUFI_INFO("wifiDNS : %s\r\n", (char*)info->station_config.wifiDNS);
        mask |=0x400;
	}
    if(mask & 0x30){
        //vesync_refresh_wifi(info);
    }
    BLUFI_INFO("mask 0x%04x\r\n", mask);
    if((mask & 0x7ff) == 0x7ff){
        vesync_reply_response("/beginConfigRequest",ERR_CONFIG_CMD_SUCCESS,"CONFIG_CMD_SUCCESS");
        vesync_flash_write_net_info(info);
        vesync_connect_wifi((char *)info->station_config.wifiSSID,(char *)info->station_config.wifiPassword);
    }else{
        if(strlen((char *)product_config.cid) != CID_LENGTH){
            BLUFI_ERROR("CID was missed !\r\n");
            vesync_reply_response("/beginConfigRequest",ERR_CONFIG_CID_MISSED, "CONFIG_CID_MISSED");
        }else if(info->station_config.wifiSSID == 0){
            BLUFI_ERROR("wifiSSID was missed !\r\n");
            vesync_reply_response("/beginConfigRequest",ERR_CONFIG_WIFI_SSID_MISSED, "CONFIG_WIFI_SSID_MISSED");
        }else if(info->mqtt_config.configKey == 0){
            BLUFI_ERROR("configKey was missed !\r\n");
            vesync_reply_response("/beginConfigRequest",ERR_CONFIG_CONFIGKEY_MISSED, "CONFIG_CONFIGKEY_MISSED");
        }else if(info->mqtt_config.serverIP == 0){
            BLUFI_ERROR("serverIP was missed !\r\n");
            vesync_reply_response("/beginConfigRequest",ERR_CONFIG_SERVER_IP_MISSED, "CONFIG_SERVER_IP_MISSED");
        }else if(info->station_config.server_url == 0){
            BLUFI_ERROR("server_url was missed !\r\n");
            vesync_reply_response("/beginConfigRequest",ERR_CONFIG_SERVER_URL_MISSED, "ERR_CONFIG_SERVER_URL_MISSED");
        }else if(info->station_config.account_id == 0){
            BLUFI_ERROR("account_id was missed !\r\n");
            vesync_reply_response("/beginConfigRequest",ERR_CONFIG_ACCOUNT_ID_MISSED, "ERR_CONFIG_ACCOUNT_ID_MISSED");
        }
        // else if(strcmp(info->mqtt_config.pid,DEV_PID) != 0){
        //  	confignet_upload_realtime_log(ERR_CONFIG_PID_DO_NOT_MATCHED, "CONFIG_PID_DO_NOT_MATCHED");
        // }
        else{

        }
    }
}

/**
 * @brief 回复APP设备的设备cid信息
 */
static void vesync_reply_device_cid_information(char *url,int err_code,char *err_describe)
{
	uint8_t buffer[128];
	uint8_t buflen = 0;
	buflen = sprintf((char *)buffer, "{\"uri\":\"%s\",\"err\":%d,\"deviceCid\":\"%s\"}",url,err_code, product_config.cid);

	if(esp_blufi_send_custom_data(buffer, buflen) != ESP_OK){
		BLUFI_INFO("Send device cid to app failed !\n");
	}
	BLUFI_INFO("blufi send to APP : %s \r\n", buffer);
}
/**
 * [handle_tcp_message  解析APP端通过蓝牙接口直接发送过来的数据]
 * @param  data   [网络数据]
 * @param  length [数据长度]
 * @return        [无]
 */
static void vesync_blufi_recv_custom_message(net_info_t *info,const char *data, int length)
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
		if(!strcmp(uri->valuestring, "/beginConfigRequest")){		    //开始配网
			//确认是配网信息
			vesync_Handle_ConfigNetJson(info,root); 					//解析处理json数据
		}else if(!strcmp(uri->valuestring, "/cancelConfig")){			//取消配网
            vesync_reply_response("/cancelConfig",ERR_CONFIG_CMD_SUCCESS, "CONFIG_CMD_SUCCESS");
            vesync_scan_wifi_list_stop();
            //vesync_set_bt_status(BT_CONFIG_NET_CANCEL);                 //关闭配网超时定时器    
		}else if(!strcmp(uri->valuestring, "/queryWifiList")){
            if(vesync_scan_wifi_list_start()!=0){
                vesync_reply_response("/queryWifiList",ERR_CONFIG_WIFI_DEIVER_INIT, "CONFIG_WIFI_DEIVER_INIT");
            }else{
                vesync_set_bt_status(BT_CONFIG_NET_START);                  //开启配网超时定时器
            }
		}else if(!strcmp(uri->valuestring, "/queryDeviceCid")){
            if(strlen((char *)product_config.cid) !=0){
                vesync_reply_device_cid_information("/queryDeviceCid",ERR_CONFIG_CMD_SUCCESS,"CONFIG_CMD_SUCCESS");
            }else{
                vesync_reply_response("/queryDeviceCid",ERR_NO_ALLOCATION_CID, "NO_ALLOCATION_CID");
            }
        }else{
			BLUFI_INFO("Config parameter error !");
		}
	}
	cJSON_Delete(root);									
}

/**
 * @brief 蓝牙配网状态事件回调
 * @param event 
 * @param param 
 */
static void vesync_blufi_event_handler(esp_blufi_cb_event_t event, esp_blufi_cb_param_t *param)
{
    static uint8_t blufi_server_if = 0xff;
    static uint16_t blufi_conn_id = 0xffff;

    switch (event){
        case ESP_BLUFI_EVENT_INIT_FINISH:
            BLUFI_INFO("BLUFI init finish\n");
            break;
        case ESP_BLUFI_EVENT_DEINIT_FINISH:
            BLUFI_INFO("BLUFI deinit finish\n");
            break;
        case ESP_BLUFI_EVENT_BLE_CONNECT:
            ESP_LOGI(GATTS_TABLE_TAG,"BLUFI ble connect\n");
            blufi_server_if = param->connect.server_if;
            blufi_conn_id = param->connect.conn_id;
            blufi_security_init();  //使能配网加密
            break;
        case ESP_BLUFI_EVENT_BLE_DISCONNECT:
            ESP_LOGI(GATTS_TABLE_TAG,"BLUFI ble disconnect\n");
            blufi_security_deinit();
            break;
        case ESP_BLUFI_EVENT_REPORT_ERROR:
            ESP_LOGE(GATTS_TABLE_TAG,"BLUFI report error, error code %d\n", param->report_error.state);
            esp_blufi_send_error_info(param->report_error.state);
            vesync_reply_response("/report_error",(ERR_TOTAL+param->report_error.state),"report_error");
            break;
        case ESP_BLUFI_EVENT_RECV_SLAVE_DISCONNECT_BLE:
            ESP_LOGI(GATTS_TABLE_TAG,"blufi close a gatt connection");
            esp_blufi_close(blufi_server_if, blufi_conn_id);
            break;
        case ESP_BLUFI_EVENT_DEAUTHENTICATE_STA:
            break;
        case ESP_BLUFI_EVENT_RECV_CUSTOM_DATA:  //手机下发custom data 
            vesync_blufi_recv_custom_message(&net_info,(char *)param->custom_data.data, param->custom_data.data_len);
            //vesync_blufi_notify((uint8_t *)param->custom_data.data, param->custom_data.data_len);
            //esp_log_buffer_hex("Custom Data", param->custom_data.data, param->custom_data.data_len);
            break;
        default:
            break;
    }
}

static void gatts_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    uint8_t res = 0xff;
    esp_ble_gatts_cb_param_t *p_data = (esp_ble_gatts_cb_param_t *) param;
    switch (event) {
        case ESP_GATTS_REG_EVT:{
            esp_err_t set_dev_name_ret = esp_ble_gap_set_device_name(advertise_name);
            if (set_dev_name_ret){
                ESP_LOGE(GATTS_TABLE_TAG, "set device name failed, error code = %x", set_dev_name_ret);
            }
            //config adv data
            esp_err_t ret = esp_ble_gap_config_adv_data(&adv_data);
            if (ret){
                ESP_LOGE(GATTS_TABLE_TAG, "config adv data failed, error code = %x", ret);
            }
            //config scan response data
            ret = esp_ble_gap_config_adv_data(&scan_rsp_data);
            if (ret){
                ESP_LOGE(GATTS_TABLE_TAG, "config scan response data failed, error code = %x", ret);
            }
            ESP_LOGI(GATTS_TABLE_TAG, "gatts register with adv data");
            
            //添加设备属性表 uuid 0x180a
            esp_err_t create_attr_ret = esp_ble_gatts_create_attr_tab(device_info_att_db, gatts_if, DEV_DEV_NB, SVC_INST_ID);
            if (create_attr_ret){
                ESP_LOGE(GATTS_TABLE_TAG, "create attr table failed, error code = %x", create_attr_ret);
            }
            vesync_set_bt_status(BT_CREATE_SERVICE);
        }
       	    break;
        case ESP_GATTS_READ_EVT:
            ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_READ_EVT");
       	    break;
        case ESP_GATTS_WRITE_EVT:{          //写没有应答
                res = find_char_and_desr_index(p_data->write.handle);
                if (!param->write.is_prep){
                    if(res == IDX_CHAR_VAL_B){
                        rx_frame_t rx_frame ={0};
                        rx_frame.len = param->write.len;
                        memcpy((char *)rx_frame.buff,(char *)param->write.value,param->write.len);
                        xQueueSend(hal_bt_receive_queue,&rx_frame,portTICK_PERIOD_MS);
                    }else if(res == IDX_CHAR_CFG_A){
                        if(param->write.len == 2){
                            uint16_t descr_value = param->write.value[1]<<8 | param->write.value[0];
                            if (descr_value == 0x0001){
                                ESP_LOGI(GATTS_TABLE_TAG, "notify enable");
                            }else if (descr_value == 0x0002){
                                ESP_LOGI(GATTS_TABLE_TAG, "indicate enable");
                            }else{
                                ESP_LOGI(GATTS_TABLE_TAG, "notify disable");
                            }
                        }
                    }
                }else{
                    /* send response when param->write.need_rsp is true*/
                    if (param->write.need_rsp){
                        ESP_LOGE(GATTS_TABLE_TAG, "need response");
                        esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
                    }
                }
            }
      	    break;
        case ESP_GATTS_EXEC_WRITE_EVT: //写应答
            // the length of gattc prapare write data must be less than GATTS_DEMO_CHAR_VAL_LEN_MAX. 
            ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_EXEC_WRITE_EVT");
            example_exec_write_event_env(&prepare_write_env, param);
            break;
        case ESP_GATTS_MTU_EVT:
            ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_MTU_EVT, MTU %d", param->mtu.mtu);
            break;
        case ESP_GATTS_CONF_EVT:
            ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_CONF_EVT, status = %d", param->conf.status);
            break;
        case ESP_GATTS_START_EVT:
            ESP_LOGI(GATTS_TABLE_TAG, "SERVICE_START_EVT, status %d, service_handle %d", param->start.status, param->start.service_handle);
            break;
        case ESP_GATTS_CONNECT_EVT:
            ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_CONNECT_EVT, conn_id = %d gatts_if =%d", param->connect.conn_id,gatts_if);
            ble_conn_id = p_data->connect.conn_id;
    	    ble_gatts_if = gatts_if;
            memcpy(remote_device_addr,p_data->connect.remote_bda,ESP_BD_ADDR_LEN);
            vesync_set_bt_status(BT_CONNTED);
            break;
        case ESP_GATTS_DISCONNECT_EVT:
            ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_DISCONNECT_EVT, reason = 0x%x", param->disconnect.reason);
            vesync_set_bt_status(BT_DISCONNTED);
            break;  
        case ESP_GATTS_CREAT_ATTR_TAB_EVT:{
            if (param->add_attr_tab.status != ESP_GATT_OK){
                ESP_LOGE(GATTS_TABLE_TAG, "create attribute table failed, error code=0x%x", param->add_attr_tab.status);
            }else{
                ESP_LOGI(GATTS_TABLE_TAG, "add tab uuid = %x", param->add_attr_tab.svc_uuid.uuid.uuid16);
                if (param->add_attr_tab.svc_uuid.uuid.uuid16 == ESP_GATT_UUID_DEVICE_INFO_SVC){ //之前是否注册设备信息服务属性
                    esp_err_t create_attr_ret = esp_ble_gatts_create_attr_tab(gatt_db, gatts_if, HRS_IDX_NB, SVC_INST_ID);
                    if (create_attr_ret){
                       ESP_LOGE(GATTS_TABLE_TAG, "create attr table failed, error code = %x", create_attr_ret);
                    }
                }
                
                ESP_LOGI(GATTS_TABLE_TAG, "create attribute table successfully, the number handle = %d\n",param->add_attr_tab.num_handle);
                memcpy(heart_rate_handle_table, param->add_attr_tab.handles, sizeof(heart_rate_handle_table));
                esp_ble_gatts_start_service(heart_rate_handle_table[IDX_SVC]);
            }
            break;
        }
        case ESP_GATTS_STOP_EVT:
        case ESP_GATTS_OPEN_EVT:
        case ESP_GATTS_CANCEL_OPEN_EVT:
        case ESP_GATTS_CLOSE_EVT:
        case ESP_GATTS_LISTEN_EVT:
        case ESP_GATTS_CONGEST_EVT:
        case ESP_GATTS_UNREG_EVT:
        case ESP_GATTS_DELETE_EVT:
        default:
            break;
    }
}

/**
 * @brief 主动断开当前的蓝牙连接
 * @return uint32_t 
 */
uint32_t vesync_bt_disconnect(void)
{
    return esp_ble_gap_disconnect(remote_device_addr);
}
/**
 * @brief 蓝牙是否连接成功？
 * @return true 
 * @return false 
 */
bool vesync_bt_connected(void)
{
    return (bt_status == BT_CONNTED);
}
/**
 * @brief gatts数据回调
 * @param event 
 * @param gatts_if 
 * @param param 
 */
static void vesync_hal_gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    if (event == ESP_GATTS_REG_EVT) {
        if (param->reg.status == ESP_GATT_OK) {
            heart_rate_profile_tab[PROFILE_APP_IDX].gatts_if = gatts_if;
        } else {
            ESP_LOGE(GATTS_TABLE_TAG, "regI (1287) Vesync_BT: vesync_set_bt_status 0app failed, app_id %04x, status %d",
                    param->reg.app_id,
                    param->reg.status);
            return;
        }
    }
    do {
        int idx;
        for (idx = 0; idx < PROFILE_NUM; idx++) {
            if (gatts_if == ESP_GATT_IF_NONE || gatts_if == heart_rate_profile_tab[idx].gatts_if) {
                if (heart_rate_profile_tab[idx].gatts_cb) {
                    heart_rate_profile_tab[idx].gatts_cb(event, gatts_if, param);
                }
            }
        }
    } while (0);
}

/**
 * @brief 蓝牙广播超时回调处理
 * @param timer xQueueSend(cmd_cmd_queue,&spp_cmd_buff,10/portTICK_PERIOD_MS);
 */
static void vesynv_advertise_timerout_callback(TimerHandle_t timer)
{
    if(xTimerIsTimerActive(&timer) != pdFALSE){
        method_timer_delete(&timer);
        ESP_LOGI(GATTS_TABLE_TAG, "delete timer!");
    }else{
        vesync_set_bt_status(BT_ADVERTISE_STOP);
    }
    ESP_LOGI(GATTS_TABLE_TAG, "advertise timer stop");
}

/**
 * @brief 配网超时回调处理
 * @param timer 
 */
static void vesynv_net_config_timerout_callback(TimerHandle_t timer)
{
    if(xTimerIsTimerActive(&timer) != pdFALSE){
        method_timer_delete(&timer);
        ESP_LOGI(GATTS_TABLE_TAG, "delete timer!");
    }else{
        vesync_set_bt_status(BT_CONFIG_NET_TIMEOUT);
    }
    ESP_LOGI(GATTS_TABLE_TAG, "net_config timer stop");
}

/**
 * @brief 设置蓝牙链路状态
 * @param new_status 
 */
static void vesync_set_bt_status(BT_STATUS_T new_status)
{
    if(bt_status != new_status){
        switch(new_status){
            case BT_CREATE_SERVICE:
                    if(method_timer_create(&advertise_timer ,APP_ADV_TIMEOUT_IN_SECONDS,false,vesynv_advertise_timerout_callback) != true){
                        ESP_LOGE(GATTS_TABLE_TAG, "create advertise time fail!!!");
                    }
                break;
            case BT_ADVERTISE_START:{
                    esp_power_level_t ble_power;
                    ble_power = esp_ble_tx_power_get(ESP_BLE_PWR_TYPE_DEFAULT);
                    ESP_LOGI(GATTS_TABLE_TAG, "ble power level [%d]",ble_power);
                }
                break;
            case BT_ADVERTISE_STOP:
                    vesync_bt_advertise_stop();
                break;
            case BT_CONNTED:
                break;
            case BT_DISCONNTED:
                break;
            case BT_CONFIG_NET_START:
                    method_timer_start(&net_config_timer);
                break;
            case BT_CONFIG_NET_TIMEOUT:
                    method_timer_stop(&net_config_timer);
                break;
            case BT_CONFIG_NET_SUCCEED:
                    method_timer_stop(&net_config_timer);
                break;
            case BT_CONFIG_NET_CANCEL:
                    method_timer_stop(&net_config_timer);
                break;
            default:
                break;
        }
        bt_status = new_status;
        if(vesync_bt_connect_status_callback != NULL){  //应用程获取蓝牙状态
            vesync_bt_connect_status_callback(new_status);
        }
    }
    ESP_LOGI(GATTS_TABLE_TAG, "vesync set new status bits [%d]" ,new_status);
}

/**
 * @brief 返回蓝牙链路连接状态
 * @return BT_STATUS_T 
 */
BT_STATUS_T vesync_get_bt_status(void)
{
    //ESP_LOGI(GATTS_TABLE_TAG, "vesync get current status [%d]" ,bt_status);
    return bt_status;
}

/**   
 * @brief 设置广播超时
 * @param timeout 为0不设置广播超时时间，一直广播
 */
void vesync_bt_advertise_start(uint32_t timeout)
{   
    method_timer_stop(&advertise_timer);
    if(timeout != 0){
        /* 创建广播监测超时定时器 */
        ESP_LOGI(GATTS_TABLE_TAG, "bt refresh timer [%d]ms" ,timeout);
        method_timer_change_period(&advertise_timer,timeout);
        method_timer_start(&advertise_timer);
    }
    esp_ble_gap_start_advertising(&adv_params);
}

/**
 * @brief 停止广播
 */
void vesync_bt_advertise_stop(void)
{
    method_timer_stop(&advertise_timer);
    ESP_ERROR_CHECK(esp_ble_gap_stop_advertising());
}

/**
 * @brief 蓝牙接收队列任务处理
 * @param pvParameters 
 */
static void vesync_bt_handler_loop(void *pvParameters)
{
    rx_frame_t rx_frame ={0};
    for(;;){
        if(hal_bt_receive_queue !=0){
            if(xQueueReceive(hal_bt_receive_queue, &rx_frame, portMAX_DELAY)) {
                //esp_log_buffer_hex(GATTS_TABLE_TAG,rx_frame.buff,rx_frame.len);
                if(m_bt_handler != NULL){
                     m_bt_handler((unsigned char *)rx_frame.buff, rx_frame.len);    //接收app数据并赋值给info_str
                 }
            }
        }
    }
    vTaskDelete(NULL);
}

/**
 * @brief 创建蓝牙HAL数据收发任务
 */
static void vesync_hal_bt_task_init(void)
{
    hal_bt_receive_queue = xQueueCreate(QUEUE_LEN, sizeof(rx_frame_t));
    if(hal_bt_receive_queue == 0){
        ESP_LOGE(GATTS_TABLE_TAG, "create bt task fail!");
    }
    xTaskCreate(vesync_bt_handler_loop, "vesync_bt_handler_loop", 4096, NULL, 9, NULL);
}

/**
 * @brief 创建蓝牙配网服务和初始化配网功能
 */
static void vesync_blufi_init(void)
{
    esp_err_t ret;
    esp_bt_controller_status_t bt_status;
    bt_status = esp_bt_controller_get_status();

    ESP_LOGI(GATTS_TABLE_TAG,"BLUFI status %d\n", bt_status);
    ESP_LOGI(GATTS_TABLE_TAG,"BLUFI VERSION %04x\n", esp_blufi_get_version());

    if(bt_status == ESP_BT_CONTROLLER_STATUS_ENABLED ){
        ret = esp_blufi_register_callbacks(&vesync_blufi_callbacks);
        if(ret){
            ESP_LOGE(GATTS_TABLE_TAG,"%s blufi register failed, error code = %x\n", __func__, ret);
            return;
        }
        esp_blufi_profile_init();
        ESP_LOGI(GATTS_TABLE_TAG,"blufi register success");
    }
}

/**
 * @brief 动态修改蓝牙广播名称
 * @param name 
 * @return uint32_t 
 */
uint32_t vesync_bt_dynamic_set_ble_advertise_name(char *name)
{
    uint32_t ret;
    //vesync_bt_advertise_stop();
    strcpy(advertise_name,name);
    ret = esp_ble_gap_set_device_name(name);
    if (ret){
        ESP_LOGE(GATTS_TABLE_TAG, "set device name failed, error code = %x", ret);
    }
    ret = esp_ble_gap_config_adv_data(&scan_rsp_data);
    if (ret){
        ESP_LOGE(GATTS_TABLE_TAG, "config scan response data failed, error code = %x", ret);
    }
    //vesync_bt_advertise_start(0);
    return ret;
}

/**
 * @brief 动态修改广播参数
 * @param product_type 
 * @param product_num 
 * @return uint32_t 
 */
uint32_t vesync_bt_dynamic_ble_advertise_para(uint8_t product_type,uint8_t product_num)
{
    uint32_t ret;
    adver_manufacturer[9] = product_type;
    adver_manufacturer[10] = product_num;
    ret = esp_ble_gap_config_adv_data(&adv_data);
    if (ret){
        ESP_LOGE(GATTS_TABLE_TAG, "config adv data failed, error code = %x", ret);
    }
    return ret;
}
/**
 * @brief 
 */
void vesync_hal_bt_client_deinit(void)
{
    ESP_ERROR_CHECK(esp_ble_gatts_app_unregister(ESP_APP_ID));
    ESP_ERROR_CHECK(esp_bluedroid_disable());
    ESP_ERROR_CHECK(esp_bluedroid_deinit()); 
    ESP_ERROR_CHECK(esp_bt_controller_disable());
    ESP_ERROR_CHECK(esp_bt_controller_deinit());
    ESP_LOGI(GATTS_TABLE_TAG,"vesync_hal_bt_client_deinit");
}
/**
 * @brief 初始化蓝牙协议栈及配置蓝牙广播参数
 * @param adver_name 蓝牙广播名称
 * @param version 
 * @param product_type 
 * @param product_num 
 * @param custom 
 * @param enable_blufi 
 * @param cb 
 * @return int32_t 
 */
int32_t vesync_bt_client_init(char *adver_name,uint8_t version,uint8_t product_type,uint8_t product_num,uint8_t *custom,bool enable_blufi,
                                    vesync_bt_status_cb_t bt_status_cb, bt_recv_cb_t cb)
{
    esp_err_t ret =0;
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    //strncpy((char *)sw_version,(char *)adver_name,sizeof(sw_version)-1);
    strcpy((char *)advertise_name,(char *)adver_name);
    uint8_t mac[6] ={0};

    adver_manufacturer[2] = version;
    esp_read_mac(&mac[0], ESP_MAC_BT);
    for(uint8_t i=0;i<sizeof(mac);i++){
        adver_manufacturer[3+i] = mac[5-i];
    }
    adver_manufacturer[9] = product_type;
    adver_manufacturer[10] = product_num;

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        ESP_LOGE(GATTS_TABLE_TAG, "%s enable controller failed: %s", __func__, esp_err_to_name(ret));
        return ret;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) {
        ESP_LOGE(GATTS_TABLE_TAG, "%s enable controller failed: %s", __func__, esp_err_to_name(ret));
        return ret;
    }

    ret = esp_bluedroid_init();
    if (ret) {
        ESP_LOGE(GATTS_TABLE_TAG, "%s init bluetooth failed: %s", __func__, esp_err_to_name(ret));
        return ret;
    }

    ret = esp_bluedroid_enable();
    if (ret) {
        ESP_LOGE(GATTS_TABLE_TAG, "%s enable bluetooth failed: %s", __func__, esp_err_to_name(ret));
        return ret;
    }

    ret = esp_ble_gatts_register_callback(vesync_hal_gatts_event_handler);
    if (ret){
        ESP_LOGE(GATTS_TABLE_TAG, "gatts register error, error code = %x", ret);
        return ret;
    }

    ret = esp_ble_gap_register_callback(vesync_hal_gap_event_handler);
    if (ret){
        ESP_LOGE(GATTS_TABLE_TAG, "gap register error, error code = %x", ret);
        return ret;
    }

    ret = esp_ble_gatts_app_register(ESP_APP_ID);
    if (ret){
        ESP_LOGE(GATTS_TABLE_TAG, "gatts app register error, error code = %x", ret);
        return ret;
    }

    ret = esp_ble_gatt_set_local_mtu(BLE_MAX_MTU);
    if (ret){
        ESP_LOGE(GATTS_TABLE_TAG, "set local  MTU failed, error code = %x", ret);
    }

    ret = esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT,TX_POWER);
    if (ret){
        ESP_LOGE(GATTS_TABLE_TAG, "gatts app register error, error code = %x", ret);
        return ret;
    }

    vesync_hal_bt_task_init();

    m_bt_handler = cb;
    vesync_bt_connect_status_callback = bt_status_cb;

    if(enable_blufi){
        method_timer_create(&net_config_timer,NET_CONFIG_TIME_OUT,false,vesynv_net_config_timerout_callback);
        vesync_blufi_init();
    }
    return ret;
}