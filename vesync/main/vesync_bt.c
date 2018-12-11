/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/****************************************************************************
*
* This file is for gatt server. It can send adv data, be connected by client.
* Run the gatt_client demo, the client demo will automatically connect to the gatt_server_service_table demo.
* Client demo will enable gatt_server_service_table's notify after connection. Then two devices will exchange
* data.
*
****************************************************************************/

#include "vesync_bt.h"
#include "vesync_uart.h"
#include "vesync_wifi.h"
#include "vesync_public.h"

#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"

#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_task_wdt.h"

#define GATTS_TABLE_TAG "Vesync_BT"

#define APP_ADV_INTERVAL_MIN            0x20                                                /**< The advertising interval (in units of 0.625 ms. This value corresponds to 300 ms). */
#define APP_ADV_INTERVAL_MAX            0x40                                                /**< The advertising interval (in units of 0.625 ms. This value corresponds to 300 ms). */
#define DEVICE_NAME                     "ESP32_BT"
#define TX_POWER                        ESP_PWR_LVL_N0                                     /* +3dbm*/
#define APP_ADV_TIMEOUT_IN_SECONDS      ADVER_TIME_OUT                                     /**< The advertising timeout (in units of seconds). */


#define PROFILE_NUM                 1
#define PROFILE_APP_IDX             0
#define ESP_APP_ID                  0x55
#define SVC_INST_ID                 0

#define RING_BUFF_LEN               400
#define GATTS_DEMO_CHAR_VAL_LEN_MAX 500
#define PREPARE_BUF_MAX_SIZE        1024
#define CHAR_DECLARATION_SIZE       (sizeof(uint8_t))

#define ADV_CONFIG_FLAG             (1 << 0)
#define SCAN_RSP_CONFIG_FLAG        (1 << 1)

static void vesync_set_bt_status(BT_STATUS_T new_status);
bt_recv_cb_t m_bt_handler;

uint16_t heart_rate_handle_table[HRS_IDX_NB];

static uint16_t ble_conn_id = 0xffff;
static esp_gatt_if_t ble_gatts_if;
static BT_STATUS_T bt_status = BT_INIT;

static TimerHandle_t advertise_timer;
static RingbufHandle_t ringbuf_read;
static RingbufHandle_t ringbuf_write;

typedef struct{
    uint8_t                 *prepare_buf;
    int                     prepare_len;
} prepare_type_env_t;

static prepare_type_env_t prepare_write_env;

#define CONFIG_SET_RAW_ADV_DATA
#ifdef CONFIG_SET_RAW_ADV_DATA
static uint8_t raw_adv_data[] = {
        /* flags */
        0x02, 0x01, 0x06,
        /*user defined*/
        0x0c,0xff,0x06,0xd0,0x1,0x36,0x35,0x34,0x33,0x32,0x31,0xe0,0xa0
};
static uint8_t raw_scan_rsp_data[] = {
        /* flags */
        0x02, 0x01, 0x06,
        /* name */
        0x09, 0x9,'E','S','P','3','2','_','B','T'
};

#else
static uint8_t service_uuid[16] = {
    /* LSB <--------------------------------------------------------------------------------> MSB */
    //first uuid, 16bit, [12],[13] is the value
    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00,
};

/* The length of adv data must be less than 31 bytes */
static esp_ble_adv_data_t adv_data = {
    .set_scan_rsp        = false,
    .include_name        = true,
    .include_txpower     = true,
    .min_interval        = 0x20,
    .max_interval        = 0x40,
    .appearance          = 0x00,
    .manufacturer_len    = 0,    //TEST_MANUFACTURER_DATA_LEN,
    .p_manufacturer_data = NULL, //test_manufacturer,
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
    .include_txpower     = true,
    .min_interval        = 0x20,
    .max_interval        = 0x40,
    .appearance          = 0x00,
    .manufacturer_len    = 0, //TEST_MANUFACTURER_DATA_LEN,
    .p_manufacturer_data = NULL, //&test_manufacturer[0],
    .service_data_len    = 0,
    .p_service_data      = NULL,
    .service_uuid_len    = 16,
    .p_service_uuid      = service_uuid,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};
#endif /* CONFIG_SET_RAW_ADV_DATA */

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

