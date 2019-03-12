/**
 * @file hygrothermograph.c
 * @brief 温湿度计设备功能接口
 * @author WatWu
 * @date 2019-03-11
 */

#include "esp32/ulp.h"
#include "ulp_adc.h"

#include "vesync_user.h"
#include "vesync_build_cfg.h"
#include "vesync_sntp_service.h"

#include "sht30.h"
#include "vadisplay.h"
#include "buzzer.h"
#include "analog_param.h"
#include "touchkey.h"
#include "low_power.h"
#include "hygrothermograph.h"

#define BT_PARSE_CHAN                           0       //蓝牙协议解析通道

static const char* TAG = "hygrother";
static float temperature;
static float humidity;

static TimerHandle_t temp_humi_update_timer;            //温湿度值刷新定时器
static TimerHandle_t enter_low_power_timer;             //开机后进入低功耗模式定时器
static uint32_t ble_connect_status = BLE_DISCONNECTED;
static bt_frame_t bt_prase = {0};                       //蓝牙协议解析结构体

/**
 * @brief 更新温湿度值的屏幕显示
 */
static void update_temp_humi_display(void)
{
    sht30_get_temp_and_humi(&temperature, &humidity);
    temperature = celsius_to_fahrenheit(temperature);
    LOG_I(TAG, "Get sht30 data, temperature : %f, humidity : %f", temperature, humidity);
    va_display_temperature(temperature, FAHRENHEIT_UNIT);
    va_display_humidity(humidity);
    bu9796a_display_ble_icon(true);
    bu9796a_display_wifi_icon(true);
}

/**
 * @brief 温湿度计参数更新定时器回调函数，读取最新温湿度计值
 * @param arg [无]
 */
static void temp_humi_update_timer_callback(void *arg)
{
    update_temp_humi_display();
    // printf_os_task_manager();

    LOG_I(TAG, "Battery voltage : %dmv", 4 * analog_adc_read_battery_mv());
    LOG_I(TAG, "Bettery charge status : %d", get_battery_charge_status());
    LOG_I(TAG, "Bettery charge fully status : %d", get_battery_charge_fully_status());
    LOG_I(TAG, "Touch value : %d", get_touch_key_value());
    LOG_I(TAG, "TLV8811 out : %d", analog_adc_read_tlv8811_out_mv());
    vesync_printf_system_time();

    LOG_I(TAG, "last result=%d, sample counter=%d, wake up counter=%d,adc max=%d, min=%d.",
          ulp_last_result & UINT16_MAX, ulp_all_sample_counter & UINT16_MAX, ulp_wakeup_counter & UINT16_MAX,
          ulp_adc_max & UINT16_MAX, ulp_adc_min & UINT16_MAX);

    static uint32_t second_count = 0;           //秒统计值
    second_count++;
    if(second_count == 20)                      //每20秒更新温湿度值到APP
    {
        second_count = 0;
        xTaskNotify(app_event_center_taskhd, UPDATE_TEMP_HUMI_TO_APP, eSetBits);			//通知事件处理中心任务
    }
}

/**
 * @brief 进入低功耗模式定时器回调函数
 * @param arg [无]
 */
static void enter_low_power_timer_callback(void *arg)
{
    LOG_E(TAG, "Enter low power mode !");
    enter_low_power_mode();
}

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
 * @brief 回复APP温湿度计历史记录数据
 */
static void reply_temp_humi_history_to_app(void)
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
 * @brief 蓝牙协议解析
 * @param data [原始蓝牙接收数据]
 * @param len  [数据长度]
 */
