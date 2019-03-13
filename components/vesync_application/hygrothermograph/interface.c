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
    frame_ctrl_t send_ctl = {ACTIVE_REPORT_CTL};        //发送数据格式码
    temp_humi_data_t send_data = {0};

    uint16_t send_cmd = REPORT_TEMP_HUMI;
    uint16_t send_len = 8;

    send_data.temp = (int16_t)(get_device_temperature() * 10);
    send_data.humi = (int16_t)(get_device_humidity() * 10);
    send_data.timestamp = 1544410793;

    seq++;
    vesync_bt_notify(send_ctl, &seq, send_cmd, (unsigned char *)&send_data, send_len);
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
