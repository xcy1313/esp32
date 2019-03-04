/**
 * @file vesync_user.c
 * @brief vesync平台应用层
 * @author WatWu
 * @date 2018-09-04
 */

#include <string.h>
#include <stdlib.h>

#include "vesync_log.h"
#include "vesync_api.h"

#include "vesync_wifi.h"
#include "vesync_net_service.h"
#include "vesync_production.h"
#include "vesync_flash.h"
#include "vesync_interface.h"
#include "vesync_ota.h"
#include "vesync_build_cfg.h"
#include "vesync_button.h"
#include "vesync_device.h"
#include "vesync_developer.h"

static const char* TAG = "vesync_user";

/**
 * @brief 接收json数据回调函数
 * @param data [原始格式的json数据]
 */
static void vesync_recv_json_data(char *data)
{
    LOG_I(TAG, "User recv : %s", data);
    cJSON *root = cJSON_Parse(data);
    if(NULL == root)
    {
        LOG_E(TAG, "Parse cjson error !");
        return;
    }
    vesync_printf_cjson(root);

    cJSON *jsonCmd = cJSON_GetObjectItemCaseSensitive(root, "jsonCmd");
    if(true == cJSON_IsObject(jsonCmd))
    {
        cJSON *cid = cJSON_GetObjectItemCaseSensitive(jsonCmd, "cid");
        if(true == cJSON_IsString(cid))
        {
            LOG_I(TAG, "Get cid success !");
            cJSON *report = cJSON_CreateObject();
            if(NULL != report)
            {
                time_t seconds;
                seconds = time((time_t *)NULL);
                char traceId_buf[64];
                itoa(seconds, traceId_buf, 10);
                cJSON_AddStringToObject(report, "traceId", traceId_buf);		//==TODO==，需要修改成毫秒级
                cJSON_AddNumberToObject(report, "code", 0);
                cJSON_AddStringToObject(report, "msg", "cid set ok");
            }
            char* out = cJSON_PrintUnformatted(report);
            LOG_I(TAG, "Response server : %s",out);
            vesync_response_production_command(out, MQTT_QOS1, 0);
            free(out);
            cJSON_Delete(report);
        }
        else
            LOG_E(TAG, "Get cid error !");

        cJSON_Delete(root);									//务必记得释放资源！
    }
    else
        LOG_E(TAG, "Get jsonCmd error !");
}

static void ota_event_handler(uint32_t len,vesync_ota_status_t status)
{
    switch(status){
        case OTA_TIME_OUT:
                LOG_I(TAG, "OTA_TIME_OUT");
            break;
        case OTA_BUSY:
                LOG_I(TAG, "OTA_BUSY");
            break;
        case OTA_PROCESS:
                LOG_I(TAG, "OTA_PROCESS ...%d",len);
            break;
        case OTA_FAILED:
                LOG_I(TAG, "OTA_FAILED");
            break;
        case OTA_SUCCESS:
                LOG_I(TAG, "OTA_SUCCESS");
            break;
        default:
            break;
    }
}

void ble_rec_handler(const unsigned char *data, unsigned char len)
{
    uint32_t ret;
    if(data[0] ==  1){
        ret = vesync_scan_wifi_list_start();
        LOG_I(TAG, "wifi scan ret = %d",ret);
    }
}

static void app_button_event_handler(void *p_event_data)
{
    LOG_I(TAG, "key pattern [%d]\r\n" ,*(uint8_t *)p_event_data);
    switch(*(uint8_t *)p_event_data){
        case Short_key:
            vesync_enter_production_testmode(NULL,NULL);
            //vesync_ota_init("http://192.168.16.25:8888/firmware-debug/esp32/vesync_sdk_esp32.bin",ota_event_handler);
            //vesync_refresh_https_token();
            break;
        default:
            break;
    }
}

static void device_status(device_status_e status)
{
    LOG_I(TAG, "device status %d\n",status);
}
/**
 * @brief vesync平台应用层入口函数
 */
void vesync_user_entry(void *args)
{
    LOG_I(TAG, "Application layer start !");
    LOG_E(TAG, "Application layer start version with[%s]",FIRM_VERSION);
    vesync_button_init(19,app_button_event_handler);
    vesync_bt_client_init(PRODUCT_NAME,PRODUCT_VER,"1116",PRODUCT_TYPE,PRODUCT_NUM,NULL,true,NULL,ble_rec_handler);
    vesync_bt_advertise_start(0);
    vesync_regist_devstatus_cb(device_status);
    vesync_developer_start();
    vesync_client_connect_wifi("R6100-2.4G", "12345678");
    // vesync_bt_dynamic_set_ble_advertise_name("esp32_test");
    // vesync_bt_dynamic_ble_advertise_para(0x88,0x88);
    //
    //vesync_regist_recvjson_cb(vesync_recv_json_data);
    
    vTaskDelete(NULL);
}
