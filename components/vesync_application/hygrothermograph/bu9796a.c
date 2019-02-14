/**
 * @file bu9796a.c
 * @brief bu9796a显示驱动
 * @author WatWu
 * @date 2019-01-08
 */

#include "esp_i2c.h"
#include "vesync_api.h"

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
#define MODESET_DIS_ON                  0x48        //开启显示指令，48:1/3dias，4C：1/2dias
#define ADSET_ZERO                      0x00        //设置显示地址
#define DISCTL_FR_FRAME_NORMAL          0x3C        //设置显示控制
#define BLKCTL_DO_NOT_BLINK             0x70        //不闪烁
#define APCTL_NORMAL                    0x7C        //全部像素正常显示
#define APCTL_ALL_PIXELS_ON             0x7E        //全部像素点亮
#define APCTL_ALL_PIXELS_OFF            0x7D        //全部像素关闭

#define DISPLAY_RAM_SIZE                6

static const char *TAG = "BU9796A";

static uint8_t display_ram[DISPLAY_RAM_SIZE] = {0}; //屏幕显示缓存
static unsigned char code_distab[10] = {0x5f, 0x06, 0x3d, 0x2f, 0x66, 0x6b, 0x7b, 0x0e, 0x7f, 0x6f};

/**
 * @brief 开启背光
 */
void bu9796a_backlight_on(void)
{
    gpio_set_level(BU9796A_BACKLIGHT_GPIO, BACKLIGHT_ON);   //开启背光
}

/**
 * @brief 关闭背光
 */
void bu9796a_backlight_off(void)
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
    if(len <= DISPLAY_RAM_SIZE)
    {
        memcpy(&(i2cData[6]), (const void*)data, len);
        error = i2c_master_write_slave(BU9796A_IIC_PORT, SLAVE_ADDRESS, i2cData, 6 + len); //写入以上指令及数据
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

    return error;
}

/**
 * @brief 屏幕显示数字，写到显示内存
 * @param num_pos   [数字位置，0-4]
 * @param number    [待显示的数字,0-9]
 */
void bu9796a_display_number_to_ram(uint8_t num_pos, uint8_t number)
{
    uint8_t hightest_bit = display_ram[num_pos] & 0x80;     //保留最高位
    if(number < 10)
        display_ram[num_pos] = code_distab[number] | hightest_bit;
}

/**
 * @brief 清楚屏幕显示的数字
 * @param num_pos [数字位置，0-4]
 */
void bu9796a_display_number_clear(uint8_t num_pos)
{
    uint8_t hightest_bit = display_ram[num_pos] & 0x80;     //保留最高位
    display_ram[num_pos] = 0 | hightest_bit;
}

/**
 * @brief 显示数字负号
 */
void bu9796a_display_minus(void)
{
    uint8_t hightest_bit = display_ram[0] & 0x80;           //保留最高位
    display_ram[0] = 0x20 | hightest_bit;
}

/**
 * @brief 屏幕显示蓝牙图标，写到显示内存
 * @param dis_flag [显示标识，true或false]
 */
void bu9796a_display_ble_icon(uint8_t dis_flag)
{
    if(dis_flag == true)
        display_ram[1] |= 0x80;
    else
        display_ram[1] &= 0x7F;
}

/**
 * @brief 屏幕显示WiFi图标，写到显示内存
 * @param dis_flag [显示标识，true或false]
 */
void bu9796a_display_wifi_icon(uint8_t dis_flag)
{
    if(dis_flag == true)
        display_ram[5] |= 0x80;
    else
        display_ram[5] &= 0x7F;
}

/**
 * @brief 屏幕显示电池电量图标
 * @param power_dump [剩余电量，0-3，代表剩余电量级别，0为0%~25%，以此类推]
 */
void bu9796a_display_bat_power_icon(uint8_t power_dump)
{
    uint8_t hightest_4bit = display_ram[5] & 0xF0;      //保留高4位
    switch(power_dump)
    {
        case 0:
            display_ram[5] = 0x08 | hightest_4bit;
            break;
        case 1:
            display_ram[5] = 0x0C | hightest_4bit;
            break;
        case 2:
            display_ram[5] = 0x0E | hightest_4bit;
            break;
        case 3:
            display_ram[5] = 0x0F | hightest_4bit;
            break;
    }
}

/**
 * @brief 屏幕显示最高位数字1，写到显示内存
 * @param dis_flag [显示标识，true或false]
 */
void bu9796a_display_num_one(uint8_t dis_flag)
{
    if(dis_flag == true)
        display_ram[0] |= 0x80;
    else
        display_ram[0] &= 0x7F;
}

/**
 * @brief 屏幕显示小数点，写到显示内存
 * @param dis_flag [显示标识，true或false]
 */
void bu9796a_display_num_point(uint8_t dis_flag)
{
    if(dis_flag == true)
    {
        display_ram[2] |= 0x80;
        display_ram[5] |= 0x40;
    }
    else
    {
        display_ram[2] &= 0x7F;
        display_ram[5] &= 0xBF;
    }
}

/**
 * @brief 屏幕显示湿度符号，含湿度图标和百分号
 * @param dis_flag [显示标识，true或false]
 */
void bu9796a_display_humi_icon(uint8_t dis_flag)
{
    if(dis_flag == true)
    {
        display_ram[3] |= 0x80;
        display_ram[4] |= 0x80;
    }
    else
    {
        display_ram[3] &= 0x7F;
        display_ram[4] &= 0x7F;
    }
}

/**
 * @brief 屏幕显示温度单位
 * @param units [待显示的温度单位，0为摄氏度，1为华氏度]
 */
void bu9796a_display_temp_units(uint8_t units)
{
    display_ram[5] |= 0x40;     //温度单位的圆圈
    if(units == 0)
    {
        display_ram[5] |= 0x10;
        display_ram[5] &= 0xDF;
    }
    else
    {
        display_ram[5] |= 0x20;
        display_ram[5] &= 0xEF;
    }
}

/**
 * @brief 清屏
 */
void bu9796a_clear_display(void)
{
    memset(display_ram, 0, DISPLAY_RAM_SIZE);
}

/**
 * @brief 刷新显示
 */
void bu9796a_update_display(void)
{
    bu9796a_ram_write_sequence(display_ram, DISPLAY_RAM_SIZE);
}
