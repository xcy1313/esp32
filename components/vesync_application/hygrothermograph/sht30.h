/**
 * @file sht30.h
 * @brief sht30驱动接口
 * @author WatWu
 * @date 2019-01-03
 */
#ifndef SHT30_H_
#define SHT30_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SHT30_POLYNOMIAL 0x131 // P(x) = x^8 + x^5 + x^4 + 1 = 100110001

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

#ifdef __cplusplus
}
#endif

#endif
