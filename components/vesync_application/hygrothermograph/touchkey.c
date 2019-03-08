/**
 * @file touchkey.c
 * @brief 触摸、按键等开关量输入
 * @author WatWu
 * @date 2019-01-11
 */

#include "driver/gpio.h"
#include "driver/touch_pad.h"
#include "touchkey.h"

#include "vesync_log.h"

#define TOUCH_PAD_CHAN              TOUCH_PAD_NUM0      //触摸按键通道，GPIO4
#define TOUCH_PAD_THRESH            25                  //触摸检测阈值

static const char *TAG = "touchkey";

static uint16_t touch_initial_val = 0;
static uint8_t power_status = POWER_ON;
static uint8_t power_key_new = POWER_KEY_UP;
static uint8_t power_key_last = POWER_KEY_UP;
static int touch_flag = false;

/*
  Read values sensed at available touch pads.
  Use 2 / 3 of read value as the threshold
  to trigger interrupt when the pad is touched.
  Note: this routine demonstrates a simple way
  to configure activation threshold for the touch pads.
  Do not touch any pads when this routine
  is running (on application start).
 */
static void touch_set_thresholds(void)
{
    uint16_t touch_value;
    touch_pad_read_filtered(TOUCH_PAD_CHAN, &touch_value);
    touch_initial_val = touch_value * 19 / 20 + 10;
    LOG_I(TAG, "Touch get value : %d, set thresh : %d", touch_value, touch_initial_val);
    // ESP_ERROR_CHECK(touch_pad_set_thresh(TOUCH_PAD_CHAN, touch_initial_val));
    ESP_ERROR_CHECK(touch_pad_set_thresh(TOUCH_PAD_CHAN, 600));
}

/*
  Handle an interrupt triggered when a pad is touched.
  Recognize what pad has been touched and save it in a table.
 */
static void touch_rtc_intr(void * arg)
{
    uint32_t pad_intr = touch_pad_get_status();
    touch_pad_clear_status();
    if((pad_intr >> TOUCH_PAD_CHAN) & 0x01)
    {
        touch_flag = true;
    }
}

/**
 * @brief 初始化触摸接口
 */
void touch_key_init(void)
{
    touch_pad_init();
    touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER);
    touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V);
    touch_pad_config(TOUCH_PAD_CHAN, 0);
    touch_pad_filter_start(10);
    touch_set_thresholds();
    touch_pad_isr_register(touch_rtc_intr, NULL);
    touch_pad_set_trigger_mode(TOUCH_TRIGGER_BELOW);
    touch_pad_intr_enable();

    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;

    io_conf.pin_bit_mask = 1ULL << POWER_KEY;
    gpio_config(&io_conf);

    io_conf.pin_bit_mask = 1ULL << REACTION_KEY;
    gpio_config(&io_conf);

    io_conf.pin_bit_mask = 1ULL << BAT_CHARGING;
    gpio_config(&io_conf);

    io_conf.pin_bit_mask = 1ULL << BAT_CHARGE_FULLY;
    gpio_config(&io_conf);
}

/**
 * @brief 获取原始触摸值
 * @return uint16_t [触摸值]
 */
uint16_t get_touch_key_value(void)
{
    uint16_t touch_value;
    touch_pad_read_filtered(TOUCH_PAD_CHAN, &touch_value);
    return touch_value;
}

/**
 * @brief 获取触摸状态
 * @return int [触摸状态，1为有触摸，0为无触摸]
 */
int get_touch_key_status(void)
{    
    int status = touch_flag;
    if(touch_flag == true)
        touch_flag = false;

    return status;
}

/**
 * @brief 获取电源按键状态
 * @return int [按键状态，0为按下，1为弹起]
 */
int get_power_key_status(void)
{
    return gpio_get_level(POWER_KEY);
}

/**
 * @brief 获取人体红外感应开关状态
 * @return int [人体红外感应开关状态，1为自动模式，0为手动模式]
 */
int get_reaction_key_status(void)
{
    return gpio_get_level(REACTION_KEY);
}

/**
 * @brief 获取电池的充电状态
 * @return int [电池充电状态，1为未充电，0为充电中]
 */
int get_battery_charge_status(void)
{
    return gpio_get_level(BAT_CHARGING);
}

/**
 * @brief 获取电池的充满电状态
 * @return int [电池充电满电状态，1为未充满，0为已充满]
 */
int get_battery_charge_fully_status(void)
{
    return gpio_get_level(BAT_CHARGE_FULLY);
}

/**
 * @brief 检查电源按键状态，实时更新电源状态
 */
void check_power_key_status(void)
{
    power_key_new = get_power_key_status();
    if(power_key_last != power_key_new)
    {
        power_key_last = power_key_new;
        if(power_key_new == POWER_KEY_DOWN)
        {
            if(power_status == POWER_ON)
                power_status = POWER_OFF;
            else
                power_status = POWER_ON;
        }
    }
}

/**
 * @brief 获取设备电源开关状态
 * @return uint8_t [电源开关状态]
 */
uint8_t get_device_power_status(void)
{
    return power_status;
}
