/**
 * @file interface.c
 * @brief 温湿度计接口
 * @author WatWu
 * @date 2019-03-13
 */

#include "vesync_api.h"

#include "hygrothermograph.h"
#include "interface.h"

static const char* TAG = "interface";

/**
 * @brief 打包蓝牙应用层协议帧到数组
 * @param ctl       [协议控制码]
 * @param cmd       [命令码]
 * @param seq       [消息序列]
 * @param data      [数据域]
 * @param length    [数据域长度]
 * @param out_buff  [打包后的缓存数组]
 */
static void pack_ble_frame_to_buffer(uint8_t ctl, uint16_t cmd, uint8_t seq, uint8_t* data, uint16_t length, uint8_t* out_buff)
{
    uint32_t buff_len = length + 9;
    out_buff[0] = 0XA5;
    out_buff[1] = ctl;
    out_buff[2] = seq;
    *(uint16_t*)&out_buff[3] = length + 2;
    *(uint16_t*)&out_buff[5] = cmd;
    memcpy(&out_buff[7], data, length);

    uint8_t sum = 0;
    for(int i = 0; i < buff_len - 2; i++)
        sum += out_buff[i];

    out_buff[buff_len - 2] = sum;
    out_buff[buff_len - 1] = 0X5A;
}

/**
 * @brief 通过ble蓝牙发送自定义长度的数据，由于底层接口限制了一包只能发送20字节，此函数进行拆分发送
 * @param data      [待发送的数据]
 * @param data_len  [数据长度]
 * @return uint32_t [发送结果]
 */
static uint32_t ble_send_custom_data(uint8_t* data, uint16_t data_len)
{
    uint16_t residue_len = data_len;
    uint16_t send_len = 0;
    uint32_t ret;

    while(1)
    {
        if(residue_len > 20)
        {
            ret = vesync_bt_notify_send(&data[send_len], 20);
            if(ret != 0)
                return ret;
            residue_len -= 20;
            send_len += 20;
        }
        else
        {
            ret = vesync_bt_notify_send(&data[send_len], residue_len);
            if(ret != 0)
                return ret;
            break;
        }
        usleep(10);         //暂时使用延时来等待上一包数据发送完成
    }

    return ret;
}

/**
 * @brief 更新温湿度值到APP，通过ble蓝牙
 */
void update_temp_humi_to_app(void)
{
    static uint8_t seq = 0;
    seq++;

    frame_ctrl_t send_ctl = {ACTIVE_REPORT_CTL};        //发送数据格式码
    temp_humi_data_t send_data = {0};
    uint16_t send_cmd = REPORT_TEMP_HUMI;

    time_t seconds;
    seconds = time((time_t *)NULL);

    send_data.temp = (int16_t)(get_device_temperature() * 10);
    send_data.humi = (int16_t)(get_device_humidity() * 10);
    send_data.timestamp = (uint32_t)seconds;

    vesync_bt_notify(send_ctl, &seq, send_cmd, (unsigned char *)&send_data, sizeof(temp_humi_data_t));
    LOG_I(TAG, "Update temp humi to app !");
}

/**
 * @brief 回复APP温湿度计历史记录数据
 */
void reply_temp_humi_history_to_app(void)
{
    static uint8_t seq = 0;
    seq++;                      //每次发送包序列自增1

    uint8_t history_buff[sizeof(temp_humi_history_t)] = {0};
    uint16_t history_len;
    int ret = vesync_flash_read(HUMITURE_FLASH_LABEL, HUMITURE_HISTORY_KEY, history_buff, &history_len);
    if(ret == 0)
    {
        LOG_I(TAG, "Get flash data !");
        uint8_t send_data[sizeof(temp_humi_history_t) + 9] = {0};
        pack_ble_frame_to_buffer(ACK_WITH_NO_ERROR, QUERY_HISTORY, seq, history_buff, history_len, send_data);
        ble_send_custom_data(send_data, history_len + 9);
    }
    else
    {
        LOG_E(TAG, "Read flash error, ret=%d", ret);
        frame_ctrl_t send_ctl = {ACK_WITH_NO_ERROR};        //发送数据格式码
        uint16_t send_cmd = QUERY_HISTORY;
        uint8_t send_data[2] = {0};
        uint16_t send_len = 2;
        vesync_bt_notify(send_ctl, &seq, send_cmd, (unsigned char *)&send_data, send_len);
    }
}

/**
 * @brief 温湿度计清空历史数据
 * @return uint32_t [处理结果]
 */
uint32_t device_clear_temp_humi_history(void)
{
    static uint8_t seq = 0;
    seq++;                      //每次发送包序列自增1

    uint32_t ret = -1;
    ret = vesync_flash_erase_key(HUMITURE_FLASH_LABEL, HUMITURE_HISTORY_KEY);
    if(ret == 0)                    //擦除成功
    {
        frame_ctrl_t send_ctl = {ACK_WITH_NO_ERROR};
        uint16_t send_cmd = CLEAR_HISTORY;
        vesync_bt_notify(send_ctl, &seq, send_cmd, NULL, 0);
    }
    else
    {
        LOG_E(TAG, "Erase history failed !");
        frame_ctrl_t send_ctl = {ACK_WITH_ERROR};
        uint16_t send_cmd = CLEAR_HISTORY;
        uint8_t ack = 0x05;
        vesync_bt_notify(send_ctl, &seq, send_cmd, &ack, 1);
    }

    return ret;
}

/**
 * @brief 回复APP要查询的温湿度计预警值设置参数
 * @param warn_setting  [温湿度预警值设置]
 * @return uint32_t     [回复结果]
 */
