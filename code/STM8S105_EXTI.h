#ifndef _STM8S105_EXTI_H_
	#define _STM8S105_EXTI_H_

#ifdef _STM8S105_EXTI_C_
	#define OS_EXT
#else
	#define OS_EXT extern
#endif

typedef enum
{
	EXTI_PORT_GPIOA = (uint8_t)0x00,	// GPIO Port A
	EXTI_PORT_GPIOB = (uint8_t)0x01,	// GPIO Port B
	EXTI_PORT_GPIOC = (uint8_t)0x02,	// GPIO Port C
	EXTI_PORT_GPIOD = (uint8_t)0x03,	// GPIO Port D
	EXTI_PORT_GPIOE = (uint8_t)0x04		// GPIO Port E
}EXTI_Port_TypeDef;

typedef enum
{
	EXTI_SENSITIVITY_FALL_LOW  = (uint8_t)0x00,	// Interrupt on Falling edge and Low level
	EXTI_SENSITIVITY_RISE_ONLY = (uint8_t)0x01,	// Interrupt on Rising edge only
	EXTI_SENSITIVITY_FALL_ONLY = (uint8_t)0x02,	// Interrupt on Falling edge only
	EXTI_SENSITIVITY_RISE_FALL = (uint8_t)0x03	// Interrupt on Rising and Falling edges
}EXTI_Sensitivity_TypeDef;

typedef enum
{
	EXTI_TLISENSITIVITY_FALL_ONLY = (uint8_t)0x00,	// Top Level Interrupt on Falling edge only
	EXTI_TLISENSITIVITY_RISE_ONLY = (uint8_t)0x04	// Top Level Interrupt on Rising edge only
}EXTI_TLISensitivity_TypeDef;


OS_EXT void MCU_EXTI_Init(void);

OS_EXT @near @interrupt void EXTI_PORTA_IRQHandler(void);
OS_EXT @near @interrupt void EXTI_PORTB_IRQHandler(void);
OS_EXT @near @interrupt void EXTI_PORTC_IRQHandler(void);
OS_EXT @near @interrupt void EXTI_PORTD_IRQHandler(void);
OS_EXT @near @interrupt void EXTI_PORTE_IRQHandler(void);

OS_EXT void EXTI_INT_Disable(GPIO_TypeDef* GPIOx, u8 GPIO_Pin);
OS_EXT void EXTI_INT_Enable(GPIO_TypeDef* GPIOx, u8 GPIO_Pin);
OS_EXT void EXTI_DeInit(void);

OS_EXT void EXTI_SetExtIntSensitivity(EXTI_Port_TypeDef Port, EXTI_Sensitivity_TypeDef SensitivityValue);
OS_EXT void EXTI_SetTLISensitivity(EXTI_TLISensitivity_TypeDef SensitivityValue);


	#undef OS_EXT
#endif

