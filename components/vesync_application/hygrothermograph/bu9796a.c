/**
 * @file bu9796a.c
 * @brief bu9796a显示驱动
 * @author WatWu
 * @date 2019-01-08
 */

#include <string.h>

#include "esp_i2c.h"
#include "vesync_log.h"

#define BU9796A_BACKLIGHT_GPIO          2           //BU9796A背光引脚定义
#define BACKLIGHT_ON                    1
#define BACKLIGHT_OFF                   0

#define BU9796A_IIC_PORT                1           //BU9796A所接的IIC总线端口

// BU9796A的命令字节的最高位为标记接下来的字节是命令还是数据，最高位为1表示紧接的字节为命令，为0则是数据
#define NEXT_IS_COMMAND                 0x80
#define NEXT_IS_DATA                    0x7F

#define SLAVE_ADDRESS                   0x3E        //从机地址
#define ICSET_SOFT_RESET                0x6A        //软件复位指令
#define ICSET_INTERNAL_OSC              0x68        //使能内部时钟
#define MODESET_DIS_OFF                 0x40        //关闭显示指令
#define MODESET_DIS_ON                  0x48        //开启显示指令
#define ADSET_ZERO                      0x00        //设置显示地址
#define DISCTL_FR_FRAME_NORMAL          0x2F        //设置显示控制，Power save mode FR=3，Frame inversion，Power save mode SR=Normal mode
#define BLKCTL_DO_NOT_BLINK             0x70        //不闪烁
#define APCTL_NORMAL                    0x7C        //全部像素正常显示
#define APCTL_ALL_PIXELS_ON             0x7E        //全部像素点亮
#define APCTL_ALL_PIXELS_OFF            0x7D        //全部像素关闭

static const char *TAG = "BU9796A";

/**
 * @brief 开启背光
 */
static void bu9796a_backlight_on(void)
{
    gpio_set_level(BU9796A_BACKLIGHT_GPIO, BACKLIGHT_ON);   //开启背光
}

/**
 * @brief 关闭背光
 */
static void bu9796a_backlight_off(void)
{
    gpio_set_level(BU9796A_BACKLIGHT_GPIO, BACKLIGHT_OFF);   //关闭背光
}

/**
 * @brief 初始化BU9796A通信接口
 * @return int32_t [初始化结果，0为成功]
 */
int32_t bu9796a_init(void)
{
    int32_t error;
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = 1ULL << BU9796A_BACKLIGHT_GPIO;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
    bu9796a_backlight_off();

    error = i2c_master_init(BU9796A_IIC_PORT, 23, 22, 100000);

    return error;
}

/**
 * @brief BU9796A初始化序列
 * @return int32_t [初始化结果，0为成功]
 */
int32_t bu9796a_initialize_sequence(void)
{
    int32_t error;
    uint8_t i2cData[3];
    i2cData[0] = ICSET_SOFT_RESET | NEXT_IS_COMMAND;//软件复位
    i2cData[1] = MODESET_DIS_OFF | NEXT_IS_COMMAND; //关闭显示
    i2cData[2] = ADSET_ZERO;                        //设置显示地址为0
    i2c_master_write_slave_stop_condition(BU9796A_IIC_PORT);    //产生stop信号
    error = i2c_master_write_slave(BU9796A_IIC_PORT, SLAVE_ADDRESS, i2cData, 3); //写入以上初始化序列指令

    return error;
}

/**
 * @brief BU9796A开启显示序列
 * @return int32_t [开启结果，0为成功]
 */
int32_t bu9796a_display_on_sequence(void)
{
    int32_t error;
    uint8_t i2cData[5];
    i2cData[0] = ICSET_INTERNAL_OSC | NEXT_IS_COMMAND;      //使能内部时钟
    i2cData[1] = DISCTL_FR_FRAME_NORMAL | NEXT_IS_COMMAND;  //设置显示参数
    i2cData[2] = BLKCTL_DO_NOT_BLINK | NEXT_IS_COMMAND;     //设置不闪烁
    i2cData[3] = APCTL_NORMAL | NEXT_IS_COMMAND;            //全部像素正常显示
    i2cData[4] = MODESET_DIS_ON | NEXT_IS_COMMAND;          //开启显示
    error = i2c_master_write_slave(BU9796A_IIC_PORT, SLAVE_ADDRESS, i2cData, 5); //写入以上初始化序列指令
    bu9796a_backlight_on();                                 //开启背光

    return error;
}

/**
 * @brief BU9796A写显示数据序列
 * @param data      [显示的RAM数据内容]
 * @param len       [数据长度]
 * @return int32_t  [写入结果，0为成功]
 */
int32_t bu9796a_ram_write_sequence(uint8_t *data, uint8_t len)
{
    int32_t error;
    uint8_t i2cData[20];
    i2cData[0] = ICSET_INTERNAL_OSC | NEXT_IS_COMMAND;      //使能内部时钟
    i2cData[1] = DISCTL_FR_FRAME_NORMAL | NEXT_IS_COMMAND;  //设置显示参数
    i2cData[2] = BLKCTL_DO_NOT_BLINK | NEXT_IS_COMMAND;     //设置不闪烁
    i2cData[3] = APCTL_NORMAL | NEXT_IS_COMMAND;            //全部像素正常显示
    i2cData[4] = MODESET_DIS_ON | NEXT_IS_COMMAND;          //开启显示
    i2cData[5] = ADSET_ZERO & NEXT_IS_DATA;                 //设置初始地址
    if(len < 12)
    {
        memcpy(&(i2cData[6]), (const void*)data, len);
        error = i2c_master_write_slave(BU9796A_IIC_PORT, SLAVE_ADDRESS, i2cData, 6 + len); //写入以上指令及数据
        bu9796a_backlight_on();                             //开启背光
    }
    else
    {
        error = -1;
        LOG_E(TAG, "RAM data length is too long !");
    }

    return error;
}

/**
 * @brief BU9796A关闭显示序列
 * @return int32_t [开启结果，0为成功]
 */
int32_t bu9796a_display_off_sequence(void)
{
    int32_t error;
    uint8_t i2cData[2];
    i2cData[0] = ICSET_INTERNAL_OSC | NEXT_IS_COMMAND;      //使能内部时钟
    i2cData[1] = MODESET_DIS_OFF | NEXT_IS_COMMAND;         //关闭显示
    error = i2c_master_write_slave(BU9796A_IIC_PORT, SLAVE_ADDRESS, i2cData, 2); //写入以上初始化序列指令
    bu9796a_backlight_off();                                 //关闭背光

    return error;
}

/**
 * @brief BU9796A开启全部像素显示
 * @return int32_t [开启结果，0为成功]
 */
int32_t bu9796a_display_all_pixels_on(void)
{
    int32_t error;
    uint8_t i2cData[5];
    i2cData[0] = ICSET_INTERNAL_OSC | NEXT_IS_COMMAND;      //使能内部时钟
    i2cData[1] = 0x22 | NEXT_IS_COMMAND;  //设置显示参数
    i2cData[2] = BLKCTL_DO_NOT_BLINK | NEXT_IS_COMMAND;     //设置不闪烁
    i2cData[3] = APCTL_ALL_PIXELS_ON | NEXT_IS_COMMAND;     //全部像素正常显示
    i2cData[4] = MODESET_DIS_ON | NEXT_IS_COMMAND | 0x04;          //开启显示
    error = i2c_master_write_slave(BU9796A_IIC_PORT, SLAVE_ADDRESS, i2cData, 5); //写入以上初始化序列指令
    bu9796a_backlight_on();                                 //开启背光

    return error;
}
