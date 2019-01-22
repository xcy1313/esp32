/**
 * @file sht30.h
 * @brief sht30驱动接口
 * @author WatWu
 * @date 2019-01-03
 */
#ifndef SHT30_H_
#define SHT30_H_

#include <stdint.h>

#define CELSIUS_UNIT            0           //摄氏度单位
#define FAHRENHEIT_UNIT         1           //华氏度单位

/**
 * @brief 初始化传感器
 * @return uint32_t [初始化结果，0为成功]
 */
uint32_t sht30_init(void);

/**
 * @brief 读取温湿度值
 * @param temp 		[温度值]
 * @param humi 		[湿度值]
 * @return uint32_t [读取结果]
 */
uint32_t sht30_get_temp_and_humi(float *temp, float *humi);

#endif