static bool advertise_timer_init(uint32_t time_out);
static void gatts_profile_event_handler(esp_gatts_cb_event_t event,
					esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

/* One gatt-based profile one app_id and one gatts_if, this array will store the gatts_if returned by ESP_GATTS_REG_EVT */
static struct gatts_profile_inst heart_rate_profile_tab[PROFILE_NUM] = {
    [PROFILE_APP_IDX] = {
        .gatts_cb = gatts_profile_event_handler,
        .gatts_if = ESP_GATT_IF_NONE,       /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
    },
};

/* user Service characteristic*/
static const uint16_t GATTS_SERVICE_PRIMARY_UUID   = 0xFFF0;
static const uint16_t GATTS_CHAR_NOTIFY_UUID       = 0xFF01;
static const uint16_t GATTS_CHAR_WRITE_UUID        = 0xFF02;

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

void vesync_bt_gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
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

    ESP_LOGI(GATTS_TABLE_TAG, "ble send notify gatts_if=%d,conn_id=%d len=%d", ble_gatts_if,ble_conn_id,len);
    esp_log_buffer_hex(GATTS_TABLE_TAG, sendbuf, sendlen);
    ret = esp_ble_gatts_send_indicate(ble_gatts_if, ble_conn_id, heart_rate_handle_table[IDX_CHAR_VAL_A],
                                        (uint16_t)sendlen, (uint8_t *)sendbuf, false);

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

static void gatts_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    uint8_t res = 0xff;
    esp_ble_gatts_cb_param_t *p_data = (esp_ble_gatts_cb_param_t *) param;
    switch (event) {
        case ESP_GATTS_REG_EVT:{
            esp_err_t set_dev_name_ret = esp_ble_gap_set_device_name(DEVICE_NAME);
            if (set_dev_name_ret){
                ESP_LOGE(GATTS_TABLE_TAG, "set device name failed, error code = %x", set_dev_name_ret);
            }
    #ifdef CONFIG_SET_RAW_ADV_DATA
            esp_err_t raw_adv_ret = esp_ble_gap_config_adv_data_raw(raw_adv_data, sizeof(raw_adv_data));
            if (raw_adv_ret){
                ESP_LOGE(GATTS_TABLE_TAG, "config raw adv data failed, error code = %x ", raw_adv_ret);
            }
            esp_err_t raw_scan_ret = esp_ble_gap_config_scan_rsp_data_raw(raw_scan_rsp_data, sizeof(raw_scan_rsp_data));
            if (raw_scan_ret){
                ESP_LOGE(GATTS_TABLE_TAG, "config raw scan rsp data failed, error code = %x", raw_scan_ret);
            }
    #else
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
    #endif
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
                        BaseType_t done =0;
                        xRingbufferPrintInfo(ringbuf_write);
                        BaseType_t rst_len = xRingbufferGetCurFreeSize(ringbuf_write);
                        if(rst_len >= param->write.len){
                            xRingbufferSend(ringbuf_write, (char *)param->write.value, param->write.len,portMAX_DELAY);
                        }else{
                            ESP_LOGE(GATTS_TABLE_TAG, "rx fifo full and data lose!");
                        }
                        ESP_LOGI(GATTS_TABLE_TAG, "done %d ,rst_len len [%d]，write_len [%d]",done,rst_len,param->write.len);
                        //esp_log_buffer_hex(GATTS_TABLE_TAG,(char *)(p_data->write.value),p_data->write.len);
                    }else if(res == IDX_CHAR_CFG_A){
                        if(param->write.len == 2){
                            uint16_t descr_value = param->write.value[1]<<8 | param->write.value[0];
                            if (descr_value == 0x0001){
                                ESP_LOGI(GATTS_TABLE_TAG, "notify enable");
#if 0                       
                                uint8_t notify_data[15];
                                for (int i = 0; i < sizeof(notify_data); ++i){
                                    notify_data[i] = i % 0xff;
                                }
                                //the size of notify_data[] need less than MTU size
                                esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id, heart_rate_handle_table[IDX_CHAR_VAL_A],
                                                        sizeof(notify_data), notify_data, false);
#endif 
                            }else if (descr_value == 0x0002){
                                ESP_LOGI(GATTS_TABLE_TAG, "indicate enable");
#if 0                        
                                uint8_t indicate_data[15];
                                for (int i = 0; i < sizeof(indicate_data); ++i)
                                {
                                    indicate_data[i] = i % 0xff;
                                }
                                //the size of indicate_data[] need less than MTU size
                                esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id, heart_rate_handle_table[IDX_CHAR_VAL_A],
                                                    sizeof(indicate_data), indicate_data, true);
#endif                                            
                            }else{
                                ESP_LOGI(GATTS_TABLE_TAG, "notify disable");
                            }
                        }
                    }
    #if 0                               
                        if(m_bt_handler != NULL){
                            m_bt_handler((char *)param->write.value, param->write.len);
                        }
    #endif                    
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
#if 0            
            esp_log_buffer_hex(GATTS_TABLE_TAG, param->connect.remote_bda, 6);
            esp_ble_conn_update_params_t conn_params = {0};
            memcpy(conn_params.bda, param->connect.remote_bda, sizeof(esp_bd_addr_t));
            /* For the IOS system, please reference the apple official documents about the ble connection parameters restrictions. */
            conn_params.latency = 0;
            conn_params.max_int = 0x20;    // max_int = 0x20*1.25ms = 40ms
            conn_params.min_int = 0x10;    // min_int = 0x10*1.25ms = 20ms
            conn_params.timeout = 400;    // timeout = 400*10ms = 4000ms
            //start sent the update connection parameters to the peer device.
            esp_ble_gap_update_conn_params(&conn_params);
#endif            
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


