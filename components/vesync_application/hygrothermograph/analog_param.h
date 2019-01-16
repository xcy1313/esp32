/**
 * @file analog_param.h
 * @brief ESP32 ADC驱动
 * @author WatWu
 * @date 2019-01-15
 */

#ifndef ANALOG_PARAM_H
#define ANALOG_PARAM_H

/**
 * @brief 模拟量ADC采集初始化
 */
void analog_adc_init(void);

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

#endif
