#ifndef _STM8S105_GLOBAL_H_
	#define _STM8S105_GLOBAL_H_

#ifdef _STM8S105_GLOBAL_C_
	#define OS_EXT
#else
	#define OS_EXT extern
#endif


typedef enum
{
	GPIO_Mode_In_FL_No_IT      = (uint8_t)0b00000000,	// Input floating, no external interrupt
	GPIO_Mode_In_PU_No_IT      = (uint8_t)0b01000000,	// Input pull-up, no external interrupt
	GPIO_Mode_In_FL_IT         = (uint8_t)0b00100000,	// Input floating, external interrupt
	GPIO_Mode_In_PU_IT         = (uint8_t)0b01100000,	// Input pull-up, external interrupt
	GPIO_Mode_Out_OD_Low_Fast  = (uint8_t)0b10100000,	// Output open-drain, low level, 10MHz
	GPIO_Mode_Out_PP_Low_Fast  = (uint8_t)0b11100000,	// Output push-pull, low level, 10MHz
	GPIO_Mode_Out_OD_Low_Slow  = (uint8_t)0b10000000,	// Output open-drain, low level, 2MHz
	GPIO_Mode_Out_PP_Low_Slow  = (uint8_t)0b11000000,	// Output push-pull, low level, 2MHz
	GPIO_Mode_Out_OD_HiZ_Fast  = (uint8_t)0b10110000,	// Output open-drain, high-impedance level, 10MHz
	GPIO_Mode_Out_PP_High_Fast = (uint8_t)0b11110000,	// Output push-pull, high level, 10MHz
	GPIO_Mode_Out_OD_HiZ_Slow  = (uint8_t)0b10010000,	// Output open-drain, high-impedance level, 2MHz
	GPIO_Mode_Out_PP_High_Slow = (uint8_t)0b11010000	// Output push-pull, high level, 2MHz
}GPIO_Mode_TypeDef;

typedef enum
{
	GPIO_Pin_0    = ((uint8_t)0x01),	// Pin 0 selected
	GPIO_Pin_1    = ((uint8_t)0x02),	// Pin 1 selected
	GPIO_Pin_2    = ((uint8_t)0x04),	// Pin 2 selected
	GPIO_Pin_3    = ((uint8_t)0x08),	// Pin 3 selected
	GPIO_Pin_4    = ((uint8_t)0x10),	// Pin 4 selected
	GPIO_Pin_5    = ((uint8_t)0x20),	// Pin 5 selected
	GPIO_Pin_6    = ((uint8_t)0x40),	// Pin 6 selected
	GPIO_Pin_7    = ((uint8_t)0x80),	// Pin 7 selected
	GPIO_Pin_LNib = ((uint8_t)0x0F),	// Low nibble pins selected
	GPIO_Pin_HNib = ((uint8_t)0xF0),	// High nibble pins selected
	GPIO_Pin_All  = ((uint8_t)0xFF)		// All pins selected
}GPIO_Pin_TypeDef;

#define NOP		_asm("nop")
#define RIM		_asm("rim")				// 开总中断
#define SIM		_asm("sim")				// 关总中断
#define HALT	_asm("halt\n")


//////////////////////////////////////////////////////////////
//BT LED PC4
#define BT_LED_ON				(GPIOC->ODR.field.B4 = 1)	
#define BT_LED_OFF				(GPIOC->ODR.field.B4 = 0)
#define BT_LED_PDDR				(GPIOC->DDR.field.B4)
#define BT_LED_PCR1				(GPIOC->CR1.field.B4)	

//AUX LED PC5
#define LINE_IN_LED_ON			(GPIOC->ODR.field.B5 = 1)	
#define LINE_IN_LED_OFF			(GPIOC->ODR.field.B5 = 0)
#define LINE_IN_LED_PDDR		(GPIOC->DDR.field.B5)
#define LINE_IN_LED_PCR1		(GPIOC->CR1.field.B5)		

