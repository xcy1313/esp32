/**
 * @file app_public_events.c
 * @brief 处理和网络对接相关
 * @author Jiven
 * @date 2018-12-21
 */

#include "vesync_flash.h"
#include "vesync_production.h"
#include "vesync_net_service.h"
#include "vesync_interface.h"
#include "vesync_wifi.h"
#include "vesync_ota.h"

#include "app_handle_phone.h"
#include "app_handle_server.h"
#include "app_handle_scales.h"
#include <time.h>

#include "vesync_ca_cert.h"
#include "vesync_https.h"

#include "vesync_build_cfg.h"
#include "vesync_log.h"

static const char *TAG = "app_handle_server";
static char trace_time[14] = {'\0'};
static char production_upgrade_url[128] = {'\0'};
static char new_version[8] = {'\0'};
factory_test_bit_t factory_test_bit = 0;

#define HTTPS_QUEUE_LEN 300
typedef struct{
    uint32_t bit;
    uint16_t len;
    uint8_t  buff[HTTPS_QUEUE_LEN];
}https_send_frame_t;

static xQueueHandle https_message_send_queue = NULL;

static TaskHandle_t s_network_service_taskhd = NULL;

static device_net_status_t device_net_status = NET_CONFNET_NOT_CON;		//设备配网状态，默认为离线状态

void app_handle_production_upgrade_response_ack(char *trace_id);
void app_handle_production_cid_response(void);

static uint8_t upgrade_souce = UPGRADE_APP;

/**
 * @brief 设置升级来源
 * @param source 
 */
void app_set_upgrade_source(uint8_t source)
{
    upgrade_souce = source;
}
/**
 * @brief 获取升级来源
 * @return uint8_t 
 */
uint8_t app_get_upgrade_source(void)
{
    return upgrade_souce;
}
/**
 * @brief 返回trace_id
 * @param time 
 */
char *vesync_get_time(void)
{
    return &trace_time[0];
}
/**
 * @brief 接收json数据回调函数
 * @param data [原始格式的json数据]
 */