static void ble_command_handle(unsigned char *data, unsigned char len)
{
    for(unsigned char i = 0; i < len; i++)
    {
        if(bt_data_frame_decode(data[i], BT_PARSE_CHAN, &bt_prase) == 1)
        {
            //解析到蓝牙协议帧
            frame_ctrl_t res_ctl =      //应答包res状态
            {
                .data = 0,
            };
            struct                      //应答数据包
            {
                uint8_t  buf[20];
                uint16_t len;
            } resp_strl = {{0}, 0};

            uint8_t *cnt = NULL;
            uint8_t *opt = &bt_prase.frame_data[0];                     //指针指向data_buf数据域；
            uint8_t len = bt_prase.frame_data_len - sizeof(uint16_t);   //长度减去包含2个字节的命令包
            cnt = &bt_prase.frame_cnt;
            LOG_I(TAG, "bt_prase.frame_ctrl.bitN.ack_flag = %d", bt_prase.frame_ctrl.bitN.ack_flag);

            if(bt_prase.frame_ctrl.bitN.ack_flag == PACKET_COMMAND)     //下发的是命令包
            {
                LOG_I(TAG, "app set cmd [0x%04x]", bt_prase.frame_cmd);
                switch(bt_prase.frame_cmd)
                {
                    case QUERY_HISTORY:
                        reply_temp_humi_history_to_app();
                        break;
                    case CLEAR_HISTORY:
                        break;
                    case QUERY_PREWARNING:
                        break;
                    case SET_PREWARNING:
                        break;
                    default:
                        LOG_E(TAG, "app set other cmd!");
                        break;
                }

                if(bt_prase.frame_ctrl.bitN.request_flag == NEED_ACK)   //判断该命令是否需要应答
                {
                    res_ctl.bitN.ack_flag = 1;       //标示当前数据包为应答包;
                    if(res_ctl.bitN.error_flag == 1)
                    {
                        LOG_E(TAG, "ERROR CODE！");
                        // vesync_bt_notify(res_ctl, cnt, bt_prase.frame_cmd, resp_strl.buf, resp_strl.len); //返回1个字节的具体错误码
                    }
                    else
                    {
                        // vesync_bt_notify(res_ctl, cnt, bt_prase.frame_cmd, resp_strl.buf, resp_strl.len); //返回应答设置或查询包
                    }
                    LOG_I(TAG, "ack is need with command[0x%04x] ctrl[0x%02x].............", bt_prase.frame_cmd, res_ctl.data);
                }
            }
            else if(bt_prase.frame_ctrl.bitN.ack_flag == PACKET_RESP)   //app返回的应答
            {
                LOG_I(TAG, "app response");
                switch(bt_prase.frame_cmd)
                {
                    // case CMD_REPORT_VESION:
                    //     LOG_I(TAG, "CMD_REPORT_VESION");
                    //     break;
                    default:
                        LOG_I(TAG, "app resp other[0x%02x]", bt_prase.frame_cmd);
                        break;
                }
            }
        }
    }
}

/**
 * @brief 蓝牙数据接收回调函数
 * @param data  [接收的蓝牙数据]
 * @param len   [数据长度]
 */
void ble_recv_data_callback(const unsigned char *data, unsigned char len)
{
    LOG_I(TAG, "BLE recv %d byte data :", len);

    int i = 0;
    while(i < len)
    {
        printf("%x ", data[i++]);
    }
    printf("\n");

    ble_command_handle(data, len);
}

/**
 * @brief 蓝牙状态回调
 * @param bt_status [当前实时蓝牙状态]
 */
void ble_status_callback(BT_STATUS_T bt_status)
{
    LOG_I(TAG, "set ble status :%d", bt_status);
    switch(bt_status)
    {
        case BT_CREATE_SERVICE:
            break;
        case BT_ADVERTISE_START:
            break;
        case BT_ADVERTISE_STOP:
            break;
        case BT_CONNTED:
            vesync_bt_advertise_stop();
            xTimerStop(enter_low_power_timer, TIMER_BLOCK_TIME);
            xTaskNotify(app_event_center_taskhd, UPDATE_TEMP_HUMI_TO_APP, eSetBits);			//通知事件处理中心任务
            ble_connect_status = BLE_CONNECTED;
            break;
        case BT_DISCONNTED:
            // vesync_bt_advertise_start(120 * 1000);
            xTimerStart(enter_low_power_timer, TIMER_BLOCK_TIME);
            ble_connect_status = BLE_DISCONNECTED;
            break;
        default:
            break;
    }
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

    send_data.temp = (int16_t)(temperature * 10);
    send_data.humi = (int16_t)(humidity * 10);
    send_data.timestamp = 1544410793;

    seq++;
    vesync_bt_notify(send_ctl, &seq, send_cmd, (unsigned char *)&send_data, send_len);
    LOG_I(TAG, "Update temp humi to app !");
}

/**
 * @brief 温湿度计设备初始化
 */
void hygrothermograph_init(void)
{
    LOG_I(TAG, "Hygrothermograph init !");
    sht30_init();
    buzzer_init();
    analog_adc_init();
    va_display_init();
    update_temp_humi_display();

    temp_humi_update_timer = xTimerCreate("temp_humi", 1000 / portTICK_RATE_MS, pdTRUE, NULL, temp_humi_update_timer_callback);
    enter_low_power_timer = xTimerCreate("low_power", 10 * 1000 / portTICK_RATE_MS, pdFALSE, NULL, enter_low_power_timer_callback);
    xTimerStart(temp_humi_update_timer, TIMER_BLOCK_TIME);
    xTimerStart(enter_low_power_timer, TIMER_BLOCK_TIME);

    vesync_bt_client_init(PRODUCT_NAME, PRODUCT_VER, "0001", PRODUCT_TYPE, PRODUCT_NUM, NULL, true, ble_status_callback, ble_recv_data_callback);
    vesync_bt_advertise_start(0);
}
