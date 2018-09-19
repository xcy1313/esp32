#define _STM8S105_ADC_C_
#include "include.h"

/********************************************************************
Function Name:	
Description:	ADC 引腳配置
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
void MCU_ADC_Init(void)
{
//	CLK->PCKENR2 |= 0x08;			// Enable the clock for ADC
	ADC1->CR2 = ADC1_ALIGN_LEFT;	// Configure the data alignment
}

/********************************************************************
Function Name:	
Description:	获取 AD 通道值 至 输入参数里
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
u16 ADC_GetADValue(u8 u8AdCh)
{
	u8 i,j;
	u16 temp;
	
	ADC1->CSR = u8AdCh;	
	ADC1->CR1 |= 0x01;
	for(j=0;(j<250)&&((ADC1->CSR&0x80)==0); j++)	
	{
		NOP;
	}
	
	temp = (ADC1->DRH);
	temp = (temp<<2);
	temp = (temp | (ADC1->DRL&0x03));

	return temp;
}

/********************************************************************
Function Name:	ADC1_IRQHandler
Description:	
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
@near @interrupt void ADC1_IRQHandler(void)
{
	return;
}

/********************************************************************
Function Name:	
Description:	Configure the ADC1 conversion on selected channel.
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
void ADC1_DeInit(void)
{
	ADC1->CSR   = ADC1_CSR_RESET_VALUE;
	ADC1->CR1   = ADC1_CR1_RESET_VALUE;
	ADC1->CR2   = ADC1_CR2_RESET_VALUE;
	ADC1->CR3   = ADC1_CR3_RESET_VALUE;
	ADC1->TDRH  = ADC1_TDRH_RESET_VALUE;
	ADC1->TDRL  = ADC1_TDRL_RESET_VALUE;
	ADC1->HTRH  = ADC1_HTRH_RESET_VALUE;
	ADC1->HTRL  = ADC1_HTRL_RESET_VALUE;
	ADC1->LTRH  = ADC1_LTRH_RESET_VALUE;
	ADC1->LTRL  = ADC1_LTRL_RESET_VALUE;
	ADC1->AWCRH = ADC1_AWCRH_RESET_VALUE;
	ADC1->AWCRL = ADC1_AWCRL_RESET_VALUE;
}

/*******************************************************************************
*函数介绍: Configure the ADC1 conversion on selected channel.
*输入参数: None
*输出参数: None
*返 回 值: None
*******************************************************************************/
void ADC1_ConversionConfig(ADC1_ConvMode_TypeDef ADC1_ConversionMode, ADC1_Channel_TypeDef ADC1_Channel, ADC1_Align_TypeDef ADC1_Align)
{
	ADC1->CR2 &= (u8)(~ADC1_CR2_ALIGN);		// Clear the align bit
	ADC1->CR2 |= (u8)(ADC1_Align);			// Configure the data alignment

	if (ADC1_ConversionMode == ADC1_CONVERSIONMODE_CONTINUOUS)
	{
	
		ADC1->CR1 |= ADC1_CR1_CONT;			// Set the continuous coversion mode
	}
	else
	{
		ADC1->CR1 &= (u8)(~ADC1_CR1_CONT);	// Set the single conversion mode
	}

	ADC1->CSR &= (u8)(~ADC1_CSR_CH);		// Clear the ADC1 channels
	ADC1->CSR |= (u8)(ADC1_Channel);		// Select the ADC1 channel
}

/*******************************************************************************
*函数介绍: Configure the ADC1 prescaler division factor.
*输入参数: None
*输出参数: None
*返 回 值: None
*******************************************************************************/
void ADC1_PrescalerConfig(ADC1_PresSel_TypeDef ADC1_Prescaler)
{
	ADC1->CR1 &= (u8)(~ADC1_CR1_SPSEL);	// Clear the SPSEL bits
	ADC1->CR1 |= (u8)(ADC1_Prescaler);	// Select the prescaler division factor according to ADC1_PrescalerSelection values
}

/*******************************************************************************
*函数介绍: Configure the ADC1 conversion on external trigger event.
*输入参数: None
*输出参数: None
*返 回 值: None
*******************************************************************************/
void ADC1_ExternalTriggerConfig(ADC1_ExtTrig_TypeDef ADC1_ExtTrigger, FunctionalState NewState)
{
	ADC1->CR2 &= (u8)(~ADC1_CR2_EXTSEL);		// Clear the external trigger selection bits

	if (NewState != DISABLE)
	{
		ADC1->CR2 |= (u8)(ADC1_CR2_EXTTRIG);	// Enable the selected external Trigger
	}
	else
	{
		ADC1->CR2 &= (u8)(~ADC1_CR2_EXTTRIG);	// Disable the selected external trigger
	}

	ADC1->CR2 |= (u8)(ADC1_ExtTrigger);			// Set the selected external trigger
}

