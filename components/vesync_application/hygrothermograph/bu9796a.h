/**
 * @file bu9796a.h
 * @brief bu9796a显示驱动
 * @author WatWu
 * @date 2019-01-08
 */

#ifndef BU9796A_H
#define BU9796A_H

/**
 * @brief 开启背光
 */
void bu9796a_backlight_on(void);

/**
 * @brief 关闭背光
 */
void bu9796a_backlight_off(void);

/**
 * @brief 初始化BU9796A通信接口
 * @return int32_t [初始化结果，0为成功]
 */
int32_t bu9796a_init(void);

/**
 * @brief BU9796A初始化序列
 * @return int32_t [初始化结果，0为成功]
 */
int32_t bu9796a_initialize_sequence(void);

/**
 * @brief BU9796A开启显示序列
 * @return int32_t [开启结果，0为成功]
 */
int32_t bu9796a_display_on_sequence(void);

/**
 * @brief BU9796A写显示数据序列
 * @param data      [显示的RAM数据内容]
 * @param len       [数据长度]
 * @return int32_t  [写入结果，0为成功]
 */
int32_t bu9796a_ram_write_sequence(uint8_t *data, uint8_t len);

/**
 * @brief BU9796A关闭显示序列
 * @return int32_t [开启结果，0为成功]
 */
int32_t bu9796a_display_off_sequence(void);

/**
 * @brief BU9796A开启全部像素显示
 * @return int32_t [开启结果，0为成功]
 */
int32_t bu9796a_display_all_pixels_on(void);

/**
 * @brief 屏幕显示数字，写到显示内存
 * @param num_pos   [数字位置，0-4]
 * @param number    [待显示的数字，0-9]
 */
void bu9796a_display_number_to_ram(uint8_t num_pos, uint8_t number);

/**
 * @brief 清楚屏幕显示的数字
 * @param num_pos [数字位置，0-4]
 */
void bu9796a_display_number_clear(uint8_t num_pos);

/**
 * @brief 显示数字负号
 */
void bu9796a_display_minus(void);

/**
 * @brief 屏幕显示蓝牙图标，写到显示内存
 * @param dis_flag [显示标识，true或false]
 */
void bu9796a_display_ble_icon(uint8_t dis_flag);

/**
 * @brief 屏幕显示最高位数字1，写到显示内存
 * @param dis_flag [显示标识，true或false]
 */
void bu9796a_display_num_one(uint8_t dis_flag);

/**
 * @brief 屏幕显示小数点，写到显示内存
 * @param dis_flag [显示标识，true或false]
 */
void bu9796a_display_num_point(uint8_t dis_flag);

/**
 * @brief 屏幕显示湿度符号，含湿度图标和百分号
 * @param dis_flag [显示标识，true或false]
 */
void bu9796a_display_humi_icon(uint8_t dis_flag);

/**
 * @brief 屏幕显示温度单位
 * @param units [待显示的温度单位，0为摄氏度，1为华氏度]
 */
void bu9796a_display_temp_units(uint8_t units);

/**
 * @brief 刷新显示
 */
void bu9796a_update_display(void);

#endif
