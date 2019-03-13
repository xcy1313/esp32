/**
 * @file vesync_production.c
 * @brief 产测模式
 * @author WatWu
 * @date 2018-05-28
 */

#include <stdlib.h>

#include "vesync_log.h"
#include "vesync_wifi.h"
#include "vesync_bt_hal.h"
#include "vesync_net_service.h"
#include "vesync_production.h"
#include "vesync_flash.h"
#include "vesync_device.h"
#include "vesync_build_cfg.h"
#include "vesync_interface.h"

static const char* TAG = "vesync_production";

static production_status_e production_status = PRODUCTION_EXIT;
static production_status_cb_t production_status_cb = NULL;
static production_para_cb_t   production_para_cb = NULL;

// 产测交互的四个MQTT主题中，cid那一栏使用mac地址填充后主题字符串的最长长度为37字节
static char production_request_topic[40];
static char production_response_topic[40];
static char production_bypass_topic[40];
static char production_bypass_rsp_topic[40];

/**
 * @brief 获取产测状态
 * @return production_status_e [产测状态值]
 */
production_status_e vesync_get_production_status(void)
{
    return production_status;
}

/**
 * @brief 设置产测状态，并同步进行回调
 * @param status [产测状态值]
 */
void vesync_set_production_status(production_status_e status)
{
    production_status = status;
    if(NULL != production_status_cb)
    {
        production_status_cb(status);
    }
}

/**
 * @brief 进入产测模式
 * @param cb   [产测状态回调函数]
 */
void vesync_enter_production_testmode(production_para_cb_t para_cb,production_status_cb_t cb)
{
    production_status_cb = cb;
    production_para_cb = para_cb;

    vesync_client_connect_wifi(PRODUCTION_WIFI_SSID, PRODUCTION_WIFI_KEY);	// wifi driver初始化，否则无法获取mac地址
    vesync_set_production_status(RPODUCTION_START);

    LOG_I(TAG, "Production param :");
    LOG_I(TAG, "==============================");
    //产测时使用MAC地址代替CID用于产测通过前的mqtt连接
    char cid_mac[6 * 3];
    char username[64];
    vesync_get_wifi_sta_mac_string(cid_mac);
    sprintf(username, "%s%s", DEV_AUTHKEY, DEV_PID);
    LOG_I(TAG, "cid : %s", cid_mac);
    LOG_I(TAG, "username : %s", username);
    LOG_I(TAG, "server : %s:1883", PRODUCTION_SERVER_ADDR);
    LOG_I(TAG, "ssid : %s", PRODUCTION_WIFI_SSID);
    LOG_I(TAG, "pwd : %s", PRODUCTION_WIFI_KEY);
    LOG_I(TAG, "==============================");
    
    vesync_wait_network_connected(10000);
    //vesync_config_cloud_mqtt_client(cid_mac, TEST_MQTT_SERVER_ADDR, 61613, "etekcity", "hardware");
    vesync_config_cloud_mqtt_client_with_tls(cid_mac, PRODUCTION_SERVER_ADDR, 1883, username, "0");
    vesync_mqtt_client_connect_to_cloud();

    // 填充产测相关的主题buffer
    sprintf(production_request_topic, DEVICE_REQUEST_TOPIC, cid_mac);
    sprintf(production_response_topic, DEVICE_RESPONSE_TOPIC, cid_mac);
    sprintf(production_bypass_topic, CLOUD_BYPASS_TOPIC, cid_mac);
    sprintf(production_bypass_rsp_topic, CLOUD_BYPASS_RSP_TOPIC, cid_mac);
}

/**
 * @brief 订阅产测系统的mqtt主题
 * @return int 	[订阅结果]
 */
int vesync_subscribe_production_topic(void)
{
    int ret;
    ret = vesync_mqtt_subscribe_topic(vesync_get_cloud_mqtt_client(), production_response_topic, MQTT_QOS1);
    ret = vesync_mqtt_subscribe_topic(vesync_get_cloud_mqtt_client(), production_bypass_topic, MQTT_QOS1);
    return ret;
}

/**
 * @brief 发布产测相关的数据
 * @param data 		[数据内容指针]
 * @param qos 		[QoS]
 * @param retain 	[retain]
 * @return int 		[发布结果]
 */
int vesync_publish_production_data(char* data, int qos, int retain)
{
    int ret;
    ret = vesync_mqtt_publish_rawdata_to_topic(vesync_get_cloud_mqtt_client(), production_request_topic, data, strlen(data), qos, retain);
    return ret;
}

/**
 * @brief 响应回复产测服务器的指令
 * @param data 		[数据内容指针]
 * @param qos 		[QoS]
 * @param retain 	[retain]
 * @return int 		[发布结果]
 */
int vesync_response_production_command(char* data, int qos, int retain)
{
    int ret;
    ret = vesync_mqtt_publish_rawdata_to_topic(vesync_get_cloud_mqtt_client(), production_bypass_rsp_topic, data, strlen(data), qos, retain);
    return ret;
}