void vesync_recv_json_data(char *data)
{
    LOG_I(TAG, "User recv : %s", data);
    cJSON *root = cJSON_Parse(data);
    if(NULL == root){
        LOG_E(TAG, "Parse cjson error !");
        return;
    }
    vesync_printf_cjson(root);

    cJSON *trace_Id = cJSON_GetObjectItemCaseSensitive(root, "traceId");
    if(true == cJSON_IsString(trace_Id)){
        strcpy(trace_time, trace_Id->valuestring);
        LOG_I(TAG, "trase_Id %s", trace_time);
    }else{
        LOG_E(TAG, "prase trase_Id error");
    }
    
    cJSON *jsonCmd = cJSON_GetObjectItemCaseSensitive(root, "jsonCmd");
    if(true == cJSON_IsObject(jsonCmd))
    {
        cJSON *switch_item = cJSON_GetObjectItemCaseSensitive(jsonCmd, "switch");
        if(true == cJSON_IsString(switch_item)){
            if(!strcmp(switch_item->valuestring, "on")){
                LOG_I(TAG, "project test start!");
                vesync_set_production_status(RPODUCTION_RUNNING);   //状态调整为产测模式已开始;
                app_handle_production_response_ack(vesync_get_time(),PRODUCT_TEST_START);
            }
        }
        cJSON *button_item = cJSON_GetObjectItemCaseSensitive(jsonCmd, "button");
        if(true == cJSON_IsString(button_item)){
            if(!strcmp(button_item->valuestring, "on")){
                LOG_I(TAG, "button test start");
                factory_test_bit |= FACTORY_TEST_SYNC_BUTTON_BIT;
                //app_handle_production_report_button(trace_time,3);
            }
        }
        cJSON *charge_item = cJSON_GetObjectItemCaseSensitive(jsonCmd, "charge");
        if(true == cJSON_IsString(charge_item)){
            if(!strcmp(charge_item->valuestring, "on")){
                LOG_I(TAG, "charge test start!");
                factory_test_bit |= FACTORY_TEST_SYNC_CHARGE_BIT;
                resend_cmd_bit |= RESEND_CMD_FACTORY_CHARGE_BIT;
                app_uart_encode_send(MASTER_SET,CMD_FACTORY_CHARGING,0,0,true);
                //app_handle_production_report_charge(trace_time,1,80);
            }
        }
        cJSON *weight_item = cJSON_GetObjectItemCaseSensitive(jsonCmd, "weight");
        if(true == cJSON_IsString(weight_item)){
            if(!strcmp(weight_item->valuestring, "on")){
                LOG_I(TAG, "weight test start!");
                resend_cmd_bit |= RESEND_CMD_FACTORY_WEIGHT_BIT;
                factory_test_bit |= FACTORY_TEST_SYNC_WEIGHT_BIT;
                app_uart_encode_send(MASTER_SET,CMD_FACTORY_WEIGHT,0,0,true);
                //app_handle_production_report_weight(trace_time,10000,500);
            }
        }
        cJSON *bt_item = cJSON_GetObjectItemCaseSensitive(jsonCmd, "bt");
        if(true == cJSON_IsString(bt_item)){
            if(!strcmp(bt_item->valuestring, "on")){
                LOG_I(TAG, "bt test start!");
                factory_test_bit |= FACTORY_TEST_SYNC_BT_BIT;
                //app_handle_production_response_bt_rssi(trace_time,-10);
                app_product_ble_start();
            }
        }
        cJSON *firmware = cJSON_GetObjectItemCaseSensitive(jsonCmd, "firmware");
        if(firmware != NULL){
            LOG_I(TAG, "upgrade test start!");
            cJSON* newVersion = cJSON_GetObjectItemCaseSensitive(firmware, "newVersion");
            if(cJSON_IsString(newVersion)){
				strcpy(new_version, newVersion->valuestring);	//记录升级的新版本
                LOG_I(TAG, "upgrade new_version %s",new_version);
			}
            cJSON* url = cJSON_GetObjectItemCaseSensitive(firmware, "url");
            if(cJSON_IsString(url)){
				if(NULL != production_upgrade_url){
                    uint8_t url_len;
                    app_set_upgrade_source(UPGRADE_PRODUCTION);
					strcpy(production_upgrade_url, url->valuestring);
                    url_len = strlen(url->valuestring);
                    sprintf(&production_upgrade_url[url_len],"%s.V%s%s",PRODUCT_WIFI_NAME,new_version,".bin");
                    LOG_I(TAG, "upgrade url %s",production_upgrade_url);
                    app_handle_production_upgrade_response_ack(trace_time);
                    vesync_hal_bt_client_deinit();
                    vesync_ota_init(production_upgrade_url,ota_event_handler);
				}
			}
        }
        cJSON *cid = cJSON_GetObjectItemCaseSensitive(jsonCmd, "cid");
        if(true == cJSON_IsString(cid))
        {
            LOG_I(TAG, "Get cid success !");
            strcpy((char *)product_config.cid, cid->valuestring);
            if(vesync_flash_write_product_config(&product_config) == 0){
                app_handle_production_cid_response();
                vesync_set_production_status(PRODUCTION_EXIT);   //状态调整为产测模式已结束;
                resend_cmd_bit |= RESEND_CMD_FACTORY_STOP_BIT;
                app_uart_encode_send(MASTER_SET,CMD_FACTORY_SYNC_STOP,0,0,true);
            }
        }else
            LOG_E(TAG, "Get cid error !");
        
        cJSON_Delete(root);									//务必记得释放资源！
    }
    else
        LOG_E(TAG, "Get jsonCmd error !");
}

/**
 * @brief 主动发起升级结果
 * @param result 
 * @param trace_id 
 */
