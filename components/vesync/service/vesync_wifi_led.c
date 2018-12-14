/**
 * @file vesync_wifi_led.c
 * @brief vesync设备统一的WiFi指示灯行为
 * @author WatWu
 * @date 2018-11-19
 */

#include "vesync_main.h"
#include "freertos/timers.h"

#include "vesync_wifi_led.h"

static const char* TAG = "vesync_wifi_led";

static TimerHandle_t led_blink_timer;					//wifi指示灯刷新定时器
static uint32_t blink_count;							//wifi指示灯闪烁次数
static uint32_t cur_count_times;						//记录当前闪烁的次数
static uint8_t blink_off_times;							//WiFi指示灯闪烁时熄灭时间与点亮时间的倍数
static uint8_t blink_status;							//记录当前闪烁的状态
static uint8_t cur_off_times;							//记录当前闪烁的关灯时间
static uint8_t double_blink_status = WIFI_LED_OFF;		//双闪标志
static wifi_led_status_e led_status_last = WIFI_LED_OFF;//LED灯的上次亮灭状态
static wifi_led_status_e led_status_new = WIFI_LED_OFF;	//LED灯的最新亮灭状态
static uint8_t wifi_led_timer_init_flag = false;		//LED灯刷新定时器初始化标志，避免重复初始化

static wifi_led_turn_cb_t wifi_led_turn_callback = NULL;//定义WiFi LED状态切换回调函数指针

/**
 * @brief WiFi指示灯实时变化驱动
 * @param status [WiFi指示灯的实时亮灭状态]
 */
static void wifi_led_turn(wifi_led_status_e status)
{
	led_status_new = status;

	if(led_status_last != led_status_new)		//亮灭状态变化了才进行外部驱动的回调，减少频繁调用
	{
		led_status_last = led_status_new;
		if(NULL != wifi_led_turn_callback)
			wifi_led_turn_callback(status);
	}
}

/**
 * [led_blink_timer_callback  WiFi指示灯闪烁定时器回调函数]
 * @param  arg [回调函数参数]
 * @return     [无]
 */
static void led_blink_timer_callback(void *arg)
{
	if(blink_status)
	{
		wifi_led_turn(WIFI_LED_ON);
		blink_status = false;
	}
	else
	{
		wifi_led_turn(WIFI_LED_OFF);
		cur_off_times++;
		if(double_blink_status == WIFI_LED_ON)			//双闪，按照首次亮的时间与后续的剩余行为时间进行亮灭分割
		{
			if(cur_off_times == 3)
			{
				wifi_led_turn(WIFI_LED_ON);
			}
			if(cur_off_times == 4)
			{
				wifi_led_turn(WIFI_LED_OFF);
			}
		}
		if(cur_off_times >= blink_off_times)
		{
			cur_off_times = 0;
			blink_status = true;

			if(blink_count != 0)						//有限次数的闪烁
			{
				cur_count_times++;						//每亮灭各完成一次后才算闪烁一次
				if(cur_count_times >= blink_count)
				{
					xTimerStop(led_blink_timer, TIMER_BLOCK_TIME);
					wifi_led_turn(WIFI_LED_OFF);
					cur_count_times = 0;
				}
			}
		}
	}
}

/**
 * @brief 初始化WiFi指示灯刷新定时器
 */
static void init_wifi_led_timer(void)
{
	//创建LED闪烁控制定时器，周期模式
	led_blink_timer = xTimerCreate("led_blink", 1000 / portTICK_RATE_MS, pdTRUE, NULL, led_blink_timer_callback);
}

/**
 * [vesync_set_wifi_led WiFi指示灯闪烁设置]
 * @param  led_status [闪烁状态，开启、关闭或闪烁]
 * @param  blink_ms   [闪烁时点亮时间，单位毫秒]
 * @param  off_times  [闪烁时熄灭时间与点亮时间的倍数]
 * @param  bl_times   [闪烁次数，达到次数后自动关闭，设置为0则为连续不断闪烁]
 * @return            [无]
 */
