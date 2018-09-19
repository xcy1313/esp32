#define _STM8S105_TIMER_C_
#include "include.h"

bool bBeepEnabled;
u16 u16BeepTimer;

/********************************************************************
Function Name:	MCU_Timer_Init
Description:	
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
void MCU_Timer_Init(void)
{

	TIM2_TimeBaseInit(TIM2_PRESCALER_16, 200);
	TIM2->IER |= (u8)(TIM2_IER_UIE);
	TIM2_Cmd(ENABLE);


	TIM4_TimeBaseInit(TIM4_PRESCALER_16, 250);	// Time base configuration
	TIM4_ClearFlag(TIM4_FLAG_UPDATE);			// Clear TIM4 update flag
	TIM4_ITConfig(TIM4_IT_UPDATE, ENABLE);		// Enable update interrupt
	TIM4_Cmd(ENABLE);							// Enable TIM4 (作為系統時期 1ms 中斷)
}

/********************************************************************
Function Name:	
Description:	TIM2 更新中斷
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
@near @interrupt void TIM2_UPD_OVF_BRK_IRQHandler(void)
{
	TIM2->SR1 = (u8)(~TIM2_SR1_UIF);
	if (bBeepEnabled)
	{
		PODR_BEEP_PIN1 = !PODR_BEEP_PIN1;
		PODR_BEEP_PIN2 = !PODR_BEEP_PIN2;
		u16BeepTimer ++;
		if (u16BeepTimer == 200)
		{
			bBeepEnabled = FALSE;
			u16BeepTimer = 0;
		//s-	if (SourceSelect == BLUETOOTH_AUDIO)
		//s-		IDT_InputChannel(INPUT1_CHANNEL);
		}
	}
	
	return;
}

/********************************************************************
Function Name:	
Description:	TIM2 捕獲比較中斷
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
@near @interrupt void TIM2_CAP_COM_IRQHandler(void)
{
	return;
}
/********************************************************************
Function Name:	Beeper_SetPlay			
Description:		Set beeper on or off
Create Date:		
Created by: 	
Modify Date:		
Modified by:		
Input:			
Output: 		
Return: 		
********************************************************************/
void Beeper_SetPlay(bool bPlay)
{
	bBeepEnabled = bPlay;
}


