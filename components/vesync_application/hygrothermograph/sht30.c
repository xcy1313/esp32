/**
 * @file sht30.c
 * @brief sht30驱动接口
 * @author WatWu
 * @date 2019-01-03
 */

#include "esp_i2c.h"
#include "sht30.h"

#define SHT30_IIC_PORT          0       //SHT30传感器所接的IIC总线端口

#define SHT30_POLYNOMIAL        0x131 // P(x) = x^8 + x^5 + x^4 + 1 = 100110001
// ID register mask (bits 0...5 are SHT30-specific product code
#define SHT30_PRODUCT_ID_MASK	0x003F // SHT30-product ID = xxxx'xxxx'xx00'0111 (where x=unspecific information)

// Sensor Commands
typedef enum
{
    eSht30SoftReset = 0x30A2, // soft reset
    eSht30MeasureTHSingleShotHighRepClockstr = 0x2C06, // meas. read T first, clock stretching disabled, high repeatibility
    eSht30MeasureTHSingleShotMedRepClockstr = 0x2C0D, // meas. read T first, clock stretching disabled, high repeatibility
    eSht30MeasureTHSingleShotLowRepClockstr = 0x2C10, // meas. read T first, clock stretching disabled, high repeatibility
    eSht30MeasureTHSingleShotHighRep = 0x2400, // meas. read T first, clock stretching disabled, high repeatibility
    eSht30MeasureTHSingleShotMedRep = 0x240B, // meas. read T first, clock stretching disabled, high repeatibility
    eSht30MeasureTHSingleShotLowRep = 0x2416, // meas. read T first, clock stretching disabled, high repeatibility
} eSht30Commands_t;

// I2C address
typedef enum
{
    eSht30I2CAddress = 0x44, // sensor I2C address (7-bit)
    eSht30I2cAddressAndWriteBit = 0x88, // sensor I2C address + write bit
    eSht30I2cAddressAndReadBit = 0x89 // sensor I2C address + read bit
} eSht30I2cAddress_t;

/**
 * @brief 计算温度值
 * @param raw_value [传感器原始值]
 * @return float 	[温度值，单位：摄氏度]
 */
static float sht30_calc_temperature(const uint16_t raw_value)
{
    return 175 * (float)raw_value / 65536 - 45;
}

/**
 * @brief 计算湿度值
 * @param raw_value [传感器原始值]
 * @return float 	[湿度值，单位：百分比]
 */
static float sht30_calc_humidity(const uint16_t raw_value)
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
static uint32_t sht30_check_crc(const uint8_t data[], const uint8_t data_byte, uint8_t const checksum)
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
static uint32_t sht30_soft_reset(void)
{
    uint32_t error;
    uint8_t i2cData[2];
    i2cData[0] = (eSht30SoftReset >> 8);
    i2cData[1] = (0xFF & eSht30SoftReset);
    error = i2c_master_write_slave(SHT30_IIC_PORT, eSht30I2CAddress, i2cData, 2);
    return error;
}

/**
 * @brief 初始化传感器
 * @return uint32_t [初始化结果，0为成功]
 */
uint32_t sht30_init(void)
{
    uint32_t error;
    i2c_master_init(SHT30_IIC_PORT, 21, 19, 100000);
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
    error = i2c_master_write_slave(SHT30_IIC_PORT, eSht30I2CAddress, i2cData, 2);
    if(0 == error)
    {
        error = i2c_master_read_slave(SHT30_IIC_PORT, eSht30I2CAddress, i2cData, 6);
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

/**
 * @brief 摄氏度转换成华氏度
 * @param celsius   [摄氏度值]
 * @return float    [华氏度值]
 */
float celsius_to_fahrenheit(float celsius)
{
    return celsius * 1.8 + 32;
} 
