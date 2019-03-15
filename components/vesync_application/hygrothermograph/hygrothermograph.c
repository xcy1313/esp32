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
#include "interface.h"

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

    // LOG_I(TAG, "Battery voltage : %dmv", 4 * analog_adc_read_battery_mv());
    // LOG_I(TAG, "Bettery charge status : %d", get_battery_charge_status());
    // LOG_I(TAG, "Bettery charge fully status : %d", get_battery_charge_fully_status());
    // LOG_I(TAG, "Touch value : %d", get_touch_key_value());
    // LOG_I(TAG, "TLV8811 out : %d", analog_adc_read_tlv8811_out_mv());
    // // vesync_printf_system_time();

    // LOG_I(TAG, "last result=%d, sample counter=%d, wake up counter=%d,adc max=%d, min=%d.",
    //       ulp_last_result & UINT16_MAX, ulp_all_sample_counter & UINT16_MAX, ulp_wakeup_counter & UINT16_MAX,
    //       ulp_adc_max & UINT16_MAX, ulp_adc_min & UINT16_MAX);

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
            if(bt_prase.frame_ctrl.bitN.ack_flag == PACKET_COMMAND)     //下发的是命令包
            {
                LOG_I(TAG, "app set cmd [0x%04x]", bt_prase.frame_cmd);
                switch(bt_prase.frame_cmd)
                {
                    case QUERY_HISTORY:
                        reply_temp_humi_history_to_app();
                        break;
                    case CLEAR_HISTORY:
                        device_clear_temp_humi_history();
                        break;
                    case QUERY_PREWARNING:
                        // reply_early_warning_setting_to_app(hygrother_warning_t* warn_setting);
                        break;
                    case SET_PREWARNING:
                        app_set_early_warning_setting((hygrother_warning_t*)bt_prase.frame_data);
                        break;
                    case QUERY_TEMP_HUMI:
                        reply_temp_humi_to_app();
                        break;
                    case QUERY_CALIBRATION:
                        break;
                    case SET_CALIBRATION:
                        break;
                    case SYNC_TIME:
                        break;
                    case SWITCH_UNIT:
                        break;
                    default:
                        LOG_E(TAG, "app set other cmd!");
                        break;
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
 * @brief 获取温度值
 * @return float [温度值，单位与设置保持一致]
 */
float get_device_temperature(void)
{
    return temperature;
}

/**
 * @brief 获取湿度值
 * @return float [湿度值]
 */
float get_device_humidity(void)
{
    return humidity;
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

    ble_command_handle((unsigned char*)data, len);
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
    enter_low_power_timer = xTimerCreate("low_power", 60 * 1000 / portTICK_RATE_MS, pdFALSE, NULL, enter_low_power_timer_callback);
    xTimerStart(temp_humi_update_timer, TIMER_BLOCK_TIME);
    xTimerStart(enter_low_power_timer, TIMER_BLOCK_TIME);

    vesync_bt_client_init(PRODUCT_NAME, PRODUCT_VER, "0001", PRODUCT_TYPE, PRODUCT_NUM, NULL, true, ble_status_callback, ble_recv_data_callback);
    vesync_bt_advertise_start(0);
}