void app_handle_production_upgrade_response_result(char *trace_id,uint8_t result)
{
    cJSON *root = cJSON_CreateObject();
    if(NULL != root){
        cJSON* firmware = NULL;
        cJSON_AddItemToObject(root, "firmware", firmware = cJSON_CreateObject());
        if(NULL != firmware){
            cJSON_AddStringToObject(firmware, "newVersion",new_version);
            cJSON_AddNumberToObject(firmware, "status", result);
            cJSON_AddStringToObject(firmware, "url", production_upgrade_url);
        }
        cJSON *report = vesync_json_add_method_head(trace_id,"reportFirmUp",firmware);
        vesync_printf_cjson(report);
        char* out = cJSON_PrintUnformatted(report);
        vesync_publish_production_data(out, MQTT_QOS1, 0);
        free(out);
        cJSON_Delete(report);
    }
    cJSON_Delete(root);
}
/**
 * @brief 应答服务器升级开始
 * @param json 
 */
void app_handle_production_upgrade_response_ack(char *trace_id)
{
    cJSON *report = cJSON_CreateObject();
    LOG_I(TAG, "response trace_id : %s",trace_id);
    if(NULL != report){
        cJSON_AddStringToObject(report, "traceId", trace_id);		//==TODO==，需要修改成毫秒级
        cJSON_AddNumberToObject(report, "code", 0);
        cJSON_AddStringToObject(report, "msg", "firmware set ok");
    }
    vesync_printf_cjson(report);
    char* out = cJSON_PrintUnformatted(report);
    vesync_response_production_command(out, MQTT_QOS1, 0);
    free(out);

    cJSON_Delete(report);
}

/**
 * @brief 应答cid
 */
void app_handle_production_cid_response(void)
{
    cJSON *report = cJSON_CreateObject();
    if(NULL != report){
        cJSON_AddStringToObject(report, "traceId", trace_time);
        cJSON_AddNumberToObject(report, "code", 0);
        cJSON_AddStringToObject(report, "msg", "cid set ok");
    }
    char* out = cJSON_PrintUnformatted(report);
    LOG_I(TAG, "Response server : %s",out);
    vesync_response_production_command(out, MQTT_QOS1, 0);
    free(out);
    cJSON_Delete(report);
}

/**
 * @brief 应答服务器下发
 * @param test_item 
 */
void app_handle_production_response_ack(char *trace_id,uint8_t test_item)
{
    cJSON *root = cJSON_CreateObject();
    if(NULL != root)
    {
        cJSON* info = NULL;
        cJSON_AddItemToObject(root, "info", info = cJSON_CreateObject());
        if(NULL != info)
        {
            switch(test_item){
                case PRODUCT_TEST_START:
                    cJSON_AddStringToObject(info, "switch", "start");
                    break;
                default:
                    break;
            }
            cJSON *report = vesync_json_add_method_head(trace_id,"updateDevInfo",info);
            vesync_printf_cjson(report);
            char* out = cJSON_PrintUnformatted(report);
            vesync_response_production_command(out, MQTT_QOS1, 0);
            free(out);
            cJSON_Delete(report);
        }
    }
    cJSON_Delete(root);
}

/**
 * @brief 上报产测时的按键测试次数
 * @param times [按键测试的次数]
 */
void app_handle_production_report_button(char *trace_id,int times)
{
    cJSON *root = cJSON_CreateObject();
    if(NULL != root)
    {
        cJSON* info = NULL;
        cJSON_AddItemToObject(root, "info", info = cJSON_CreateObject());
        if(NULL != info)
        {
            cJSON_AddNumberToObject(info, "button", times);
            cJSON *report = vesync_json_add_method_head(trace_id,"updateDevInfo",info);
            vesync_printf_cjson(report);
            char* out = cJSON_PrintUnformatted(report);
            vesync_response_production_command(out, MQTT_QOS1, 0);
            free(out);
            cJSON_Delete(report);
        }
    }
    cJSON_Delete(root);
}

/**
 * @brief 上报产测时的充电状态
 * @param charge_status [充电状态，1 - 表示正在充电；0 - 表示未充电]
 * @param power_percent [当前电量百分比数值]
 */
