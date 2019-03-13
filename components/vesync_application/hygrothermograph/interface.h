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
#define ACK_WITH_ERROR                          0X50        //查询或设置命令格式的应答格式，由错误标志位

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
 * @brief 回复APP温湿度计历史记录数据
 */
void reply_temp_humi_history_to_app(void);

/**
 * @brief 更新温湿度值到APP，通过ble蓝牙
 */
void update_temp_humi_to_app(void);

/**
 * @brief 上传温湿度历史记录到服务器
 * @param history_data [温湿度历史数据]
 */
void upload_temp_humi_history_to_server(temp_humi_history_t* history_data);

#endif