/**
 * @brief 产测系统连接成功后上报固定数据
 */
int vesync_production_connected_report_to_server(char *state)
{
    int ret;
    char read_buf[20]={"\0"};
    uint16_t mcu_version;
    //上电上报信息中携带设备mac地址
    char mac_string[6 * 3];
    vesync_get_wifi_sta_mac_string(mac_string);
    char ap_mac_string[6*3];
    vesync_get_wifi_ap_mac_string(ap_mac_string);
    char str_mcu_version[4];
    int rssi = vesync_get_ap_rssi(8);

    if(production_para_cb != NULL){
        production_para_cb(read_buf,2);
        mcu_version = *(uint16_t *)&read_buf[0];
        sprintf(str_mcu_version,"%04x",mcu_version);
        LOG_I(TAG, "mcu version[0x%04x]",mcu_version);
        LOG_I(TAG, "str mcu version[%s]",str_mcu_version);
    }
    cJSON *root = cJSON_CreateObject();
    if(NULL != root){
        cJSON *info = NULL;
        time_t seconds;
		seconds = time((time_t *)NULL);
		char traceId_buf[64];
		itoa(seconds, traceId_buf, 10);
        cJSON_AddStringToObject(root, "traceId", traceId_buf);
        cJSON_AddStringToObject(root, "method", "updateDevInfo");
        cJSON_AddStringToObject(root, "pid", DEV_PID);
        cJSON_AddStringToObject(root, "cid", mac_string);
        cJSON_AddItemToObject(root, "info", info = cJSON_CreateObject());
        if(NULL != info){
            cJSON *version = NULL;
            cJSON_AddStringToObject(info, "initState", state);
            cJSON_AddStringToObject(info, "mac", mac_string);
            cJSON_AddNumberToObject(info, "rssi", rssi);         //==TODO==，add by watwu
            cJSON_AddStringToObject(info, "routerMac", ap_mac_string);
            cJSON_AddStringToObject(info, "mcuVersion", str_mcu_version);
            cJSON_AddItemToObject(info, "version", version = cJSON_CreateObject());
            if(NULL != version){
                cJSON_AddStringToObject(version, "firmVersion", FIRM_VERSION);
            }
        }
    }else
        return -1;

    char* out = cJSON_PrintUnformatted(root);
    //发送设备状态
    ret = vesync_mqtt_publish_rawdata_to_topic(vesync_get_cloud_mqtt_client(), production_request_topic, out, strlen(out), MQTT_QOS1, 1);
    vesync_printf_cjson(root);
    free(out);    
    cJSON_Delete(root);

    return ret;
}

/**
 * @brief 解析cid主题信息
 * @param data
 */
static bool vesync_prase_topic_cid_packet(product_config_t *info, char *data)
{
    bool ret = false;
    cJSON *root = cJSON_Parse(data);
    if(NULL == root)
    {
        LOG_I(TAG, "Parse cjson error !\r\n");
        return ret;
    }

    cJSON* state = cJSON_GetObjectItemCaseSensitive(root, "state");
    if(state != NULL)
    {
        cJSON* cid = cJSON_GetObjectItemCaseSensitive(state, "cid");
        if(cJSON_IsString(cid))
        {
            if(strlen(cid->valuestring) == CID_LENGTH)
            {
                strcpy((char *)info->cid, cid->valuestring);
                LOG_I(TAG, "cid : %s\r\n", info->cid);

                //info->cid_holder = CID_HOLDER;
                if(vesync_flash_write_product_config(info) == 0)
                {
                    //set_production_status(RPODUCTION_TEST_PASS);

                    // 	vesync_resp_json_to_server(cid);
                    // }else{
                    // 	set_production_status(PRODUCTION_TEST_FAIL);
                    // 	LOG_I(TAG, "Save cid to flash fail !\r\n");
                    // 	LOG_I(TAG, "Erase sector ret : %d\r\n",erase_ret);
                    // 	LOG_I(TAG, "Write sector ret : %d\r\n",write_ret);

                    // 	cJSON *cid_error = cJSON_CreateObject();
                    // 	if(NULL != cid_error){
                    // 		cJSON_AddStringToObject(cid_error,"cid","error");
                    // 		cJSON* cid_report = cJSON_GetObjectItem( cid_error, "cid" );
                    // 		vesync_resp_json_to_server(cid_report);
                    // 	}
                    // 	cJSON_Delete(cid_error);
                }
            }
        }
    }
    cJSON_Delete(root);

    return true;
}

/**
 * @brief 解析产测系统数据流
 * @param topic [接收的主题]
 * @param data 	[接收的数据]
 */
void vesync_prase_production_json_packet(const char *topic, char *data)
{
    if(vesync_get_production_status() == PRODUCTION_EXIT) return;

    vesync_prase_topic_cid_packet(&product_config, data);
    // if(strcmp(topic, sub_Topic) == 0) 	//订阅的cid主题
    // {
    //     vesync_prase_topic_cid_packet(&product_config, data);
    // }
    // else
    // {

    // }
}
