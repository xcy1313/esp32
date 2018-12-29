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

#include "app_handle_server.h"
#include "app_handle_scales.h"

#include "esp_log.h"

static const char *TAG = "app_handle_server";

/**
 * @brief
 * @param pvParameters
 */
static void app_handle_server_task_handler(void *pvParameters)
{
    while(1)
    {

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

/**
 * @brief 上报产测时的按键测试次数
 * @param times [按键测试的次数]
 */
void app_handle_production_report_button(int times)
{
    cJSON *root = cJSON_CreateObject();
    if(NULL != root)
    {
        cJSON* info = NULL;
        cJSON_AddItemToObject(root, "info", info = cJSON_CreateObject());
        if(NULL != info)
        {
            cJSON_AddNumberToObject(info, "button", times);
            cJSON *report = vesync_json_add_method_head("updateDevInfo",info);
            vesync_printf_cjson(report);
            char* out = cJSON_PrintUnformatted(report);
            vesync_publish_production_data(out, MQTT_QOS1, 0);
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
void app_handle_production_report_charge(int charge_status, int power_percent)
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
            cJSON *report = vesync_json_add_method_head("updateDevInfo",info);
            vesync_printf_cjson(report);
            char* out = cJSON_PrintUnformatted(report);
            vesync_publish_production_data(out, MQTT_QOS1, 0);
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
void app_handle_production_report_weight(int weight, int imped)
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
            cJSON *report = vesync_json_add_method_head("updateDevInfo",info);
            vesync_printf_cjson(report);
            char* out = cJSON_PrintUnformatted(report);
            vesync_publish_production_data(out, MQTT_QOS1, 0);
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
void app_handle_production_response_bt_rssi(int rssi)
{
    cJSON *root = cJSON_CreateObject();
    if(NULL != root)
    {
        cJSON* info = NULL;
        cJSON_AddItemToObject(root, "info", info = cJSON_CreateObject());
        if(NULL != info)
        {
            cJSON_AddNumberToObject(info, "bt-rssi", rssi);
            cJSON *report = vesync_json_add_method_head("updateDevInfo",info);
            vesync_printf_cjson(report);
            char* out = cJSON_PrintUnformatted(report);
            vesync_response_production_command(out, MQTT_QOS1, 0);
            free(out);
            cJSON_Delete(report);
        }
    }
    cJSON_Delete(root);
}