//RED LED PC6
#define RED_LED_ON				(GPIOC->ODR.field.B6 = 1)
#define RED_LED_OFF				(GPIOC->ODR.field.B6 = 0)
#define RED_LED_PDDR			(GPIOC->DDR.field.B6)
#define RED_LED_PCR1			(GPIOC->CR1.field.B6)	

//POWER LED PC7
#define POWER_LED_ON			(GPIOC->ODR.field.B7 = 1)
#define POWER_LED_OFF			(GPIOC->ODR.field.B7 = 0)
#define VOL_OFF_READ			(GPIOC->IDR.field.B7)
#define POWER_LED_PDDR			(GPIOC->DDR.field.B7)
#define POWER_LED_PCR1			(GPIOC->CR1.field.B7)		

//////////////////////////////////////////////////////////////
//AIN0 KEY PB0
#define ADC_LINE0_PDDR			(GPIOB->DDR.field.B0)		
#define ADC_LINE0_PCR1			(GPIOB->CR1.field.B0)		
#define ADC_LINE0_PIDR			(GPIOB->IDR.field.B0)

//AIN1 KEY PB1
#define ADC_LINE1_PDDR			(GPIOB->DDR.field.B1)		
#define ADC_LINE1_PCR1			(GPIOB->CR1.field.B1)		
#define ADC_LINE1_PIDR			(GPIOB->IDR.field.B1)

//AIN2 BAT CHECK PB2
#define ADC_LINE2_PDDR			(GPIOB->DDR.field.B2)		
#define ADC_LINE2_PCR1			(GPIOB->CR1.field.B2)		
#define ADC_LINE2_PIDR			(GPIOB->IDR.field.B2)

//AIN3 BAT NTC PB3
#define ADC_LINE3_READ			(GPIOB->IDR.field.B3)
#define ADC_LINE3_PDDR			(GPIOB->DDR.field.B3)		
#define ADC_LINE3_PCR1			(GPIOB->CR1.field.B3)	

//I2C SCL PB4
#define PDDR_IIC_SCL				GPIOB->DDR.field.B4
#define PCR1_IIC_SCL					GPIOB->CR1.field.B4
#define PCR2_IIC_SCL					GPIOB->CR2.field.B4
#define PIDR_IIC_SCL					GPIOB->IDR.field.B4
#define Write_P_IIC_SCL_1				(GPIOB->ODR.field.B4 = 1)	
#define Write_P_IIC_SCL_0				(GPIOB->ODR.field.B4 = 0)	

//I2C SDA PB5
#define PDDR_IIC_SDA				GPIOB->DDR.field.B5
#define PCR1_IIC_SDA				GPIOB->CR1.field.B5
#define PCR2_IIC_SDA				GPIOB->CR2.field.B5
#define PIDR_SDA				GPIOB->IDR.field.B5
#define Write_P_IIC_SDA_1				(GPIOB->ODR.field.B5 = 1)	
#define Write_P_IIC_SDA_0				(GPIOB->ODR.field.B5 = 0)	

//AUX CHECK PB6
#define AUX_CHECK_READ			(GPIOB->IDR.field.B6)	
#define AUX_CHECK_PDDR			(GPIOB->DDR.field.B6)		
#define AUX_CHECK_PCR1			(GPIOB->CR1.field.B6)	

//SWITCH CHECK PB7
#define SWITCH_CHECK_READ			(GPIOB->IDR.field.B7)	
#define SWITCH_CHECK_PDDR			(GPIOB->DDR.field.B7)		
#define SWITCH_CHECK_PCR1			(GPIOB->CR1.field.B7)	
//////////////////////////////////////////////////////////////

//SLEEP PF4
#define SLEEP_PDDR			(GPIOF->DDR.field.B4)		
#define SLEEP_PCR1			(GPIOF->CR1.field.B4)	
#define SLEEP_PIDR_READ	(GPIOF->IDR.field.B4)	
//////////////////////////////////////////////////////////////
//BT POWER PA1
#define BT_POWER_H				(GPIOA->ODR.field.B1 = 1)
#define BT_POWER_L				(GPIOA->ODR.field.B1 = 0)
#define BT_POWER_PDDR			(GPIOA->DDR.field.B1)		
#define BT_POWER_PCR1			(GPIOA->CR1.field.B1)		

