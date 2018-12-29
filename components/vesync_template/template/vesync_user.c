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

/**
 * @brief vesync平台应用层入口函数
 */
void vesync_user_entry(void *args)
{
    LOG_I(TAG, "Application layer start !");

    vesync_enter_production_testmode(NULL);
    vesync_regist_recvjson_cb(vesync_recv_json_data);

    vTaskDelete(NULL);
}
