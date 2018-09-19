#define _STM8S105_GLOBAL_C_
#include "include.h"


void GPIO_Reset(void);
/********************************************************************
Function Name:	MCU_CLK_Init
Description:	
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
void MCU_CLK_Init(void)
{
#ifdef SYS_CLK_M16MHz_C16MHz
	CLK->CKDIVR = 0x00;
#else
	CLK->CKDIVR = 0x10;			// Fmaster=4MHZ 4分频
#endif

	CLK->PCKENR1 = 0b10110000;	// TIM1,TIM3,TIM2,TIM4,UART2使用外设部的晶振
	CLK->PCKENR2 = 0b00001000;	// ADC使用外设部的晶振
	CLK->PCKENR2 |= 0x08;		// Enable the clock for ADC

	CLK->SWCR |= 0x02;			// 使能时钟切换
	CLK->SWR = 0xE1;			// HSI为主时钟源
	
//	CFG->GCR |= 0x02;
}

/********************************************************************
Function Name:	GPIO_Init
Description:	Initializes the GPIOx according to the specified parameters.
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
void GPIO_Init(GPIO_TypeDef* GPIOx, u8 GPIO_Pin, GPIO_Mode_TypeDef GPIO_Mode)
{
	if((((u8)(GPIO_Mode)) & (u8)0x80) != (u8)0x00)		// Output mode
	{
		if((((u8)(GPIO_Mode)) & (u8)0x10) != (u8)0x00)	// High level
		{
			GPIOx->ODR.byte |= GPIO_Pin;
		}
		else	// Low level
		{
			GPIOx->ODR.byte &= (u8)(~(GPIO_Pin));
		}

		GPIOx->DDR.byte |= GPIO_Pin;					// Set Output mode
	}
	else		// Input mode
	{
		GPIOx->DDR.byte &= (u8)(~(GPIO_Pin));			// Set Input mode
	}

	if((((u8)(GPIO_Mode)) & (u8)0x40) != (u8)0x00)		// Pull-Up or Push-Pull
	{
		GPIOx->CR1.byte |= GPIO_Pin;
	}
	else		// Float or Open-Drain
	{
		GPIOx->CR1.byte &= (u8)(~(GPIO_Pin));
	}

	if((((u8)(GPIO_Mode)) & (u8)0x20) != (u8)0x00)		// Interrupt or Slow slope
	{
		GPIOx->CR2.byte |= GPIO_Pin;
	}
	else		// No external interrupt or No slope control
	{
		GPIOx->CR2.byte &= (u8)(~(GPIO_Pin));
	}
}

/********************************************************************
Function Name:	GPIO_Configuration
Description:	
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
void GPIO_Configuration(void)
{
	GPIO_Reset();
	
/////////////////////////////////////OUTPUT////////////////////////////////
	//PC7
 	POWER_LED_PDDR = 1;			
 	POWER_LED_PCR1 = 1;			
	//PC6
	RED_LED_PDDR = 1;
	RED_LED_PCR1 = 1;
	//PC5
	LINE_IN_LED_PDDR = 1;		
 	LINE_IN_LED_PCR1 = 1;
	//PC4
	BT_LED_PDDR = 1;			
 	BT_LED_PCR1 = 1;	
	//PD4
	BT_PAIR_BUTTON_PDDR = 1;	
	BT_PAIR_BUTTON_PCR1 = 1;	
	//PD2
	BT_ANSWER_BUTTON_PDDR = 1;
	BT_ANSWER_BUTTON_PCR1 = 1;	
	//PD7
	BT_VOLMAX_PDDR = 1;
	BT_VOLMAX_PCR1 = 1;
	//PA1
	BT_POWER_PDDR = 1;			
	BT_POWER_PCR1 = 1;	
	//PB4
	PDDR_IIC_SCL = 1;
	PCR1_IIC_SCL = 1;
	//PB5
	PDDR_IIC_SDA = 1;
	PCR1_IIC_SDA = 1;

	//PC1
	PDDR_BEEP_PIN1 = 1;
	PCR1_BEEP_PIN1 = 1;
	//PE5
	PDDR_BEEP_PIN2 = 1;
	PCR1_BEEP_PIN2 = 1;
	//PA3
	AMP_ON_PDDR = 1;
	AMP_ON_PCR1 = 1;
	//PD3
	BT_LOWBATTERY_PDDR = 1;
	BT_LOWBATTERY_PCR1 = 1;
/////////////////////////////////////INPUT////////////////////////////////

	//PB0 
	ADC_LINE0_PDDR = 0;
	ADC_LINE0_PCR1 = 0;	
	//PB1
	ADC_LINE1_PDDR = 0;			
	ADC_LINE1_PCR1 = 0;			
	//PB2
	ADC_LINE2_PDDR = 0;			
	ADC_LINE2_PCR1 = 0;	
	//PB3
	ADC_LINE3_PDDR = 0;			
	ADC_LINE3_PCR1 = 0;			
	//PB6
	AUX_CHECK_PDDR = 0;
	AUX_CHECK_PCR1 = 0;
	//PB7
	SWITCH_CHECK_PDDR = 0;
	SWITCH_CHECK_PCR1 = 0;

	//PA2
	CHARGE_CHECK_PDDR = 0;
	CHARGE_CHECK_PCR1 = 0;
	//PD6
	BT_PAIRING_PDDR = 0;		
	BT_PAIRING_PCR1 = 0;		
	//PD5
	BT_PAIRED_PDDR = 0;			
	BT_PAIRED_PCR1 = 0;			
	//PD0
	BT_PHONE_PDDR = 0;			
	BT_PHONE_PCR1 = 0;		

	//PF4
	SLEEP_PDDR = 0;
	SLEEP_PCR1 = 0;

}

/********************************************************************
Function Name:	GPIO_Reset
Description:	
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
void GPIO_Reset(void)
{
	GPIO_Init(GPIOA, GPIO_Pin_All, GPIO_Mode_In_FL_No_IT);
	GPIO_Init(GPIOB, GPIO_Pin_All, GPIO_Mode_In_FL_No_IT);
	GPIO_Init(GPIOC, GPIO_Pin_All, GPIO_Mode_In_FL_No_IT);
	GPIO_Init(GPIOD, GPIO_Pin_All, GPIO_Mode_In_FL_No_IT);
	GPIO_Init(GPIOE, GPIO_Pin_All, GPIO_Mode_In_FL_No_IT);
	GPIO_Init(GPIOF, GPIO_Pin_All, GPIO_Mode_In_FL_No_IT);
}

