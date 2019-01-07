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

#include "vesync_log.h"

static const char *TAG = "app_handle_server";
static char trace_time[14] = {'\0'};
char upgrade_url[128] = {'\0'};
char new_version[10] = {'\0'};

static device_net_status_t device_net_status = NET_CONFNET_NOT_CON;		//设备配网状态，默认为离线状态

void app_handle_production_upgrade_response_ack(char *trace_id);
void app_handle_production_cid_response(void);

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
                //app_handle_production_report_button(trace_time,3);
            }
        }
        cJSON *charge_item = cJSON_GetObjectItemCaseSensitive(jsonCmd, "charge");
        if(true == cJSON_IsString(charge_item)){
            if(!strcmp(charge_item->valuestring, "on")){
                LOG_I(TAG, "charge test start!");
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
                app_uart_encode_send(MASTER_SET,CMD_FACTORY_WEIGHT,0,0,true);
                //app_handle_production_report_weight(trace_time,10000,500);
            }
        }
        cJSON *bt_item = cJSON_GetObjectItemCaseSensitive(jsonCmd, "bt");
        if(true == cJSON_IsString(bt_item)){
            if(!strcmp(bt_item->valuestring, "on")){
                LOG_I(TAG, "bt test start!");
                //app_handle_production_response_bt_rssi(trace_time,-10);
                app_product_ble_start();
            }
        }
        cJSON *firmware = cJSON_GetObjectItemCaseSensitive(jsonCmd, "firmware");
        if(firmware != NULL){
            LOG_I(TAG, "upgrade test start!");
            vesync_set_production_status(RPODUCTION_RUNNING);   //状态调整为产测模式已开始;
            cJSON* newVersion = cJSON_GetObjectItemCaseSensitive(firmware, "newVersion");
            if(cJSON_IsString(newVersion)){
				strcpy(new_version, newVersion->valuestring);	//记录升级的新版本
                LOG_I(TAG, "upgrade new_version %s",new_version);
			}
            cJSON* url = cJSON_GetObjectItemCaseSensitive(firmware, "url");
            if(cJSON_IsString(url)){
				if(NULL != upgrade_url){
					strcpy(upgrade_url, url->valuestring);
                    LOG_I(TAG, "upgrade url %s",upgrade_url);
                    if(strcmp(new_version,FIRM_VERSION) >0){
                        app_handle_production_upgrade_response_ack(trace_time);
                        vesync_ota_init(upgrade_url,ota_event_handler);
                    }
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
            cJSON_AddStringToObject(firmware, "url", upgrade_url);
        }
        cJSON *report = vesync_json_add_method_head(trace_id,"reporttFirmUp",firmware);
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
    if(NULL != report){
        cJSON_AddStringToObject(report, "traceId", trace_id);		//==TODO==，需要修改成毫秒级
        cJSON_AddNumberToObject(report, "code", 0);
        cJSON_AddStringToObject(report, "msg", "firmware set ok");
    }
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
 * @brief
 * @param pvParameters
 */
static void app_handle_server_task_handler(void *pvParameters){
    while(1){
        if(0 == vesync_wait_network_connected(3000)){

        }
    }
    vTaskDelete(NULL);
}
/**
 * @brief app端创建通知任务
 */
void app_hadle_server_create(void)
{
    vesync_flash_config(true, USER_HISTORY_DATA_NAMESPACE);//初始化用户沉淀数据flash区域
    //xTaskCreate(app_handle_server_task_handler, "app_handle_server_task_handler", 8192, NULL, 13, NULL);
}
