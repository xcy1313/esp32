#ifndef _STM8S105_ADC_H_
	#define _STM8S105_ADC_H_

#ifdef _STM8S105_ADC_C_
	#define OS_EXT
#else
	#define OS_EXT extern
#endif


typedef enum 
{
	ADC1_CONVERSIONMODE_SINGLE     = (uint8_t)0x00,	// Single conversion mode
	ADC1_CONVERSIONMODE_CONTINUOUS = (uint8_t)0x01	// Continuous conversion mode
}ADC1_ConvMode_TypeDef;
typedef enum 
{
	ADC1_CHANNEL_0  = (uint8_t)0x00,	// Analog channel 0
	ADC1_CHANNEL_1  = (uint8_t)0x01,	// Analog channel 1
	ADC1_CHANNEL_2  = (uint8_t)0x02,	// Analog channel 2
	ADC1_CHANNEL_3  = (uint8_t)0x03,	// Analog channel 3
	ADC1_CHANNEL_4  = (uint8_t)0x04,	// Analog channel 4
	ADC1_CHANNEL_5  = (uint8_t)0x05,	// Analog channel 5
	ADC1_CHANNEL_6  = (uint8_t)0x06,	// Analog channel 6
	ADC1_CHANNEL_7  = (uint8_t)0x07,	// Analog channel 7
	ADC1_CHANNEL_8  = (uint8_t)0x08,	// Analog channel 8
	ADC1_CHANNEL_9  = (uint8_t)0x09,	// Analog channel 9
	ADC1_CHANNEL_12 = (uint8_t)0x0C		// Analog channel 12 refer to product datasheet for channel 12 availability
}ADC1_Channel_TypeDef;
typedef enum 
{
	ADC1_ALIGN_LEFT  = (uint8_t)0x00,	// Data alignment left
	ADC1_ALIGN_RIGHT = (uint8_t)0x08	// Data alignment right
}ADC1_Align_TypeDef;
typedef enum 
{
	ADC1_PRESSEL_FCPU_D2  = (uint8_t)0x00,	// Prescaler selection fADC1 = fcpu/2
	ADC1_PRESSEL_FCPU_D3  = (uint8_t)0x10,	// Prescaler selection fADC1 = fcpu/3
	ADC1_PRESSEL_FCPU_D4  = (uint8_t)0x20,	// Prescaler selection fADC1 = fcpu/4
	ADC1_PRESSEL_FCPU_D6  = (uint8_t)0x30,	// Prescaler selection fADC1 = fcpu/6
	ADC1_PRESSEL_FCPU_D8  = (uint8_t)0x40,	// Prescaler selection fADC1 = fcpu/8
	ADC1_PRESSEL_FCPU_D10 = (uint8_t)0x50,	// Prescaler selection fADC1 = fcpu/10
	ADC1_PRESSEL_FCPU_D12 = (uint8_t)0x60,	// Prescaler selection fADC1 = fcpu/12
	ADC1_PRESSEL_FCPU_D18 = (uint8_t)0x70	// Prescaler selection fADC1 = fcpu/18
}ADC1_PresSel_TypeDef;
typedef enum 
{
	ADC1_EXTTRIG_TIM  = (uint8_t)0x00,	// Conversion from Internal TIM1 TRGO event
	ADC1_EXTTRIG_GPIO = (uint8_t)0x10	// Conversion from External interrupt on ADC_ETR pin
}ADC1_ExtTrig_TypeDef;
typedef enum 
{
	ADC1_SCHMITTTRIG_CHANNEL0  = (uint8_t)0x00,	// Schmitt trigger disable on AIN0
	ADC1_SCHMITTTRIG_CHANNEL1  = (uint8_t)0x01,	// Schmitt trigger disable on AIN1
	ADC1_SCHMITTTRIG_CHANNEL2  = (uint8_t)0x02,	// Schmitt trigger disable on AIN2
	ADC1_SCHMITTTRIG_CHANNEL3  = (uint8_t)0x03,	// Schmitt trigger disable on AIN3
	ADC1_SCHMITTTRIG_CHANNEL4  = (uint8_t)0x04,	// Schmitt trigger disable on AIN4
	ADC1_SCHMITTTRIG_CHANNEL5  = (uint8_t)0x05,	// Schmitt trigger disable on AIN5
	ADC1_SCHMITTTRIG_CHANNEL6  = (uint8_t)0x06,	// Schmitt trigger disable on AIN6
	ADC1_SCHMITTTRIG_CHANNEL7  = (uint8_t)0x07,	// Schmitt trigger disable on AIN7
	ADC1_SCHMITTTRIG_CHANNEL8  = (uint8_t)0x08,	// Schmitt trigger disable on AIN8
	ADC1_SCHMITTTRIG_CHANNEL9  = (uint8_t)0x09,	// Schmitt trigger disable on AIN9
	ADC1_SCHMITTTRIG_CHANNEL12 = (uint8_t)0x0C,	// Schmitt trigger disable on AIN12 refer to product datasheet for channel 12 availability
	ADC1_SCHMITTTRIG_ALL       = (uint8_t)0xFF	// Schmitt trigger disable on All channels
}ADC1_SchmittTrigg_TypeDef;
typedef enum 
{
	ADC1_IT_AWDIE = (uint16_t)0x010,	// Analog WDG interrupt enable
	ADC1_IT_EOCIE = (uint16_t)0x020,	// EOC interrupt enable
	ADC1_IT_AWD   = (uint16_t)0x140,	// Analog WDG status
	ADC1_IT_AWS0  = (uint16_t)0x110,	// Analog channel 0 status
	ADC1_IT_AWS1  = (uint16_t)0x111,	// Analog channel 1 status
	ADC1_IT_AWS2  = (uint16_t)0x112,	// Analog channel 2 status
	ADC1_IT_AWS3  = (uint16_t)0x113,	// Analog channel 3 status
	ADC1_IT_AWS4  = (uint16_t)0x114,	// Analog channel 4 status
	ADC1_IT_AWS5  = (uint16_t)0x115,	// Analog channel 5 status
	ADC1_IT_AWS6  = (uint16_t)0x116,	// Analog channel 6 status
	ADC1_IT_AWS7  = (uint16_t)0x117,	// Analog channel 7 status
	ADC1_IT_AWS8  = (uint16_t)0x118,	// Analog channel 8 status
	ADC1_IT_AWS9  = (uint16_t)0x119,	// Analog channel 9 status
	ADC1_IT_AWS12 = (uint16_t)0x11C,	// Analog channel 12 status refer to product datasheet for channel 12 availability
	ADC1_IT_EOC   = (uint16_t)0x080		// EOC pending bit
}ADC1_IT_TypeDef;


