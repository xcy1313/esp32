/**
 * @file interface.h
 * @brief 温湿度计接口
 * @author WatWu
 * @date 2019-03-13
 */

#ifndef INTERFACE_H
#define INTERFACE_H

//蓝牙通信协议控制码定义
#define ACTIVE_REPORT_CTL                       0X00        //主动上报数据格式
#define COMMAND_CTL                             0X20        //查询或设置命令格式
#define ACK_WITH_NO_ERROR                       0X10        //查询或设置命令格式的应答格式，无错误标志位
#define ACK_WITH_ERROR                          0X50        //查询或设置命令格式的应答格式，有错误标志位

//蓝牙通信命令定义
#define REPORT_TEMP_HUMI                        0x0301      //设备上报温湿度值
#define QUERY_HISTORY                           0x0302      //查询温湿度历史记录
#define CLEAR_HISTORY                           0x0303      //清空温湿度历史记录
#define QUERY_PREWARNING                        0x0304      //查询温湿度预警值
#define SET_PREWARNING                          0x0305      //设置温湿度预警值
#define QUERY_TEMP_HUMI                         0X0306      //查询设备实时温湿度值
#define QUERY_CALIBRATION                       0X0307      //查询设备温湿度校准值
#define SET_CALIBRATION                         0X0308      //设置设备温湿度校准值
#define SYNC_TIME                               0X0309      //同步时间
#define SWITCH_UNIT                             0X030A      //切换单位

/**
 * @brief 更新温湿度值到APP，通过ble蓝牙
 */
void update_temp_humi_to_app(void);

/**
 * @brief 回复APP温湿度计历史记录数据
 */
void reply_temp_humi_history_to_app(void);

/**
 * @brief 温湿度计清空历史数据
 * @return uint32_t [处理结果]
 */
uint32_t device_clear_temp_humi_history(void);

/**
 * @brief 回复APP要查询的温湿度计预警值设置参数
 * @param warn_setting  [温湿度预警值设置]
 * @return uint32_t     [回复结果]
 */
uint32_t reply_early_warning_setting_to_app(hygrother_warning_t* warn_setting);

/**
 * @brief APP设置温湿度预警值
 * @param warn_setting  [温湿度预警值结构体]
 * @return uint32_t     [设置结果]
 */
uint32_t app_set_early_warning_setting(hygrother_warning_t* warn_setting);

/**
 * @brief 回复APP要查询的温湿度值
 */
void reply_temp_humi_to_app(void);

/**
 * @brief 上传温湿度历史记录到服务器
 * @param history_data [温湿度历史数据]
 */
void upload_temp_humi_history_to_server(temp_humi_history_t* history_data);

/**
 * @brief 发送温湿度预警消息到服务器
 * @param warn_val  [发生预警的上限或下限值]
 * @param curr_val  [当前值]
 * @param warn_type [预警类型，1 - 温度超上限；2 - 温度超下限； 3 - 湿度超上限；4 - 湿度超下限]
 * @param timestamp [发生预警时的时间]
 */
void send_early_warning_to_server(float warn_val, float curr_val, int warn_type, uint32_t timestamp);

#endif