/********************************************************************
Function Name:	
Description:	TIM4 更新中斷
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
@near @interrupt void TIM4_UPD_OVF_IRQHandler(void)
{
//	TIM4_ClearITPendingBit(TIM4_IT_UPDATE);			// 清中斷
	TIM4->SR1 = (u8)(~TIM4_IT_UPDATE);				// 中断中尽量少占用时间
	
	SysTickMSec++;									// 毫秒
	if(SysTickMSec >= 1000)
	{   
		SysTickMSec = 0;
		
		SysTickSecond++;							// 秒
		if(SysTickSecond >= 60)
		{
			SysTickSecond = 0;
			
			SysTickMinute++;						// 分

			if((Clock[SYSTEM_SLEEP_CLK] < 20)&&(IsPowerOn))	// min级 系统睡眠
			{
				Clock[SYSTEM_SLEEP_CLK]++;
			}
		}
		
		if(Clock[SOURCE_SWITCH_PAUSE_CLK] != 0)		// S级  发送暂停的频率
		{
			Clock[SOURCE_SWITCH_PAUSE_CLK]--;
		}
	}
	if (SysTickMSec % 80 == 0)
	{
		Key_Timer100ms();
	}
	if(Clock[GET_AD_IO_VALUE_CLK] != 0)				// ms级  获取AD值
	{
		Clock[GET_AD_IO_VALUE_CLK]--;
		if(Clock[GET_AD_IO_VALUE_CLK] == 0)
		{
			MainTask |= GET_AD_IO_VALUE_TASK;
		}
	}

	if(Clock[KEY_SCAN_CLK] != 0)					// ms级  按键扫描
	{
		Clock[KEY_SCAN_CLK]--;
		if(Clock[KEY_SCAN_CLK] == 0)
		{
			MainTask |= KEY_SCAN_TASK;
		}
	}
	
	if(Clock[BT_LED_BLINK_CLK] != 0)				// ms级  蓝牙LED 闪烁
	{
		Clock[BT_LED_BLINK_CLK]--;
		if(Clock[BT_LED_BLINK_CLK] == 0)
		{
			MainTask |= LED_BLINK_TASK;
		}
	}
	
	if(Clock[BT_PAIRING_PULSE_CLK] != 0)			// ms级  BT 配对脉冲
	{
		Clock[BT_PAIRING_PULSE_CLK]--;
		
	}
	
	if(Clock[BT_ANSWER_DOUBLE_PULSE_CLK] > 1)				// ms级  BT Answer 脉冲
	{
		Clock[BT_ANSWER_DOUBLE_PULSE_CLK]--;
		if (Clock[BT_ANSWER_DOUBLE_PULSE_CLK] >= 500)
		{
			BT_ANSWER_BUTTON_H;
		}
		else if (Clock[BT_ANSWER_DOUBLE_PULSE_CLK] >= 250)
		{
			BT_ANSWER_BUTTON_L;
		}
		else if (Clock[BT_ANSWER_DOUBLE_PULSE_CLK] > 0)
		{
			BT_ANSWER_BUTTON_H;
		}
	}
	if (Clock[BT_ANSWER_SHORT_PULSE_CLK] >1)
		Clock[BT_ANSWER_SHORT_PULSE_CLK]--;
	
	if(Clock[CHARGE_LED_BLINK_CLK] >0)				// ms级
	{
		Clock[CHARGE_LED_BLINK_CLK]--;
	}
	
	if(Clock[BT_POWER_PULSE_CLK] != 0)				// ms级  BT Power 脉冲
	{
		Clock[BT_POWER_PULSE_CLK]--;
		
	}
	if (Clock[BT_ANSWER_HANDLE] > 1)
		Clock[BT_ANSWER_HANDLE]--;
		
	if(Clock[BT_VOLMAX_CLK] != 0)				// ms级  BT Power 脉冲
	{
		Clock[BT_VOLMAX_CLK]--;
		
	}
	if (Clock[BT_LOWBATTERY_CLK] > 1)
	{
		Clock[BT_LOWBATTERY_CLK] --;
	}
	return;
}

/*******************************************************************************
*函数介绍: Initializes the TIM2 Time Base Unit according to the specified parameters.
*输入参数: TIM2_Prescaler specifies the Prescaler from TIM2_Prescaler_TypeDef.
*输入参数: TIM2_Period specifies the Period value.
*输出参数: 无
*返 回 值: 无
*******************************************************************************/
void TIM2_TimeBaseInit(TIM2_Prescaler_TypeDef TIM2_Prescaler, u16 TIM2_Period)
{
	TIM2->PSCR = (u8)(TIM2_Prescaler);		// Set the Prescaler value
	TIM2->ARRH = (u8)(TIM2_Period >> 8);	// Set the Autoreload value
	TIM2->ARRL = (u8)(TIM2_Period);
}

/*******************************************************************************
*函数介绍: Configures the TIM2 Prescaler.
*输入参数: Prescaler specifies the Prescaler Register value
*输入参数: TIM2_PSCReloadMode specifies the TIM2 Prescaler Reload mode.
*输出参数: 无
*返 回 值: 无
*******************************************************************************/
void TIM2_PrescalerConfig(TIM2_Prescaler_TypeDef Prescaler, TIM2_PSCReloadMode_TypeDef TIM2_PSCReloadMode)
{
	TIM2->PSCR = (u8)Prescaler;				// Set the Prescaler value
	TIM2->EGR  = (u8)TIM2_PSCReloadMode;	// Set or reset the UG Bit
}

/*******************************************************************************
*函数介绍: Initializes the TIM2 Channel1 according to the specified parameters.
*输入参数: TIM2_OCMode specifies the Output Compare mode  from @ref TIM2_OCMode_TypeDef.
*输入参数: TIM2_OutputState specifies the Output State  from @ref TIM2_OutputState_TypeDef.
*输入参数: TIM2_Pulse specifies the Pulse width  value.
*输入参数: TIM2_OCPolarity specifies the Output Compare Polarity  from @ref TIM2_OCPolarity_TypeDef.
*输出参数: 无
*返 回 值: 无
*******************************************************************************/
void TIM2_OC1Init(TIM2_OCMode_TypeDef TIM2_OCMode, TIM2_OutputState_TypeDef TIM2_OutputState, u8 TIM2_Pulse, TIM2_OCPolarity_TypeDef TIM2_OCPolarity)
{
	// Disable the Channel 1: Reset the CCE Bit, Set the Output State , the Output Polarity
	TIM2->CCER1 &= (u8)(~( TIM2_CCER1_CC1E | TIM2_CCER1_CC1P));
	// Set the Output State &  Set the Output Polarity
	TIM2->CCER1 |= (u8)((u8)(TIM2_OutputState & TIM2_CCER1_CC1E ) | (u8)(TIM2_OCPolarity & TIM2_CCER1_CC1P));
	// Reset the Output Compare Bits  & Set the Ouput Compare Mode
	TIM2->CCMR1 = (u8)((u8)(TIM2->CCMR1 & (u8)(~TIM2_CCMR_OCM)) | (u8)TIM2_OCMode);
	// Set the Pulse value
	TIM2->CCR1H = (u8)(TIM2_Pulse >> 8);
	TIM2->CCR1L = (u8)(TIM2_Pulse);
}