uint32_t reply_early_warning_setting_to_app(hygrother_warning_t* warn_setting)
{
    static uint8_t seq = 0;
    seq++;                      //每次发送包序列自增1

    uint32_t ret = -1;
    //此处保存设置值到flash

    //h回复APP
    frame_ctrl_t send_ctl = {ACK_WITH_NO_ERROR};
    uint16_t send_cmd = QUERY_PREWARNING;
    vesync_bt_notify(send_ctl, &seq, send_cmd, NULL, 0);

    return ret;
}

/**
 * @brief APP设置温湿度预警值
 * @param warn_setting  [温湿度预警值结构体]
 * @return uint32_t     [设置结果]
 */
uint32_t app_set_early_warning_setting(hygrother_warning_t* warn_setting)
{
    static uint8_t seq = 0;
    seq++;                      //每次发送包序列自增1

    uint32_t ret = -1;
    frame_ctrl_t send_ctl = {ACK_WITH_NO_ERROR};
    uint16_t send_cmd = SET_PREWARNING;

    ret =  vesync_bt_notify(send_ctl, &seq, send_cmd, (uint8_t*)warn_setting, sizeof(hygrother_warning_t));

    return ret;
}

/**
 * @brief 回复APP要查询的温湿度值
 */
void reply_temp_humi_to_app(void)
{
    static uint8_t seq = 0;
    seq++;

    frame_ctrl_t send_ctl = {ACK_WITH_NO_ERROR};
    temp_humi_data_t send_data = {0};
    uint16_t send_cmd = QUERY_TEMP_HUMI;

    time_t seconds;
    seconds = time((time_t *)NULL);

    send_data.temp = (int16_t)(get_device_temperature() * 10);
    send_data.humi = (int16_t)(get_device_humidity() * 10);
    send_data.timestamp = (uint32_t)seconds;
    vesync_bt_notify(send_ctl, &seq, send_cmd, (unsigned char *)&send_data, sizeof(temp_humi_data_t));
}

/**
 * @brief 上传温湿度历史记录到服务器
 * @param history_data [温湿度历史数据]
 */
void upload_temp_humi_history_to_server(temp_humi_history_t* history_data)
{
    cJSON *root = NULL;
    cJSON *info = NULL;

    root = cJSON_CreateObject();
    if(NULL != root)
    {
        cJSON_AddItemToObject(root, "info", info = cJSON_CreateArray());
        if(NULL != info)
        {
            uint16_t amount = history_data->history_amount;
            if(amount > 0)
            {
                for(uint16_t i = 0; i < amount; i++)
                {
                    cJSON *data_root = cJSON_CreateObject();
                    if(NULL != data_root)
                    {
                        cJSON_AddNumberToObject(data_root, "temp", history_data->history_list[i].temp);
                        cJSON_AddNumberToObject(data_root, "humidity", history_data->history_list[i].humi);
                        cJSON_AddNumberToObject(data_root, "sampleTime", history_data->history_list[i].timestamp);
                        cJSON_AddItemToArray(info, data_root);
                    }
                }
            }
        }
    }

    time_t seconds;
    seconds = time((time_t *)NULL);
    char traceId_buf[64];
    itoa(seconds, traceId_buf, 10);
    cJSON *report = vesync_json_add_method_head(traceId_buf, "addHumitureData", info);

    char token[128];
    vesync_get_https_token(token);
    cJSON_AddStringToObject(report, "token", token);
    char* out = cJSON_PrintUnformatted(report);
    LOG_I("JSON", "%s", out);

    char* recv_buff = malloc(512);
    if(NULL != recv_buff)
    {
        int recv_len = 512;
        vesync_https_client_request("humiture/add", out, recv_buff, &recv_len, 5 * 1000);
        LOG_I(TAG, "Server reply : %s", recv_buff);
        free(recv_buff);
    }
    else
        LOG_E(TAG, "Recv buff is null !");

    free(out);
    cJSON_Delete(report);
    cJSON_Delete(root);
}

/**
 * @brief 发送温湿度预警消息到服务器
 * @param warn_val  [发生预警的上限或下限值]
 * @param curr_val  [当前值]
 * @param warn_type [预警类型，1 - 温度超上限；2 - 温度超下限； 3 - 湿度超上限；4 - 湿度超下限]
 * @param timestamp [发生预警时的时间]
 */
void send_early_warning_to_server(float warn_val, float curr_val, int warn_type, uint32_t timestamp)
{
    cJSON *root = NULL;
    cJSON *info = NULL;

    root = cJSON_CreateObject();
    if(NULL != root)
    {
        cJSON_AddItemToObject(root, "info", info = cJSON_CreateObject());
        if(NULL != info)
        {
            cJSON_AddNumberToObject(info, "warningValue", warn_val);
            cJSON_AddNumberToObject(info, "currentValue", curr_val);
            cJSON_AddNumberToObject(info, "warningType", warn_type);
            cJSON_AddNumberToObject(info, "warningTime", timestamp);
        }
    }

    time_t seconds;
    seconds = time((time_t *)NULL);
    char traceId_buf[64];
    itoa(seconds, traceId_buf, 10);
    cJSON *report = vesync_json_add_method_head(traceId_buf, "pushHumitureWarningInfo", info);

    char token[128];
    vesync_get_https_token(token);
    cJSON_AddStringToObject(report, "token", token);
    char* out = cJSON_PrintUnformatted(report);
    LOG_I("JSON", "%s", out);

    char* recv_buff = malloc(512);
    if(NULL != recv_buff)
    {
        int recv_len = 512;
        vesync_https_client_request("humiture/earlyWarningInfo", out, recv_buff, &recv_len, 5 * 1000);
        LOG_I(TAG, "Server reply : %s", recv_buff);
        free(recv_buff);
    }
    else
        LOG_E(TAG, "Recv buff is null !");

    free(out);
    cJSON_Delete(report);
    cJSON_Delete(root);
}
