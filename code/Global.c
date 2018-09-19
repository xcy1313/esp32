#define _GLOBAL_C_
#include "include.h"

bool IsPowerOn;
u16 u16PowerHoldTime;			//susan+ 121112

void MenuHandler(void);
void Golobal_Reset(void);
void ParameterInit(void);

/********************************************************************
Function Name:	
Description:	
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
void Global_AllKey_Prcoess(void)
{
	u8 u8keycode;
	u8keycode = Key_GetCode();
	MainTask &= ~KEY_HANDLE_TASK;
	if (u8keycode == KEY_NONE)
		return;
	if ((u8keycode != KEY_POWER_LONG)&&(IsPowerOn))
	{
		u16PowerHoldTime = 0;
	}
	MenuHandler();
	AudioHandler();
	switch(SourceSelect)
	{
		case BLUETOOTH_AUDIO:
			BluetoothHandle();
			break;
		case LINE_IN_AUDIO:

			break;
	}
}

/********************************************************************
Function Name:	
Description:	
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
void MenuHandler(void)
{
	u8 u8keycode;
	u16 i;
	u8keycode = Key_GetCode();
	switch(u8keycode)
	{

		case KEY_POWER:
			if (IsPowerOn)
				MainTask |= SYSTEM_POWEROFF_TASK;
			else
				EXTI_INT_Enable(GPIOB, GPIO_Pin_1);
			break;
		case KEY_POWER_LONG:
			if (!IsPowerOn)
				break;
			u16PowerHoldTime++;
			if (u16PowerHoldTime >450)
			{
				u16PowerHoldTime = 0;
				Golobal_Reset();	
			}
			break;
		case KEY_BT:
			if (!IsPowerOn)
				break;
			SourceSelect = BLUETOOTH_AUDIO;
			Audio_SetAudioSource(SourceSelect);
			Clock[BT_ANSWER_SHORT_PULSE_CLK] = BT_ANSWER_PULSE_VERYLONG;
			BT_ANSWER_BUTTON_H;
			LINE_IN_LED_OFF;
			break;
		case KEY_LINE_IN:
			if ((!IsPowerOn)||(BT_PhoneTimes>5))
				break;
			SourceSelect = LINE_IN_AUDIO;
			Audio_SetAudioSource(SourceSelect);
			LINE_IN_LED_ON;
			Clock[BT_ANSWER_SHORT_PULSE_CLK] = BT_ANSWER_PULSE_VERYLONG;
			BT_ANSWER_BUTTON_H;
			BT_LED_OFF;
		
			break;
	}
//s-	FLASH_ProgramByte(SOURCE_SELECT_ADDR, SourceSelect);
	DelayNms(20);	

}

void PowerOFF_Handle(void)
{
	MainTask &= ~SYSTEM_POWEROFF_TASK;
	IsPowerOn = FALSE;
	AllLed_OFF();
	BT_POWER_L;	
	if (SourceSelect == BLUETOOTH_AUDIO)
		DelayNms(1100);
	IDT_MuteOn();
	if (!bCharging)
	{
		ADC1_DeInit();
		EXTI_INT_Enable(GPIOB, GPIO_Pin_1);
		EXTI_INT_Enable(GPIOA, GPIO_Pin_2);
		TIM4_Cmd(DISABLE);	
		TIM2_Cmd(DISABLE);	
	}
	Write_P_IIC_SDA_0;
	Write_P_IIC_SCL_0;
	AMP_ON_L;
	if (!bCharging)
		HALT;
}


void PowerON_Handle(void)
{
	MainTask &= ~SYSTEM_POWERON_TASK;
	IsPowerOn = TRUE;
	MCU_ADC_Init();
	TIM4_Cmd(ENABLE);	
	TIM2_Cmd(ENABLE);	
	if (!bCharging)
		POWER_LED_ON;
	if (SourceSelect == LINE_IN_AUDIO)
		LINE_IN_LED_ON;
	AMP_ON_H;
	IDT_Init();
	BT_POWER_H;										
	Clock[BT_POWER_PULSE_CLK] = 1000;
}

void Charging_PowerOFF_Handle(void)
{
	MainTask &= ~CHARGING_POWEROFF_TASK;
	MCU_ADC_Init();
	TIM4_Cmd(ENABLE);	
}
/********************************************************************
Function Name:	
Description:	
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
void AllLed_OFF(void)
{
	POWER_LED_OFF;
	LINE_IN_LED_OFF;
	BT_LED_OFF;

}


/********************************************************************
Function Name:	Global_GetADIOValueTask
Description:	获取有效通道的各AD值
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
void Global_GetADIOValueTask(void)
{
	MainTask &= ~GET_AD_IO_VALUE_TASK;
	Clock[GET_AD_IO_VALUE_CLK] = GET_AD_IO_VALUE_FREQ;
	Audio_AudioSleepCheck();	//susan 121024
	BT_GetBluetoothSignal();
}
/********************************************************************
Function Name:	
Description:	
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
void Golobal_Reset(void)
{
	WWDG->CR |= WWDG_CR_WDGA;
}


/********************************************************************
Function Name:	SystemInitial
Description:	
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
void SystemInitial(void)
{
	MCU_CLK_Init();
	GPIO_Configuration();
	MCU_Timer_Init();
//s-	MCU_ADC_Init();
	ParameterInit();
	MCU_EXTI_Init();
	RIM;
}

/********************************************************************
Function Name:	ParameterInit
Description:	全局变量初始化
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
void ParameterInit(void)
{
	Clock[GET_AD_IO_VALUE_CLK] = GET_AD_IO_VALUE_FREQ;
	Clock[KEY_SCAN_CLK] = KEY_SCAN_FREQ;
	Clock[SYSTEM_SLEEP_CLK] = 0;		

	u8ChargeTimes = 100;
	bLowBattery = FALSE;
	u8Volume = DEFLAUT_VOLUME;	
	SourceSelect = BLUETOOTH_AUDIO;	
	MainTask |= SYSTEM_POWEROFF_TASK;
}

/********************************************************************
Function Name:	DelayNms
Description:	根据参数 time 确定要延时多少 ms
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
void DelayNms(u16 time)
{
	u16 i,j;
	for(i=0;i<time;i++)
	{
		for(j=0; j<264; j++)
		{
		}
	}
}