/*******************************************************************************
*函数介绍: Enables or disables the TIM2 peripheral Preload Register on CCR1.
*输入参数: NewState new state of the Capture Compare Preload register. This parameter can be ENABLE or DISABLE.
*输出参数: 无
*返 回 值: 无
*******************************************************************************/
void TIM2_OC1PreloadConfig(FunctionalState NewState)
{

	if (NewState != DISABLE)	// Set or Reset the OC1PE Bit
	{
		TIM2->CCMR1 |= (u8)TIM2_CCMR_OCxPE;
	}
	else
	{
		TIM2->CCMR1 &= (u8)(~TIM2_CCMR_OCxPE);
	}
}

/*******************************************************************************
*函数介绍: Enables or disables TIM2 peripheral Preload register on ARR.
*输入参数: NewState new state of the TIM2 peripheral Preload register. This parameter can be ENABLE or DISABLE.
*输出参数: 无
*返 回 值: 无
*******************************************************************************/
void TIM2_ARRPreloadConfig(FunctionalState NewState)
{
	if (NewState != DISABLE)	// Set or Reset the ARPE Bit
	{
		TIM2->CR1 |= (u8)TIM2_CR1_ARPE;
	}
	else
	{
		TIM2->CR1 &= (u8)(~TIM2_CR1_ARPE);
	}
}


/*******************************************************************************
*函数介绍: Enables or disables the TIM2 peripheral.
*输入参数: NewState new state of the TIM2 peripheral. This parameter can be ENABLE or DISABLE.
*输出参数: 无
*返 回 值: 无
*******************************************************************************/
void TIM2_Cmd(FunctionalState NewState)
{
	if (NewState != DISABLE)	// set or Reset the CEN Bit
	{
		TIM2->CR1 |= (u8)TIM2_CR1_CEN;
	}
	else
	{
		TIM2->CR1 &= (u8)(~TIM2_CR1_CEN);
	}
}

/********************************************************************
Function Name:	TIM4_TimeBaseInit
Description:	Initializes the TIM4 Time Base Unit according to the specified parameters.
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
void TIM4_TimeBaseInit(TIM4_Prescaler_TypeDef TIM4_Prescaler, u8 TIM4_Period)
{
	TIM4->PSCR = (u8)(TIM4_Prescaler);	// Set the Prescaler value
	TIM4->ARR = (u8)(TIM4_Period);		// Set the Autoreload value
}

/********************************************************************
Function Name:	TIM4_ClearFlag
Description:	Clears the TIM4 pending flags.
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
void TIM4_ClearFlag(TIM4_FLAG_TypeDef TIM4_FLAG)
{
	TIM4->SR1 = (u8)(~TIM4_FLAG);
}

/********************************************************************
Function Name:	TIM4_ITConfig
Description:	Enables or disables the specified TIM4 interrupts.
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
void TIM4_ITConfig(TIM4_IT_TypeDef TIM4_IT, FunctionalState NewState)
{
	if (NewState != DISABLE)
	{
		TIM4->IER |= (u8)TIM4_IT;		// Enable the Interrupt sources
	}
	else
	{
		TIM4->IER &= (u8)(~TIM4_IT);	// Disable the Interrupt sources
	}
}

/********************************************************************
Function Name:	TIM4_Cmd
Description:	
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
void TIM4_Cmd(FunctionalState NewState)
{
	if (NewState != DISABLE)	// set or Reset the CEN Bit
	{
		TIM4->CR1 |= TIM4_CR1_CEN;
	}
	else
	{
		TIM4->CR1 &= (u8)(~TIM4_CR1_CEN);
	}
}

/********************************************************************
Function Name:	TIM4_ClearITPendingBit
Description:	
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
void TIM4_ClearITPendingBit(TIM4_IT_TypeDef TIM4_IT)
{
	TIM4->SR1 = (u8)(~TIM4_IT);	// Clear the IT pending Bit
}



