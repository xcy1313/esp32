/**
 * @file analog_param.c
 * @brief 温湿度计模拟量参数采集
 * @author WatWu
 * @date 2019-01-15
 */

#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_system.h"
#include "esp_adc_cal.h"

#include "vesync_main.h"

#define DEFAULT_VREF                1100            //默认的ADC基准值
#define NUM_OF_SAMPLES              100             //启动一次采集的ADC样本数

#define TLV8811_OUT_GPIO_CHAN       ADC1_CHANNEL_4  //TLV8811芯片输出通道，GPIO32
#define TLV8811_ENVI_TEMP_CHAN      ADC1_CHANNEL_5  //TLV8811环境温度通道，GPIO33

static const char* TAG = "esp_adc";
static esp_adc_cal_characteristics_t *adc_chars;    //ADC属性参数

/**
 * @brief 检查熔丝保存的ADC校准值
 */
static void check_efuse_vref()
{
    //Check TP is burned into eFuse
    if(esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK)
    {
        LOG_I(TAG, "eFuse Two Point: Supported");
    }
    else
    {
        LOG_I(TAG, "eFuse Two Point: NOT supported");
    }

    //Check Vref is burned into eFuse
    if(esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK)
    {
        LOG_I(TAG, "eFuse Vref: Supported");
    }
    else
    {
        LOG_I(TAG, "eFuse Vref: NOT supported");
    }
}

/**
 * @brief 打印ADC校准类型
 * @param val_type [校准类型]
 */
static void print_char_val_type(esp_adc_cal_value_t val_type)
{
    if(val_type == ESP_ADC_CAL_VAL_EFUSE_TP)
    {
        LOG_I(TAG, "Characterized using Two Point Value");
    }
    else if(val_type == ESP_ADC_CAL_VAL_EFUSE_VREF)
    {
        LOG_I(TAG, "Characterized using eFuse Vref");
    }
    else
    {
        LOG_I(TAG, "Characterized using Default Vref");
    }
}

/**
 * @brief 模拟量ADC采集初始化
 */
void analog_adc_init(void)
{
    check_efuse_vref();
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(TLV8811_OUT_GPIO_CHAN, ADC_ATTEN_11db);
    adc1_config_channel_atten(TLV8811_ENVI_TEMP_CHAN, ADC_ATTEN_11db);

    //Characterize ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_11db, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
    print_char_val_type(val_type);
    LOG_I(TAG, "ADC verf : %d", adc_chars->vref);
}

/**
 * @brief 读取tlv8811芯片的输出值
 * @return uint32_t [模拟量输出值，单位毫伏]
 */
uint32_t analog_adc_read_tlv8811_out_mv(void)
{
    uint32_t read_raw;
    uint32_t voltage;
    uint32_t adc_reading;
    uint32_t samples_count;

    adc_reading = 0;
    samples_count = 0;
    for(int i = 0; i < NUM_OF_SAMPLES; i++)
    {
        read_raw = adc1_get_raw(TLV8811_OUT_GPIO_CHAN);
        if(read_raw != -1)
        {
            adc_reading += read_raw;
            samples_count++;
        }
    }
    adc_reading /= samples_count;
    voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
    LOG_I(TAG, "TLV8811 out, raw: %d\tvoltage: %dmv", adc_reading, voltage);

    return voltage;
}

/**
 * @brief 读取tlv8811芯片的环境温度检测输出值
 * @return uint32_t [模拟量输出值，单位毫伏]
 */
uint32_t analog_adc_read_tlv8811_envi_temp_mv(void)
{
    uint32_t read_raw;
    uint32_t voltage;
    uint32_t adc_reading;
    uint32_t samples_count;

    adc_reading = 0;
    samples_count = 0;
    for(int i = 0; i < NUM_OF_SAMPLES; i++)
    {
        read_raw = adc1_get_raw(TLV8811_ENVI_TEMP_CHAN);
        if(read_raw != -1)
        {
            adc_reading += read_raw;
            samples_count++;
        }
    }
    adc_reading /= samples_count;
    voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
    LOG_I(TAG, "TLV8811 envi raw: %d\tvoltage: %dmv", adc_reading, voltage);

    return voltage;
}
