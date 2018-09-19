#ifndef _STM8S105_TIMER_H_
	#define _STM8S105_TIMER_H_

#ifdef _STM8S105_TIMER_C_
	#define OS_EXT
#else
	#define OS_EXT extern
#endif

typedef enum
{
	TIM2_PRESCALER_1     = ((uint8_t)0x00),
	TIM2_PRESCALER_2     = ((uint8_t)0x01),
	TIM2_PRESCALER_4     = ((uint8_t)0x02),
	TIM2_PRESCALER_8     = ((uint8_t)0x03),
	TIM2_PRESCALER_16    = ((uint8_t)0x04),
	TIM2_PRESCALER_32    = ((uint8_t)0x05),
	TIM2_PRESCALER_64    = ((uint8_t)0x06),
	TIM2_PRESCALER_128   = ((uint8_t)0x07),
	TIM2_PRESCALER_256   = ((uint8_t)0x08),
	TIM2_PRESCALER_512   = ((uint8_t)0x09),
	TIM2_PRESCALER_1024  = ((uint8_t)0x0A),
	TIM2_PRESCALER_2048  = ((uint8_t)0x0B),
	TIM2_PRESCALER_4096  = ((uint8_t)0x0C),
	TIM2_PRESCALER_8192  = ((uint8_t)0x0D),
	TIM2_PRESCALER_16384 = ((uint8_t)0x0E),
	TIM2_PRESCALER_32768 = ((uint8_t)0x0F)
}TIM2_Prescaler_TypeDef;
typedef enum
{
	TIM2_PSCRELOADMODE_UPDATE    = ((uint8_t)0x00),
	TIM2_PSCRELOADMODE_IMMEDIATE = ((uint8_t)0x01)
}TIM2_PSCReloadMode_TypeDef;
typedef enum
{
	TIM2_OCMODE_TIMING   = ((uint8_t)0x00),
	TIM2_OCMODE_ACTIVE   = ((uint8_t)0x10),
	TIM2_OCMODE_INACTIVE = ((uint8_t)0x20),
	TIM2_OCMODE_TOGGLE   = ((uint8_t)0x30),
	TIM2_OCMODE_PWM1     = ((uint8_t)0x60),
	TIM2_OCMODE_PWM2     = ((uint8_t)0x70)
}TIM2_OCMode_TypeDef;
typedef enum
{
	TIM2_OUTPUTSTATE_DISABLE = ((uint8_t)0x00),
	TIM2_OUTPUTSTATE_ENABLE  = ((uint8_t)0x11)
}TIM2_OutputState_TypeDef;
typedef enum
{
	TIM2_OCPOLARITY_HIGH = ((uint8_t)0x00),
	TIM2_OCPOLARITY_LOW  = ((uint8_t)0x22)
}TIM2_OCPolarity_TypeDef;


typedef enum
{
	TIM3_PRESCALER_1     = ((uint8_t)0x00),
	TIM3_PRESCALER_2     = ((uint8_t)0x01),
	TIM3_PRESCALER_4     = ((uint8_t)0x02),
	TIM3_PRESCALER_8     = ((uint8_t)0x03),
	TIM3_PRESCALER_16    = ((uint8_t)0x04),
	TIM3_PRESCALER_32    = ((uint8_t)0x05),
	TIM3_PRESCALER_64    = ((uint8_t)0x06),
	TIM3_PRESCALER_128   = ((uint8_t)0x07),
	TIM3_PRESCALER_256   = ((uint8_t)0x08),
	TIM3_PRESCALER_512   = ((uint8_t)0x09),
	TIM3_PRESCALER_1024  = ((uint8_t)0x0A),
	TIM3_PRESCALER_2048  = ((uint8_t)0x0B),
	TIM3_PRESCALER_4096  = ((uint8_t)0x0C),
	TIM3_PRESCALER_8192  = ((uint8_t)0x0D),
	TIM3_PRESCALER_16384 = ((uint8_t)0x0E),
	TIM3_PRESCALER_32768 = ((uint8_t)0x0F)
}TIM3_Prescaler_TypeDef;
typedef enum
{
	TIM3_OCMODE_TIMING   = ((uint8_t)0x00),
	TIM3_OCMODE_ACTIVE   = ((uint8_t)0x10),
	TIM3_OCMODE_INACTIVE = ((uint8_t)0x20),
	TIM3_OCMODE_TOGGLE   = ((uint8_t)0x30),
	TIM3_OCMODE_PWM1     = ((uint8_t)0x60),
	TIM3_OCMODE_PWM2     = ((uint8_t)0x70)
}TIM3_OCMode_TypeDef;
typedef enum
{
	TIM3_OUTPUTSTATE_DISABLE = ((uint8_t)0x00),
	TIM3_OUTPUTSTATE_ENABLE  = ((uint8_t)0x11)
}TIM3_OutputState_TypeDef;
typedef enum
{
	TIM3_OCPOLARITY_HIGH = ((uint8_t)0x00),
	TIM3_OCPOLARITY_LOW  = ((uint8_t)0x22)
}TIM3_OCPolarity_TypeDef;