void app_handle_production_report_charge(char *trace_id,int charge_status, int power_percent)
{
    cJSON *root = cJSON_CreateObject();
    if(NULL != root)
    {
        cJSON* info = NULL;
        cJSON_AddItemToObject(root, "info", info = cJSON_CreateObject());
        if(NULL != info)
        {
            cJSON_AddNumberToObject(info, "charge", charge_status);
            cJSON_AddNumberToObject(info, "percent", power_percent);
            cJSON *report = vesync_json_add_method_head(trace_id,"updateDevInfo",info);
            vesync_printf_cjson(report);
            char* out = cJSON_PrintUnformatted(report);
            vesync_response_production_command(out, MQTT_QOS1, 0);
            free(out);
            cJSON_Delete(report);
        }
    }
    cJSON_Delete(root);
}

/**
 * @brief 上报产测时的称重结果
 * @param weight [实时称重的结果，单位kg]
 * @param imped  [实时称重的阻抗，单位欧姆]
 */
void app_handle_production_report_weight(char *trace_id,int weight, int imped)
{
    cJSON *root = cJSON_CreateObject();
    if(NULL != root)
    {
        cJSON* info = NULL;
        cJSON_AddItemToObject(root, "info", info = cJSON_CreateObject());
        if(NULL != info)
        {
            cJSON_AddNumberToObject(info, "weight", weight);
            cJSON_AddNumberToObject(info, "imped", imped);
            cJSON *report = vesync_json_add_method_head(trace_id,"updateDevInfo",info);
            vesync_printf_cjson(report);
            char* out = cJSON_PrintUnformatted(report);
            vesync_response_production_command(out, MQTT_QOS1, 0);
            free(out);
            cJSON_Delete(report);
        }
    }
    cJSON_Delete(root);
}

/**
 * @brief 响应回复服务器蓝牙的rssi值
 * @param rssi [蓝牙信号的rssi值]
 */
void app_handle_production_response_bt_rssi(char *trace_id,int rssi)
{
    cJSON *root = cJSON_CreateObject();
    if(NULL != root)
    {
        cJSON* info = NULL;
        cJSON_AddItemToObject(root, "info", info = cJSON_CreateObject());
        if(NULL != info)
        {
            cJSON_AddNumberToObject(info, "bt-rssi", rssi);
            cJSON *report = vesync_json_add_method_head(trace_id,"updateDevInfo",info);
            vesync_printf_cjson(report);
            char* out = cJSON_PrintUnformatted(report);
            vesync_response_production_command(out, MQTT_QOS1, 0);
            free(out);
            cJSON_Delete(report);
        }
    }
    cJSON_Delete(root);
}

/**
 * @brief 获取当前设备配网状态
 * @return device_net_status_t 
 */
device_net_status_t app_handle_get_net_status(void)
{
    return device_net_status;
}

/**
 * @brief 设置当前配网状态
 * @param new_status 
 */
void app_handle_set_net_status(device_net_status_t new_status)
{
    if(new_status != device_net_status){
        device_net_status = new_status;
    }
}

/**
 * @brief 应用层网络服务通知
 */
void app_handle_net_service_task_notify_bit(uint32_t bit ,void *data,uint16_t len)
{
    https_send_frame_t https_send_frame ={0};

    //vesync_client_connect_wifi((char *)net_info.station_config.wifiSSID, (char *)net_info.station_config.wifiPassword);
    if(len !=0){
        https_send_frame.bit = bit;
        https_send_frame.len = len;
        memcpy((char *)https_send_frame.buff,(char *)data,len);
        xQueueSend(https_message_send_queue,&https_send_frame,portTICK_PERIOD_MS);
    }
    LOG_I(TAG, "app_handle_server_task_handler send bit[0x%08x]", bit);
    xTaskNotify(s_network_service_taskhd, bit, eSetBits);			//通知事件处理中心任务
}