/*******************************************************************************
*函数介绍: Enables or disables the ADC1 Schmitt Trigger on a selected channel.
*输入参数: None
*输出参数: None
*返 回 值: None
*******************************************************************************/
void ADC1_SchmittTriggerConfig(ADC1_SchmittTrigg_TypeDef ADC1_SchmittTriggerChannel, FunctionalState NewState)
{
	if (ADC1_SchmittTriggerChannel == ADC1_SCHMITTTRIG_ALL)
	{
		if (NewState != DISABLE)
		{
			ADC1->TDRL &= (u8)0x0;
			ADC1->TDRH &= (u8)0x0;
		}
		else	// NewState == DISABLE
		{
			ADC1->TDRL |= (u8)0xFF;
			ADC1->TDRH |= (u8)0xFF;
		}
	}
	else if (ADC1_SchmittTriggerChannel < ADC1_SCHMITTTRIG_CHANNEL8)
	{
		if (NewState != DISABLE)
		{
			ADC1->TDRL &= (u8)(~(u8)((u8)0x01 << (u8)ADC1_SchmittTriggerChannel));
		}
		else	// NewState == DISABLE
		{
			ADC1->TDRL |= (u8)((u8)0x01 << (u8)ADC1_SchmittTriggerChannel);
		}
	}
	else	// ADC1_SchmittTriggerChannel >= ADC1_SCHMITTTRIG_CHANNEL8
	{
		if (NewState != DISABLE)
		{
			ADC1->TDRH &= (u8)(~(u8)((u8)0x01 << ((u8)ADC1_SchmittTriggerChannel - (u8)8)));
		}
		else	// NewState == DISABLE
		{
			ADC1->TDRH |= (u8)((u8)0x01 << ((u8)ADC1_SchmittTriggerChannel - (u8)8));
		}
	}
}

/*******************************************************************************
*函数介绍: Enables or Disables the ADC1 data store into the Data Buffer registers rather than in the Data Register
*输入参数: None
*输出参数: None
*返 回 值: None
*******************************************************************************/
void ADC1_DataBufferCmd(FunctionalState NewState)
{
	if (NewState != DISABLE)
	{
		ADC1->CR3 |= ADC1_CR3_DBUF;
	}
	else	// NewState == DISABLE
	{
		ADC1->CR3 &= (u8)(~ADC1_CR3_DBUF);
	}
}

/*******************************************************************************
*函数介绍: Enables or Disables the ADC1 peripheral.
*输入参数: None
*输出参数: None
*返 回 值: None
*******************************************************************************/
void ADC1_Cmd(FunctionalState NewState)
{
	if (NewState != DISABLE)
	{
		ADC1->CR1 |= ADC1_CR1_ADON;
	}
	else	// NewState == DISABLE
	{
		ADC1->CR1 &= (u8)(~ADC1_CR1_ADON);
	}
}

/*******************************************************************************
*函数介绍: Enables or Disables the ADC1 scan mode.
*输入参数: None
*输出参数: None
*返 回 值: None
*******************************************************************************/
void ADC1_ScanModeCmd(FunctionalState NewState)
{
	if (NewState != DISABLE)
	{
		ADC1->CR2 |= ADC1_CR2_SCAN;
	}
	else	// NewState == DISABLE
	{
		ADC1->CR2 &= (u8)(~ADC1_CR2_SCAN);
	}
}

/*******************************************************************************
*函数介绍: Get one sample of measured signal.
*输入参数: None
*输出参数: None
*返 回 值: None
*******************************************************************************/
u16 ADC1_GetBufferValue(u8 Buffer)
{
	u16 temph = 0;
	u8 templ = 0;
	
	if((ADC1->CR2 & ADC1_CR2_ALIGN) != 0)	// Right alignment
	{
		templ = *(u8*)(u16)((u16)ADC1_BaseAddress + (u8)(Buffer << 1) + 1);	// Read LSB first
		temph = *(u8*)(u16)((u16)ADC1_BaseAddress + (u8)(Buffer << 1));		// Then read MSB
		temph = (u16)(templ | (u16)(temph << (u8)8));
	}
	else	// Left alignment
	{
		temph = *(u8*)(u16)((u16)ADC1_BaseAddress + (u8)(Buffer << 1));		// Read MSB firts
		templ = *(u8*)(u16)((u16)ADC1_BaseAddress + (u8)(Buffer << 1) + 1);	// Then read LSB
		temph = (u16)((u16)((u16)templ << 6) | (u16)(temph << 8));
	}

	return ((u16)temph);
}

/*******************************************************************************
*函数介绍: Enables or disables the ADC1 interrupt.
*输入参数: None
*输出参数: None
*返 回 值: None
*******************************************************************************/
void ADC1_ITConfig(ADC1_IT_TypeDef ADC1_IT, FunctionalState NewState)
{
	if (NewState != DISABLE)
	{
		ADC1->CSR |= (u8)ADC1_IT;				// Enable the ADC1 interrupts
	}
	else	// NewState == DISABLE
	{
		ADC1->CSR &= (u8)((u16)~(u16)ADC1_IT);	// Disable the ADC1 interrupts
	}
}

/*******************************************************************************
*函数介绍: Clear the ADC1 End of Conversion pending bit.
*输入参数: None
*输出参数: None
*返 回 值: None
*******************************************************************************/
void ADC1_ClearITPendingBit(ADC1_IT_TypeDef ITPendingBit)
{
	u8 temp = 0;

	if (((u16)ITPendingBit & 0xF0) == 0x10)
	{
		temp = (u8)((u16)ITPendingBit & 0x0F);	// Clear analog watchdog channel status
		if (temp < 8)
		{
			ADC1->AWSRL &= (u8)~(u8)((u8)1 << temp);
		}
		else
		{
			ADC1->AWSRH &= (u8)~(u8)((u8)1 << (temp - 8));
		}
	}
	else
	{
		ADC1->CSR &= (u8)((u16)~(u16)ITPendingBit);  // Clear EOC | AWD flag status
	}
}