//CHARGE PA2
#define CHARGE_CHECK_PDDR			(GPIOA->DDR.field.B2)		
#define CHARGE_CHECK_PCR1			(GPIOA->CR1.field.B2)	
#define READ_CHARGE_STATU			(GPIOA->IDR.field.B2)

//AMP ON PA3
#define AMP_ON_PDDR			(GPIOA->DDR.field.B3)		
#define AMP_ON_PCR1			(GPIOA->CR1.field.B3)	
#define AMP_ON_H				(GPIOA->ODR.field.B3 = 1)	
#define AMP_ON_L				(GPIOA->ODR.field.B3 = 0)	
//////////////////////////////////////////////////////////////
//PD0
#define BT_PHONE_READ			(GPIOD->IDR.field.B0)		
#define BT_PHONE_PDDR			(GPIOD->DDR.field.B0)		
#define BT_PHONE_PCR1			(GPIOD->CR1.field.B0)		

//SEND ANSWER PULSE PD2
#define BT_ANSWER_BUTTON_H		(GPIOD->ODR.field.B2 = 1)
#define BT_ANSWER_BUTTON_L		(GPIOD->ODR.field.B2 = 0)	
#define BT_ANSWER_BUTTON_PDDR	(GPIOD->DDR.field.B2)		
#define BT_ANSWER_BUTTON_PCR1	(GPIOD->CR1.field.B2)	
//PD3
#define BT_LOWBATTERY_L				(GPIOD->ODR.field.B3 = 0)		
#define BT_LOWBATTERY_H				(GPIOD->ODR.field.B3 = 1)		
#define BT_LOWBATTERY_PDDR			(GPIOD->DDR.field.B3)		
#define BT_LOWBATTERY_PCR1			(GPIOD->CR1.field.B3)	

//SEND PAIR PULSE PD4
#define BT_PAIR_BUTTON_H			(GPIOD->ODR.field.B4 = 1)
#define BT_PAIR_BUTTON_L			(GPIOD->ODR.field.B4 = 0)	
#define BT_PAIR_BUTTON_PDDR		(GPIOD->DDR.field.B4)		
#define BT_PAIR_BUTTON_PCR1		(GPIOD->CR1.field.B4)		

//PD5
#define BT_PAIRED_READ			(GPIOD->IDR.field.B5)		
#define BT_PAIRED_PDDR			(GPIOD->DDR.field.B5)		
#define BT_PAIRED_PCR1			(GPIOD->CR1.field.B5)		

//PD6
#define BT_PAIRING_READ			(GPIOD->IDR.field.B6)		
#define BT_PAIRING_PDDR			(GPIOD->DDR.field.B6)		
#define BT_PAIRING_PCR1				(GPIOD->CR1.field.B6)	

#define BT_VOLMAX_H			(GPIOD->ODR.field.B7 = 1)
#define BT_VOLMAX_L				(GPIOD->ODR.field.B7 = 0)	
#define BT_VOLMAX_PDDR			(GPIOD->DDR.field.B7)		
#define BT_VOLMAX_PCR1			(GPIOD->CR1.field.B7)	
//PC1
#define PDDR_BEEP_PIN1				(GPIOC->DDR.field.B1)		
#define PCR1_BEEP_PIN1				(GPIOC->CR1.field.B1)		
#define PODR_BEEP_PIN1				(GPIOC->ODR.field.B1)

//PE5
#define PDDR_BEEP_PIN2				(GPIOE->DDR.field.B5)		
#define PCR1_BEEP_PIN2				(GPIOE->CR1.field.B5)		
#define PODR_BEEP_PIN2				(GPIOE->ODR.field.B5)

//////////////////////////////////////////////////////////////



OS_EXT void GPIO_Init(GPIO_TypeDef* GPIOx, u8 GPIO_Pin, GPIO_Mode_TypeDef GPIO_Mode);

OS_EXT void MCU_CLK_Init(void);
OS_EXT void GPIO_Configuration(void);





	#undef OS_EXT
#endif