static void vesync_json_add_https_req(void *databuf ,uint16_t len,uint32_t mask)
{
    cJSON *root = cJSON_CreateObject();
    char *req_method = NULL;
    cJSON* info = NULL;
    static char token[128];

    int ret =0;
    char recv_buff[1024];
    int buff_len = sizeof(recv_buff);

    if(NULL == root)    return;
    
    switch(mask){
        case NETWORK_CONFIG_REQ:
            cJSON_AddItemToObject(root, "info", info = cJSON_CreateObject());
            if(NULL != info){
                char mac[6 * 3];
                vesync_get_wifi_sta_mac_string(mac);
                cJSON_AddStringToObject(info, "configKey", (char *)net_info.mqtt_config.configKey);
                cJSON_AddStringToObject(info, "accountID", (char *)net_info.station_config.account_id);
                cJSON_AddStringToObject(info, "mac", mac);
                cJSON_AddStringToObject(info, "version", FIRM_VERSION);
                cJSON_AddNumberToObject(info, "rssi", -56);
                cJSON_AddStringToObject(info, "timeZone", "8");
                req_method = "deviceRegister";
            }
            break;
        case REFRESH_TOKEN_REQ:
                req_method = "refreshDeviceToken";
            break;
        case UPLOAD_WEIGHT_DATA_REQ:
                req_method = "uploadWeighData";
                cJSON_AddItemToObject(root, "info", info = cJSON_CreateObject());
                esp_log_buffer_hex(TAG, (char *)databuf, len);
                user_history_t history = {0};
                memcpy((uint8_t *)&history ,databuf,sizeof(user_history_t));
                
                if(NULL != info){
                    cJSON* user_weight = NULL;
                    cJSON* data = NULL;
                    cJSON_AddStringToObject(info, "accountID", (char *)net_info.station_config.account_id);
                    cJSON_AddItemToObject(info,"data",data = cJSON_CreateArray());
                    if(NULL != data){
                        user_weight = cJSON_CreateObject();
                        if(NULL != user_weight){
                            cJSON_AddItemToObject(user_weight,"weigh_kg",cJSON_CreateNumber(history.weight_kg));
                            cJSON_AddItemToObject(user_weight,"weigh_lb",cJSON_CreateNumber(history.weight_lb));
                            cJSON_AddItemToObject(user_weight,"impedence",cJSON_CreateNumber(history.imped_value));
                            cJSON_AddItemToObject(user_weight,"weighTime",cJSON_CreateNumber(history.utc_time));
                            cJSON_AddItemToObject(user_weight,"unit",cJSON_CreateString(history.measu_unit));
                            cJSON_AddItemToObject(user_weight,"timezone15m",cJSON_CreateString(history.time_zone));

                            // cJSON_AddItemToObject(user_weight,"weigh_kg",history.weight_kg);
                            // cJSON_AddItemToObject(user_weight,"weigh_lb",history.weight_lb);
                            // cJSON_AddItemToObject(user_weight,"impedence",history.imped_value);
                            // cJSON_AddItemToObject(user_weight,"weighTime",history.utc_time);
                            // cJSON_AddItemToObject(user_weight,"unit",history.measu_unit);
                            // cJSON_AddItemToObject(user_weight,"timezone15m",history.time_zone);

                        }
                        cJSON_AddItemToArray(data, user_weight);
                    }
                }
            break;
        case UPGRADE_ADDR_REQ:
                req_method = "getFirmAddr";
                cJSON_AddItemToObject(root, "info", info = cJSON_CreateObject());
                if(NULL != info){
                    cJSON_AddStringToObject(info, "firmVersion", FIRM_VERSION);
                    cJSON_AddStringToObject(info, "currentVersion", FIRM_VERSION);
                }
            break;
        case REFRESH_DEVICE_ATTRIBUTE:
                req_method = "updateDevInfo";
                cJSON_AddItemToObject(root, "info", info = cJSON_CreateObject());
                if(NULL != info){
                    cJSON* version = NULL; 
                    cJSON_AddStringToObject(info, "rssi", "-45");
                    cJSON_AddItemToObject(info, "version", version = cJSON_CreateObject());
                    if(NULL != version){
                        cJSON_AddStringToObject(version, "firmVersion", FIRM_VERSION);
                    }
                }
            break;
        default:
            break;
    }
    cJSON *report = vesync_json_add_method_head("1540170843000",req_method,info);
    if(mask != REFRESH_TOKEN_REQ && mask != NETWORK_CONFIG_REQ){
        cJSON_AddStringToObject(report, "token", token);
    }
    char* out = cJSON_PrintUnformatted(report);
    LOG_I("JSON", "\n%s", out);

    //if(app_handle_get_net_status() > NET_CONFNET_NOT_CON)
    {
        strcpy((char *)net_info.station_config.server_url,"test-online.vesync.com");
        LOG_I(TAG, "servel url %s",net_info.station_config.server_url);
        LOG_I(TAG, "servel account_id %s",net_info.station_config.account_id);

        //vesync_set_https_server_address((char *)net_info.station_config.server_url);
        ret = vesync_https_client_request(req_method, out, recv_buff, &buff_len, 2 * 1000);
        if(buff_len > 0 && ret == 0){
            LOG_I(TAG, "Https recv %d byte data : \n%s", buff_len, recv_buff);
        }
    }

    free(out);
    cJSON_Delete(report);
    cJSON_Delete(root);
}