static void vesync_set_wifi_led(wifi_led_status_e led_status, uint32_t blink_ms, uint8_t off_times, uint32_t bl_times)
{
	if(false == wifi_led_timer_init_flag)
	{
		init_wifi_led_timer();
		wifi_led_timer_init_flag = true;
	}
	double_blink_status = WIFI_LED_OFF;					//默认不双闪

	switch(led_status)
	{
		case WIFI_LED_ON:
			xTimerStop(led_blink_timer, TIMER_BLOCK_TIME);
			wifi_led_turn(WIFI_LED_ON);
			break;
		case WIFI_LED_OFF:
			xTimerStop(led_blink_timer, TIMER_BLOCK_TIME);
			wifi_led_turn(WIFI_LED_OFF);
			break;
		case WIFI_LED_DOUBLE_BLINK:
			double_blink_status = WIFI_LED_ON;			//此处除了双闪标志外，其余参数与WIFI_LED_BLINK分支相同，不需要break；
		case WIFI_LED_BLINK:
			xTimerStop(led_blink_timer, TIMER_BLOCK_TIME);
			xTimerChangePeriod(led_blink_timer, blink_ms / portTICK_RATE_MS, TIMER_BLOCK_TIME);
			xTimerStart(led_blink_timer, TIMER_BLOCK_TIME);
			blink_count = bl_times;
			blink_off_times = off_times;
			blink_status = false;
			cur_off_times = 0;
			cur_count_times = 0;
			wifi_led_turn(WIFI_LED_ON);
			break;
	}
}

/**
 * [vesync_set_wifiled_behavior 设置WIFI指示灯闪烁模式]
 * @param  ledBehavior [WiFi指示灯闪烁行为]
 * @return      	   [无]
 */
void vesync_set_wifiled_behavior(wifi_led_behavior_e ledBehavior)
{
	static wifi_led_behavior_e ledBehavior_last = WIFI_LED_NOT_CONFIG;

	if(ledBehavior != ledBehavior_last || ledBehavior == WIFI_LED_RESET_DEVICE)	//欲设置的闪烁行为与当前闪烁行为不同才进行更新
	{
		ledBehavior_last = ledBehavior;
		switch(ledBehavior)
		{
			case WIFI_LED_LOGIN_SUCCESS:
				vesync_set_wifi_led(WIFI_LED_ON, 0, 0, 0);					//登录服务器成功，指示灯常亮
				break;

			case WIFI_LED_NOT_CONFIG:
				vesync_set_wifi_led(WIFI_LED_OFF, 0, 0, 0);					//未配网状态，指示灯常灭
				break;

			case WIFI_LED_APN_CONFIG:
				vesync_set_wifi_led(WIFI_LED_BLINK, 500, 1, 0);				//AP模式时，1秒的闪烁频率，连续闪烁
				break;

			case WIFI_LED_SMART_CONFIG:
				vesync_set_wifi_led(WIFI_LED_BLINK, 250, 1, 0);				//smartconfig时，0.5秒的闪烁频率，连续闪烁
				break;

			case WIFI_LED_RESET_DEVICE:
				vesync_set_wifi_led(WIFI_LED_BLINK, 125, 1, 10);			//清除配网数据时，0.25秒的闪烁频率，闪烁10次后停止
				break;

			case WIFI_LED_WIFI_LINK_FAIL:
				vesync_set_wifi_led(WIFI_LED_DOUBLE_BLINK, 100, 49, 0);		//WiFi连接失败，指示灯双闪（亮0.1s、灭0.2s、亮0.1s、灭4.6s）
				break;

			case WIFI_LED_SERVER_LINK_FAIL:
				vesync_set_wifi_led(WIFI_LED_BLINK, 1000, 4, 0);			//WiFi连接成功，服务器连接失败，指示灯亮1秒灭4秒进行闪烁
				break;

			case WIFI_LED_NET_LINKING:
				vesync_set_wifi_led(WIFI_LED_DOUBLE_BLINK, 100, 9, 0);		//配网连接中
				break;

			default :
				LOG_E(TAG, "LED behavior param undefined !");
				break;
		}
	}
}

/**
 * @brief 注册WiFi LED指示灯状态变化回调函数
 * @param cb [WiFi指示灯变化回调函数，每次指示灯进行亮灭变化时都会调用该函数]
 */
void vesync_regist_wifiled_cb(wifi_led_turn_cb_t cb)
{
	wifi_led_turn_callback = cb;
}
