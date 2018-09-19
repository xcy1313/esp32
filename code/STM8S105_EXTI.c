#define _STM8S105_EXTI_C_
#include "include.h"

/********************************************************************
Function Name:	MCU_EXTI_Init
Description:	Íâ²¿ÖÐ¶Ï ÅäÖÃ
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
void MCU_EXTI_Init(void)
{
	EXTI_DeInit();
		
	EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOB, EXTI_SENSITIVITY_FALL_ONLY);

	EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOA, EXTI_SENSITIVITY_RISE_ONLY);
	
}

/********************************************************************
Function Name:	
Description:	External Interrupt PORTA Interruption routine.
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
@near @interrupt void EXTI_PORTA_IRQHandler(void)
{
	if (!IsPowerOn)
	{
		EXTI_INT_Disable(GPIOA, GPIO_Pin_2);
		EXTI_INT_Disable(GPIOB, GPIO_Pin_1);
		MainTask |= CHARGING_POWEROFF_TASK;
	}
	return;
}

/********************************************************************
Function Name:	
Description:	External Interrupt PORTB Interruption routine.
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
@near @interrupt void EXTI_PORTB_IRQHandler(void)
{
	if (!IsPowerOn)
	{
		EXTI_INT_Disable(GPIOA, GPIO_Pin_2);
		EXTI_INT_Disable(GPIOB, GPIO_Pin_1);
		MainTask |= SYSTEM_POWERON_TASK;
	}
	return;
}

/********************************************************************
Function Name:	
Description:	External Interrupt PORTC Interruption routine.
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
@near @interrupt void EXTI_PORTC_IRQHandler(void)
{
	return;
}

/********************************************************************
Function Name:	
Description:	External Interrupt PORTD Interruption routine.
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
@near @interrupt void EXTI_PORTD_IRQHandler(void)
{
	return;
}

/********************************************************************
Function Name:	
Description:	External Interrupt PORTE Interruption routine.
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
@near @interrupt void EXTI_PORTE_IRQHandler(void)
{
	return;
}

/********************************************************************
Function Name:	EXTI_INT_Disable
Description:	
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
void EXTI_INT_Disable(GPIO_TypeDef* GPIOx, u8 GPIO_Pin)
{
	GPIOx->CR2.byte &= (u8)(~(GPIO_Pin));
}

/********************************************************************
Function Name:	EXTI_INT_Enable
Description:	
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
void EXTI_INT_Enable(GPIO_TypeDef* GPIOx, u8 GPIO_Pin)
{
	GPIOx->CR2.byte |= GPIO_Pin;
}

/********************************************************************
Function Name:	
Description:	Deinitializes the external interrupt control registers to their default reset value.
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
void EXTI_DeInit(void)
{
	EXTI->CR1 = EXTI_CR1_RESET_VALUE;
	EXTI->CR2 = EXTI_CR2_RESET_VALUE;
}

/********************************************************************
Function Name:	
Description:	Set the external interrupt sensitivity of the selected port.
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
void EXTI_SetExtIntSensitivity(EXTI_Port_TypeDef Port, EXTI_Sensitivity_TypeDef SensitivityValue)
{
	switch(Port)	// Set external interrupt sensitivity
	{
		case EXTI_PORT_GPIOA:
			EXTI->CR1 &= (u8)(~EXTI_CR1_PAIS);
			EXTI->CR1 |= (u8)(SensitivityValue);
			break;
		case EXTI_PORT_GPIOB:
			EXTI->CR1 &= (u8)(~EXTI_CR1_PBIS);
			EXTI->CR1 |= (u8)((u8)(SensitivityValue) << 2);
			break;
		case EXTI_PORT_GPIOC:
			EXTI->CR1 &= (u8)(~EXTI_CR1_PCIS);
			EXTI->CR1 |= (u8)((u8)(SensitivityValue) << 4);
			break;
		case EXTI_PORT_GPIOD:
			EXTI->CR1 &= (u8)(~EXTI_CR1_PDIS);
			EXTI->CR1 |= (u8)((u8)(SensitivityValue) << 6);
			break;
		case EXTI_PORT_GPIOE:
			EXTI->CR2 &= (u8)(~EXTI_CR2_PEIS);
			EXTI->CR2 |= (u8)(SensitivityValue);
			break;
		default:
			break;
	}
}

/********************************************************************
Function Name:	
Description:	Set the TLI interrupt sensitivity.
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
void EXTI_SetTLISensitivity(EXTI_TLISensitivity_TypeDef SensitivityValue)
{
	EXTI->CR2 &= (u8)(~EXTI_CR2_TLIS);	// Set TLI interrupt sensitivity
	EXTI->CR2 |= (u8)(SensitivityValue);
}