#define ADC_CH_NUM		2					// 系统用到AD的通道数


OS_EXT void MCU_ADC_Init(void);
OS_EXT u16 ADC_GetADValue(u8 u8AdCh);

OS_EXT @near @interrupt void ADC1_IRQHandler(void);
OS_EXT void ADC1_DeInit(void);

OS_EXT void ADC1_ConversionConfig(ADC1_ConvMode_TypeDef ADC1_ConversionMode, ADC1_Channel_TypeDef ADC1_Channel, ADC1_Align_TypeDef ADC1_Align);
OS_EXT void ADC1_PrescalerConfig(ADC1_PresSel_TypeDef ADC1_Prescaler);
OS_EXT void ADC1_ExternalTriggerConfig(ADC1_ExtTrig_TypeDef ADC1_ExtTrigger, FunctionalState NewState);
OS_EXT void ADC1_SchmittTriggerConfig(ADC1_SchmittTrigg_TypeDef ADC1_SchmittTriggerChannel, FunctionalState NewState);

OS_EXT void ADC1_DataBufferCmd(FunctionalState NewState);
OS_EXT void ADC1_Cmd(FunctionalState NewState);
OS_EXT void ADC1_ScanModeCmd(FunctionalState NewState);
OS_EXT u16  ADC1_GetBufferValue(u8 Buffer);

OS_EXT void ADC1_ITConfig(ADC1_IT_TypeDef ADC1_IT, FunctionalState NewState);
OS_EXT void ADC1_ClearITPendingBit(ADC1_IT_TypeDef ITPendingBit);




	#undef OS_EXT
#endif

