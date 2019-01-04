/**
 * @file sht30.c
 * @brief sht30驱动接口
 * @author WatWu
 * @date 2019-01-03
 */

#include "esp_i2c.h"
#include "sht30.h"

/**
 * @brief 计算温度值
 * @param raw_value [传感器原始值]
 * @return float 	[温度值，单位：摄氏度]
 */
float sht30_calc_temperature(const uint16_t raw_value)
{
    return 175 * (float)raw_value / 65536 - 45;
}

/**
 * @brief 计算湿度值
 * @param raw_value [传感器原始值]
 * @return float 	[湿度值，单位：百分比]
 */
float sht30_calc_humidity(const uint16_t raw_value)
{
    return 100 * (float)raw_value / 65536;
}

/**
 * @brief 检验CRC
 * @param data 			[需要计算CRC的数据]
 * @param data_byte 	[数据长度]
 * @param checksum 		[crc校验值]
 * @return uint32_t 	[校验结果，0为成功]
 */
uint32_t sht30_check_crc(const uint8_t data[], const uint8_t data_byte, uint8_t const checksum)
{
    uint8_t bit;
    uint8_t crc = 0xFF;
    uint8_t byteCtr;

    for(byteCtr = 0; byteCtr < data_byte; byteCtr++)
    {
        crc ^= (data[byteCtr]);
        for(bit = 8; bit > 0; --bit)
        {
            if(crc & 0x80) crc = (crc << 1) ^ SHT30_POLYNOMIAL;
            else crc = (crc << 1);
        }
    }

    if(crc != checksum) return -1;
    else return 0;
}

/**
 * @brief 软件复位传感器
 * @return uint32_t [复位结果，0为成功]
 */
uint32_t sht30_soft_reset(void)
{
    uint32_t error;
    uint8_t i2cData[2];
    i2cData[0] = (eSht30SoftReset >> 8);
    i2cData[1] = (0xFF & eSht30SoftReset);
    error = i2c_master_write_slave(0, eSht30I2CAddress, i2cData, 2);
    return error;
}

/**
 * @brief 初始化传感器
 * @return uint32_t [初始化结果，0为成功]
 */
uint32_t sht30_init(void)
{
    uint32_t error;
    i2c_master_init(0, 21, 19, 100000);
    error = sht30_soft_reset();
    return error;
}

/**
 * @brief 读取温湿度值
 * @param temp 		[温度值]
 * @param humi 		[湿度值]
 * @return uint32_t [读取结果]
 */
uint32_t sht30_get_temp_and_humi(float *temp, float *humi)
{
    uint32_t error;
    uint16_t raw_valueTemp;
    uint16_t raw_valueHumi;
    uint8_t i2cData[6];
    i2cData[0] = (eSht30MeasureTHSingleShotHighRepClockstr >> 8);
    i2cData[1] = (0xFF & eSht30MeasureTHSingleShotHighRepClockstr);
    error = i2c_master_write_slave(0, eSht30I2CAddress, i2cData, 2);
    if(0 == error)
    {
        error = i2c_master_read_slave(0, eSht30I2CAddress, i2cData, 6);
    }

    if(0 == error)
    {
        error = sht30_check_crc(&(i2cData[0]), 2, i2cData[2]);
    }

    if(0 == error)
    {
        error = sht30_check_crc(&(i2cData[3]), 2, i2cData[5]);
    }

    if(0 == error)
    {
        raw_valueTemp = ((uint16_t)(i2cData[0]) << 8) + (uint16_t)(i2cData[1]);
        raw_valueHumi = ((uint16_t)(i2cData[3]) << 8) + (uint16_t)(i2cData[4]);
        *temp = sht30_calc_temperature(raw_valueTemp);
        *humi = sht30_calc_humidity(raw_valueHumi);
    }

    return error;
}
