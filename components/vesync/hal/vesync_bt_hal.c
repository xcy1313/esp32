/**
 * @file vesync_bt_hal.c
 * @brief 蓝牙hal配置ble服务属性接口
 * @author Jiven 
 * @date 2018-12-14
 */
#include "vesync_bt_hal.h"
#include "vesync_wifi_hal.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp_log.h"

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

#define QUEUE_LEN                   100

static void vesync_set_bt_status(BT_STATUS_T new_status);
bt_recv_cb_t m_bt_handler;

uint16_t heart_rate_handle_table[HRS_IDX_NB];

static uint16_t ble_conn_id = 0xffff;
static esp_gatt_if_t ble_gatts_if;
static BT_STATUS_T bt_status = BT_INIT;

static TimerHandle_t advertise_timer;
static xQueueHandle hal_bt_receive_queue = NULL;

typedef struct{
    uint8_t len;
    uint8_t buff[80];
}rx_frame_t;

typedef struct{
    uint8_t                 *prepare_buf;
    int                     prepare_len;
} prepare_type_env_t;

static prepare_type_env_t prepare_write_env;

static bool advertise_timer_init(uint32_t time_out);
static void gatts_profile_event_handler(esp_gatts_cb_event_t event,esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
static void vesync_blufi_event_handler(esp_blufi_cb_event_t event, esp_blufi_cb_param_t *param);

static char advertise_name[30] = {0};
static uint8_t adver_manufacturer[] ={0xd0,0x6,0x1,0x36,0x35,0x34,0x33,0x32,0x31,0xe0,0xa0};
static uint8_t service_uuid[16] = {
    /* LSB <--------------------------------------------------------------------------------> MSB */
    //first uuid, 16bit, [12],[13] is the value
    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00,
};

/* The length of adv data must be less than 31 bytes */
static esp_ble_adv_data_t adv_data = {
    .set_scan_rsp        = false,
    .include_name        = true,
    .include_txpower     = false,
    .min_interval        = APP_ADV_INTERVAL_MIN,
    .max_interval        = APP_ADV_INTERVAL_MAX,
    .appearance          = 0x00,
    .manufacturer_len    = sizeof(adver_manufacturer),
    .p_manufacturer_data = adver_manufacturer,
    .service_data_len    = 0,
    .p_service_data      = NULL,
    .service_uuid_len    = sizeof(service_uuid),
    .p_service_uuid      = service_uuid,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

// scan response data
static esp_ble_adv_data_t scan_rsp_data = {
    .set_scan_rsp        = true,
    .include_name        = true,
    .include_txpower     = false,
    .min_interval        = APP_ADV_INTERVAL_MIN,
    .max_interval        = APP_ADV_INTERVAL_MAX,
    .appearance          = 0x00,
    .manufacturer_len    = 0, //TEST_MANUFACTURER_DATA_LEN,
    .p_manufacturer_data = NULL, //&test_manufacturer[0],
    .service_data_len    = 0,
    //.p_service_data      = NULL,
    //.service_uuid_len    = 16,
    .p_service_uuid      = service_uuid,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
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
static uint8_t sw_version[5] = {0x31,0x2e,0x31,0x2e,0x31};

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
                                                                sizeof(sw_version),sizeof(sw_version), sw_version}},

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
 * @brief 
 * @param notify_data 
 * @param len 为了跟串口统一 len包含命令+载荷长度 
 */
uint32_t vesync_bt_notify(frame_ctrl_t ctl,uint8_t *cnt,uint16_t cmd,const void *notify_data ,unsigned short len)
{
    uint32_t ret = 0;
    uint8_t sendbuf[200] ={0};
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
 * @brief 蓝牙配网状态事件回调
 * @param event 
 * @param param 
 */
static void vesync_blufi_event_handler(esp_blufi_cb_event_t event, esp_blufi_cb_param_t *param)
{
    static uint8_t server_if = 0xff;
    static uint16_t conn_id = 0xffff;

    switch (event){
        case ESP_BLUFI_EVENT_BLE_CONNECT:
            ESP_LOGI(GATTS_TABLE_TAG,"BLUFI ble connect\n");
            server_if = param->connect.server_if;
            conn_id = param->connect.conn_id;
            blufi_security_init();
            break;
        case ESP_BLUFI_EVENT_BLE_DISCONNECT:
            ESP_LOGI(GATTS_TABLE_TAG,"BLUFI ble disconnect\n");
            blufi_security_deinit();
            break;
        case ESP_BLUFI_EVENT_SET_WIFI_OPMODE:
            ESP_LOGI(GATTS_TABLE_TAG,"BLUFI Set WIFI opmode %d\n", param->wifi_mode.op_mode);
            ESP_ERROR_CHECK( esp_wifi_set_mode(param->wifi_mode.op_mode) );
            break;
        case ESP_BLUFI_EVENT_REQ_CONNECT_TO_AP:
            ESP_LOGI(GATTS_TABLE_TAG,"BLUFI requset wifi connect to AP\n");
            esp_wifi_disconnect();
            esp_wifi_connect();
            break;
        case ESP_BLUFI_EVENT_REQ_DISCONNECT_FROM_AP:
            ESP_LOGI(GATTS_TABLE_TAG,"BLUFI requset wifi disconnect from AP\n");
            esp_wifi_disconnect();
            break;
        case ESP_BLUFI_EVENT_REPORT_ERROR:
            ESP_LOGE(GATTS_TABLE_TAG,"BLUFI report error, error code %d\n", param->report_error.state);
            esp_blufi_send_error_info(param->report_error.state);
            break;
        case ESP_BLUFI_EVENT_GET_WIFI_STATUS: 
            ESP_LOGI(GATTS_TABLE_TAG,"BLUFI get wifi status from AP\n");
            break;
        case ESP_BLUFI_EVENT_RECV_SLAVE_DISCONNECT_BLE:
            ESP_LOGI(GATTS_TABLE_TAG,"blufi close a gatt connection");
            esp_blufi_close(server_if, conn_id);
            break;
        case ESP_BLUFI_EVENT_DEAUTHENTICATE_STA:
            break;
        case ESP_BLUFI_EVENT_RECV_STA_BSSID:
            //ESP_LOGI(GATTS_TABLE_TAG,"Recv STA BSSID %s\n", sta_config.sta.ssid);
            break;
        case ESP_BLUFI_EVENT_GET_WIFI_LIST:
            //vesync_scan_wifi_list_start();
            break;
        case ESP_BLUFI_EVENT_RECV_CUSTOM_DATA:  //手机下发custom data 
            //handle_blufi_custom_message(&device_info,(char *)param->custom_data.data, param->custom_data.data_len);
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

static bool advertise_timer_start(void)
{
    bool status = false;

    if (xTimerStart(advertise_timer, portMAX_DELAY) != pdPASS) {
        status = false;
    } else {
        status =  true;
    }

    return status;
}

static bool advertise_timer_stop(void)
{
    bool status = false;

    if (xTimerStop(advertise_timer, portMAX_DELAY) != pdPASS) {
        status = false;
    } else {
        status = true;
    }

    return status;
}

static bool advertise_timer_delete(void)
{
    xTimerDelete(advertise_timer, portMAX_DELAY);
    advertise_timer = NULL;

    return true;
}

/**
 * @brief 设置蓝牙链路状态
 * @param new_status 
 */
static void vesync_set_bt_status(BT_STATUS_T new_status)
{
    uint8_t bt_conn;

    if(bt_status != new_status){
        switch(new_status){
            case BT_CREATE_SERVICE:
                    if(advertise_timer_init(APP_ADV_TIMEOUT_IN_SECONDS) != true){
                        ESP_LOGE(GATTS_TABLE_TAG, "create advertise time fail!!!");
                    }
                break;
            case BT_ADVERTISE_START:{
                    esp_power_level_t ble_power;
                    vesync_bt_advertise_start(APP_ADV_TIMEOUT_IN_SECONDS);
                    ble_power = esp_ble_tx_power_get(ESP_BLE_PWR_TYPE_DEFAULT);
                    ESP_LOGI(GATTS_TABLE_TAG, "ble power level [%d]",ble_power);
                }
                break;
            case BT_ADVERTISE_STOP:
                break;
            case BT_CONNTED:
                    vesync_bt_advertise_stop();
                    vesync_update_connect_interval(0x10,0x20,400);
                break;
            case BT_DISCONNTED:
                    vesync_bt_advertise_start(APP_ADV_TIMEOUT_IN_SECONDS);
                break;
            default:
                break;
        }
        bt_status = new_status;
    }
    ESP_LOGI(GATTS_TABLE_TAG, "vesync set new status bits [%d]" ,new_status);
}

/**
 * @brief 返回蓝牙链路连接状态
 * @return BT_STATUS_T 
 */
BT_STATUS_T vesync_get_bt_status(void)
{
    ESP_LOGI(GATTS_TABLE_TAG, "vesync get current status [%d]" ,bt_status);
    return bt_status;
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
 * @brief 蓝牙广播超时回调处理
 * @param timer xQueueSend(cmd_cmd_queue,&spp_cmd_buff,10/portTICK_PERIOD_MS);
 */
static void vesynv_advertise_timerout_callback(TimerHandle_t timer)
{
    if(xTimerIsTimerActive(timer) != pdFALSE){
        advertise_timer_delete();
        ESP_LOGI(GATTS_TABLE_TAG, "delete timer!");
    }else{
        advertise_timer_stop();
        ESP_LOGI(GATTS_TABLE_TAG, "stop timer!");
    }
    vesync_bt_advertise_stop();
    ESP_LOGI(GATTS_TABLE_TAG, "vesync bt advertise stop!!!");
}

/**
 * @brief 创建蓝牙广播超时定时器
 * @param time_out 
 * @return true 
 * @return false 
 */
static bool advertise_timer_init(uint32_t time_out)
{
    advertise_timer = xTimerCreate("advertise_timer", 10000 / portTICK_PERIOD_MS, pdFALSE,
                                         NULL, vesynv_advertise_timerout_callback);
    if(advertise_timer == NULL){
        return false;
    }else{
        return true;
    }
}

/**   
 * @brief 设置广播超时
 * @param timeout 为0不设置广播超时时间，一直广播
 */
void vesync_bt_advertise_start(uint32_t timeout)
{   
    advertise_timer_stop();
    if(timeout != 0){
        /* 创建广播监测超时定时器 */
        ESP_LOGI(GATTS_TABLE_TAG, "bt refresh timer [%d]ms" ,timeout);
        xTimerChangePeriod(advertise_timer,timeout/portTICK_PERIOD_MS,portMAX_DELAY);
        advertise_timer_start();
    }
    esp_ble_gap_start_advertising(&adv_params);
}

/**
 * @brief 停止广播
 */
void vesync_bt_advertise_stop(void)
{
    advertise_timer_stop();
    ESP_ERROR_CHECK(esp_ble_gap_stop_advertising());
}

/**
 * @brief 
 */
void vesync_hal_bt_client_deinit(void)
{
    advertise_timer_delete();
    ESP_ERROR_CHECK(esp_ble_gatts_app_unregister(ESP_APP_ID));
    ESP_ERROR_CHECK(esp_bluedroid_disable());
    ESP_ERROR_CHECK(esp_bluedroid_deinit());
    ESP_ERROR_CHECK(esp_bt_controller_disable());
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
                esp_log_buffer_hex(GATTS_TABLE_TAG,rx_frame.buff,rx_frame.len);
                if(m_bt_handler != NULL){
                     m_bt_handler(NULL,(char *)rx_frame.buff, rx_frame.len);    //接收app数据并赋值给info_str
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
int32_t vesync_hal_bt_client_init(char *adver_name,uint8_t version,uint8_t product_type,uint8_t product_num,uint8_t *custom,bool enable_blufi,bt_recv_cb_t cb)
{
    esp_err_t ret =0;
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    strcpy((char *)advertise_name,(char *)adver_name);
    adver_manufacturer[2] = version;
    esp_read_mac(&adver_manufacturer[3], ESP_MAC_BT);
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
    if(enable_blufi){
        vesync_blufi_init();
    }
    vesync_hal_bt_task_init();

    m_bt_handler = cb;
    vesync_bt_advertise_start(ADVER_TIME_OUT);

    return ret;
}