static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    /* If event is register event, store the gatts_if for each profile */
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
            /* ESP_GATT_IF_NONE, not specify a certain gatt_if, need to call every profile cb function */
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
                    bt_conn = 2;
                    vesync_bt_advertise_stop();
                    xEventGroupSetBits(user_event_group, BT_CONNECTED_BIT);
                    xEventGroupClearBits(user_event_group, BT_DIS_CONNECTED_BIT);

                    resend_cmd_bit |= RESEND_CMD_BT_STATUS_BIT;
                    uart_encode_send(MASTER_SET,CMD_BT_STATUS,(char *)&bt_conn,sizeof(uint8_t),true);
                break;
            case BT_DISCONNTED:
                    bt_conn = 0;
                    vesync_bt_advertise_start(APP_ADV_TIMEOUT_IN_SECONDS);
                    xEventGroupSetBits(user_event_group, BT_DIS_CONNECTED_BIT);
                    xEventGroupClearBits(user_event_group, BT_CONNECTED_BIT);

                    resend_cmd_bit |= RESEND_CMD_BT_STATUS_BIT;
                    uart_encode_send(MASTER_SET,CMD_BT_STATUS,(char *)&bt_conn,sizeof(uint8_t),true);
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
 * @param timer 
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
 * @brief 蓝牙接收队列任务处理，满足长度>10开始解析
 * @param pvParameters 
 */
static void vesync_bt_handler_loop(void *pvParameters)
{
    size_t len = 0;
    for(;;){
        xEventGroupWaitBits(user_event_group, BT_CONNECTED_BIT,
            false, true, portMAX_DELAY);	//第二个false 不用等待所有的bit都要置位;

		if(xRingbufferGetMaxItemSize(ringbuf_write) - xRingbufferGetCurFreeSize(ringbuf_write) >=10){
            uint8_t *databuf;
			databuf = xRingbufferReceive(ringbuf_write,&len,portMAX_DELAY);
            ESP_LOGI(GATTS_TABLE_TAG, "vesync_bt_handler_loop %d",len);
            if(databuf == NULL){
                ESP_LOGI(GATTS_TABLE_TAG,"Test: Timeout on recv!");
            }else if(len == 0){
                ESP_LOGI(GATTS_TABLE_TAG,"End packet received.\n");
                vRingbufferReturnItem(ringbuf_write, databuf);
            }else{
                esp_log_buffer_hex(GATTS_TABLE_TAG,(char *)(databuf),len);
                if(m_bt_handler != NULL){
                    m_bt_handler((hw_info *)&info_str,(char *)databuf, len);    //接收app数据并赋值给info_str
                }
                vRingbufferReturnItem(ringbuf_write, databuf);
            }
        }else{
            vTaskDelay(200 / portTICK_PERIOD_MS);	//避免任务一直占用看门狗复位；
            esp_task_wdt_reset();
        }
    }
}

/**
 * @brief 
 */
void vesync_bt_deinit(void)
{
    advertise_timer_delete();
    ESP_ERROR_CHECK(esp_ble_gatts_app_unregister(ESP_APP_ID));
    ESP_ERROR_CHECK(esp_bluedroid_disable());
    ESP_ERROR_CHECK(esp_bluedroid_deinit());
    ESP_ERROR_CHECK(esp_bt_controller_disable());
}
/**
 * @brief  初始化蓝牙硬件配置
 * @param  用户接口回调
 * @return 无
 */
void vesync_bt_init(bt_recv_cb_t cb)
{
    esp_err_t ret;

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        ESP_LOGE(GATTS_TABLE_TAG, "%s enable controller failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) {
        ESP_LOGE(GATTS_TABLE_TAG, "%s enable controller failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    ret = esp_bluedroid_init();
    if (ret) {
        ESP_LOGE(GATTS_TABLE_TAG, "%s init bluetooth failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    ret = esp_bluedroid_enable();
    if (ret) {
        ESP_LOGE(GATTS_TABLE_TAG, "%s enable bluetooth failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    ret = esp_ble_gatts_register_callback(gatts_event_handler);
    if (ret){
        ESP_LOGE(GATTS_TABLE_TAG, "gatts register error, error code = %x", ret);
        return;
    }

    ret = esp_ble_gap_register_callback(vesync_bt_gap_event_handler);
    if (ret){
        ESP_LOGE(GATTS_TABLE_TAG, "gap register error, error code = %x", ret);
        return;
    }

    ret = esp_ble_gatts_app_register(ESP_APP_ID);
    if (ret){
        ESP_LOGE(GATTS_TABLE_TAG, "gatts app register error, error code = %x", ret);
        return;
    }

    esp_err_t local_mtu_ret = esp_ble_gatt_set_local_mtu(BLE_MAX_MTU);
    if (local_mtu_ret){
        ESP_LOGE(GATTS_TABLE_TAG, "set local  MTU failed, error code = %x", local_mtu_ret);
    }

    ret = esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT,TX_POWER);
    ESP_ERROR_CHECK(ret);

    ringbuf_write = xRingbufferCreate(RING_BUFF_LEN, RINGBUF_TYPE_BYTEBUF);
    ringbuf_read  = xRingbufferCreate(RING_BUFF_LEN, RINGBUF_TYPE_BYTEBUF);
    xTaskCreate(vesync_bt_handler_loop, "vesync_bt_handler_loop", 4096, NULL, 5, NULL);

    m_bt_handler = cb;
}