/**
 * @brief
 * @param pvParameters
 */
static void app_handle_server_task_handler(void *pvParameters){
    BaseType_t notified_ret;
	uint32_t notified_value;
    https_send_frame_t send_frame ={0};

    while(1){
        notified_ret = xTaskNotifyWait(0x00000000, 0xFFFFFFFF, &notified_value, 10000 / portTICK_RATE_MS);
        if(1 == notified_ret){
            // if(https_message_send_queue !=0){
            //     if(xQueueReceive(https_message_send_queue, &send_frame, portMAX_DELAY)) {
            //         esp_log_buffer_hex(TAG,send_frame.buff,send_frame.len);
            //     }
            // }
            if(notified_value & NETWORK_CONFIG_REQ){
                LOG_I(TAG, "NETWORK_CONFIG_REQ");
                vesync_json_add_https_req(send_frame.buff ,send_frame.len,NETWORK_CONFIG_REQ);
            }
            if(notified_value & REFRESH_TOKEN_REQ){
                LOG_I(TAG, "REFRESH_TOKEN_REQ");
                vesync_json_add_https_req(send_frame.buff ,send_frame.len,REFRESH_TOKEN_REQ);
            }
            if(notified_value & UPLOAD_WEIGHT_DATA_REQ){
                LOG_I(TAG, "UPLOAD_WEIGHT_DATA_REQ");
                if(send_frame.bit == UPLOAD_WEIGHT_DATA_REQ){
                    vesync_json_add_https_req(send_frame.buff ,send_frame.len,UPLOAD_WEIGHT_DATA_REQ);
                }
            }
            if(notified_value & UPGRADE_ADDR_REQ){
                LOG_I(TAG, "UPGRADE_ADDR_REQ");
                vesync_json_add_https_req(send_frame.buff ,send_frame.len,UPGRADE_ADDR_REQ);
            }
            if(notified_value & REFRESH_DEVICE_ATTRIBUTE){
                LOG_I(TAG, "REFRESH_DEVICE_ATTRIBUTE");
                vesync_json_add_https_req(send_frame.buff ,send_frame.len,REFRESH_DEVICE_ATTRIBUTE);
            }
        }
    }
    //vTaskDelete(NULL);
}
/**
 * @brief 创建https数据发送队列
 */
static void app_handle_https_message_queue_create(void)
{
    https_message_send_queue = xQueueCreate(5, sizeof(https_send_frame_t));
    if(https_message_send_queue == 0){
        ESP_LOGE(TAG, "create https message fail!");
    }
}

/**
 * @brief app端创建通知任务
 */
void app_hadle_server_create(void)
{
    vesync_flash_config(true, USER_HISTORY_DATA_NAMESPACE);//初始化用户沉淀数据flash区域
    vesync_init_https_module(vesync_https_ca_cert_pem);
    app_handle_https_message_queue_create();
    xTaskCreate(app_handle_server_task_handler, "app_handle_server_task_handler", 8192, NULL, 10, &s_network_service_taskhd);
}
