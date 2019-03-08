/**
 * @file analog_param.h
 * @brief ESP32 ADC驱动
 * @author WatWu
 * @date 2019-01-15
 */

#ifndef ANALOG_PARAM_H
#define ANALOG_PARAM_H

#define INFRARED__THRESHOLD                 12      //人体红外感应ADC的感应变化阈值

/**
 * @brief 模拟量ADC采集初始化
 */
void analog_adc_init(void);

/**
 * @brief 读取tlv8811芯片的原始输出值
 * @return uint32_t [模拟量输出值，原始ADC采样值]
 */
uint32_t analog_adc_read_tlv8811_out_raw(void);

/**
 * @brief 读取tlv8811芯片的输出值
 * @return uint32_t [模拟量输出值，单位毫伏]
 */
uint32_t analog_adc_read_tlv8811_out_mv(void);

/**
 * @brief 读取tlv8811芯片的环境温度检测输出值
 * @return uint32_t [模拟量输出值，单位毫伏]
 */
uint32_t analog_adc_read_tlv8811_envi_temp_mv(void);

/**
 * @brief 读取电池电压值
 * @return uint32_t [模拟量输出值，单位毫伏]
 */
uint32_t analog_adc_read_battery_mv(void);

/**
 * @brief 判断电压值的变化是否超出变化阈值
 * @param voltage   [当次采样电压值]
 * @param threshold [变化阈值]
 * @return uint8_t  [是否超阈值，1为超出阈值，0为未超]
 */
uint8_t judge_voltage_change(uint32_t voltage, uint8_t threshold);

#endif