typedef enum
{
	TIM4_PRESCALER_1   = ((uint8_t)0x00),
	TIM4_PRESCALER_2   = ((uint8_t)0x01),
	TIM4_PRESCALER_4   = ((uint8_t)0x02),
	TIM4_PRESCALER_8   = ((uint8_t)0x03),
	TIM4_PRESCALER_16  = ((uint8_t)0x04),
	TIM4_PRESCALER_32  = ((uint8_t)0x05),
	TIM4_PRESCALER_64  = ((uint8_t)0x06),
	TIM4_PRESCALER_128 = ((uint8_t)0x07)
}TIM4_Prescaler_TypeDef;
typedef enum
{
	TIM4_FLAG_UPDATE = ((uint8_t)0x01)
}TIM4_FLAG_TypeDef;
typedef enum
{
	TIM4_IT_UPDATE = ((uint8_t)0x01)
}TIM4_IT_TypeDef;






OS_EXT volatile u16 SysTickTimer;
OS_EXT volatile u16 SysTickMSec;
OS_EXT volatile u8  SysTickSecond;
OS_EXT volatile u8  SysTickMinute;

enum SystemTimingTask			// 定时任务时钟
{
	KEY_SCAN_CLK,				// ms 级 (按键扫描)
	GET_AD_IO_VALUE_CLK,		// ms 级 (取ADIO值)
	SYSTEM_SLEEP_CLK,			// min级 (系统睡眠)
	BT_LED_BLINK_CLK,			// ms 级 (蓝牙LED 闪烁)
	BT_PAIRING_PULSE_CLK,		// ms 级 (蓝牙pair脉冲)
	BT_ANSWER_SHORT_PULSE_CLK,		// ms 级 (蓝牙Answer脉冲)
	BT_ANSWER_DOUBLE_PULSE_CLK,		// ms 级 (蓝牙Answer脉冲)
	BT_PLAY_PULSE_CLK,			// ms 级 (蓝牙Play脉冲)
	BT_POWER_PULSE_CLK,			// ms 级 (蓝牙 POWER 脉冲)
	SOURCE_SWITCH_PAUSE_CLK,	// ms 级 (Source 切换发暂停的最快频率)
	CHARGE_LED_BLINK_CLK,			// ms级  (按键松开后在时间)
	BT_ANSWER_HANDLE,
	BT_VOLMAX_CLK,
	BT_LOWBATTERY_CLK,
	
	CLOCK_NUMBER				// 时钟数量
};
OS_EXT volatile u16 Clock[CLOCK_NUMBER];


OS_EXT void MCU_Timer_Init(void);

OS_EXT @near @interrupt void TIM2_UPD_OVF_BRK_IRQHandler(void);
OS_EXT @near @interrupt void TIM2_CAP_COM_IRQHandler(void);
OS_EXT @near @interrupt void TIM4_UPD_OVF_IRQHandler(void);



OS_EXT void TIM2_TimeBaseInit(TIM2_Prescaler_TypeDef TIM2_Prescaler, u16 TIM2_Period);
OS_EXT void TIM2_PrescalerConfig(TIM2_Prescaler_TypeDef Prescaler, TIM2_PSCReloadMode_TypeDef TIM2_PSCReloadMode);
OS_EXT void TIM2_OC1Init(TIM2_OCMode_TypeDef TIM2_OCMode, TIM2_OutputState_TypeDef TIM2_OutputState, u8 TIM2_Pulse, TIM2_OCPolarity_TypeDef TIM2_OCPolarity);
OS_EXT void TIM2_OC1PreloadConfig(FunctionalState NewState);
OS_EXT void TIM2_ARRPreloadConfig(FunctionalState NewState);


OS_EXT void TIM2_Cmd(FunctionalState NewState);

OS_EXT void TIM4_TimeBaseInit(TIM4_Prescaler_TypeDef TIM4_Prescaler, u8 TIM4_Period);
OS_EXT void TIM4_ClearFlag(TIM4_FLAG_TypeDef TIM4_FLAG);
OS_EXT void TIM4_ITConfig(TIM4_IT_TypeDef TIM4_IT, FunctionalState NewState);
OS_EXT void TIM4_Cmd(FunctionalState NewState);
OS_EXT void TIM4_ClearITPendingBit(TIM4_IT_TypeDef TIM4_IT);

void Beeper_SetPlay(bool bPlay);

	#undef OS_EXT
#endif


