#define _BLUETOOTH_C_
#include "include.h"

#define BATTERY_CHECK_ADLine		2
#define BATTERY_NTC_CHECK_ADLine	3
#define LOW_BATTERY_VALUE			830			// 2.4v
#define LOW_BATTERY_POWEROFF 		682			//  2.2V
#define HIGH_NTC_VALUE				550		//  1.8V

u8 KeyTimes =0;
u8   BTLedMode;				// BT LED灯的模式
bool BTLedOn;				// 蓝牙 LED 现在亮还是灭
bool bRedLedOn;
bool bCharging;
bool bLowBattery;
u16 BatteryNTC_Value;
u16 Battery_Value;
u8 u8NoChargeTime;		//susan+ 121211

 void Key_SetBTLedMode(u8 mode);
u16 GetBatteryADValue(u8 adline);
void  BTLedBlink(void);
void ChargeLedBlink(u8 mode);
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
void BluetoothHandle(void)
{

	u8 u8keycode;
	if (!IsPowerOn)
		return;
	u8keycode = Key_GetCode();
	switch(u8keycode)
	{
		case KEY_BT_LONG:
			Clock[BT_PAIRING_PULSE_CLK] = BT_PAIRING_PULSE;	// 置脉冲宽度
			BT_PAIR_BUTTON_H;
			break;
		case KEY_ANSWER:
			KeyTimes++;
			if (KeyTimes ==1)
				Clock[BT_ANSWER_HANDLE] = 700;
			break;
		case KEY_ANSER_LONG:
			Clock[BT_ANSWER_SHORT_PULSE_CLK] = BT_ANSWER_PULSE_MIDDLE;
			BT_ANSWER_BUTTON_H;
			break;
	}		
}


/********************************************************************
Function Name:	
Description:	获取蓝牙信号
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
void BT_GetBluetoothSignal(void)
{
	if(BT_PAIRING_READ)						// 读连接中 状态
	{
		if(BT_PairingTimes<10)				// 消抖
		{
			BT_PairingTimes++;
		}
	}
	else
	{
		if(BT_PairingTimes)
		{
			BT_PairingTimes--;
		}
	}
	
	if(BT_PAIRED_READ)						// 读已配对 状态
	{
		if(BT_PairedTimes<10)				// 消抖
		{
			BT_PairedTimes++;
		}
	}
	else
	{
		if(BT_PairedTimes)
		{
			BT_PairedTimes--;
		}
	}
	if(BT_PHONE_READ)						// 读来电话 状态
	{
		if(BT_PhoneTimes<10)				// 消抖
		{
			BT_PhoneTimes++;
		}
	}
	else
	{
		if(BT_PhoneTimes)
		{
			BT_PhoneTimes--;
		}
	}
	if(READ_CHARGE_STATU)						
	{
		u8NoChargeTime = 0;
		if(u8ChargeTimes < 200)				
		{
			u8ChargeTimes++;
		}
	}
	else
	{
		if (u8NoChargeTime < 100)
			u8NoChargeTime++;
		u8ChargeTimes = 0;
	}
}



/********************************************************************
Function Name:	
Description:	蓝牙信号处理
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
void BT_BTSingalHandle(void)
{
	u8 tMode,tLedMode;
	u8 m1=0, m2=0;

	if(Clock[BT_PAIRING_PULSE_CLK]==0)	// 时间到
	{
		BT_PAIR_BUTTON_L;
	}
	if((Clock[BT_ANSWER_SHORT_PULSE_CLK]==1)||(Clock[BT_ANSWER_DOUBLE_PULSE_CLK]==1))	// 时间到
	{
		Clock[BT_ANSWER_SHORT_PULSE_CLK] = 0;
		Clock[BT_ANSWER_DOUBLE_PULSE_CLK] = 0;
		BT_ANSWER_BUTTON_L;
	}
	if((Clock[BT_POWER_PULSE_CLK]==0)&&(!IsPowerOn))			// 模式 = Power
	{
	//s-	BT_POWER_L;
	}
	 if ((Clock[BT_ANSWER_HANDLE] >200)&&(KeyTimes==2))
	{
		Clock[BT_ANSWER_DOUBLE_PULSE_CLK] = BT_ANSWER_PULSE_LONG;
		BT_ANSWER_BUTTON_H;
		KeyTimes = 0;
		Clock[BT_ANSWER_HANDLE] = 0; 
	}
	else 
	if (Clock[BT_ANSWER_HANDLE]==1)
	{
     		Clock[BT_ANSWER_HANDLE]=0;
		Clock[BT_ANSWER_SHORT_PULSE_CLK] = BT_ANSWER_PULSE_MIDDLE;
		BT_ANSWER_BUTTON_H;
		KeyTimes = 0;
	}

	if (Clock[BT_VOLMAX_CLK] == 0)
		BT_VOLMAX_L;
	if (Clock[BT_LOWBATTERY_CLK] ==1)
	{
		Clock[BT_LOWBATTERY_CLK]  = 0;
		BT_LOWBATTERY_L;
	}
	
	if ((SourceSelect == BLUETOOTH_AUDIO)&&(IsPowerOn))
	{
		if(BT_PairedTimes>5)
		{
			m1 = 0x10;
		}
		if(BT_PairingTimes>5)
		{
			m2 = 0x01;
		}
		tMode = m1 | m2;
		switch(tMode)
		{
			case 0x00:		// 断开
				tLedMode = LED_BLINK_1_1;
				break;
			case 0x01:		// pairing
				tLedMode = LED_BLINK_025_025;
				break;
			case 0x10:		// paired
				tLedMode = LED_SOLID_ON;
				break;
			case 0x11:		// 异常
				tLedMode = LED_BLINK_02_02;
				break;
			default:
				break;
		}
	//	if(BTLedMode != tLedMode)		// 两个模式不一致
			Key_SetBTLedMode(tLedMode);		
	
	}
//	else
	{
		if (IsPowerOn)
		{
			if((BT_PhoneTimes>5)&&(SourceSelect == LINE_IN_AUDIO))
			{
				SourceSelect = BLUETOOTH_AUDIO;
				Audio_SetAudioSource(SourceSelect);
				LINE_IN_LED_OFF;
			}
		}
	}

}

/********************************************************************
Function Name:	Key_LedBlinkTask
Description:	
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
void Key_LedBlinkTask(void)
{
	MainTask &= ~LED_BLINK_TASK;
	
	if(SourceSelect == BLUETOOTH_AUDIO) 			// 蓝牙 闪烁
	{
		BTLedBlink();
	}
	 
	if(SourceSelect == LINE_IN_AUDIO) 		// Line-in 闪烁
	{

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
void Key_SetBTLedMode(u8 mode)
{
	BTLedMode = mode;
	MainTask |= LED_BLINK_TASK;
}

/********************************************************************
Function Name:	BTLedBlink
Description:	
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
void BTLedBlink(void)
{
	u8 index;
	u16 delay;


	if (!IsPowerOn)
		return;
	
	if(BTLedOn == TRUE)					// 读灭的延时
	{
		index = 1;
	}
	else
	{
		index = 0;						// 读亮的延时
	}
	
	switch(BTLedMode)
	{
		case LED_SOLID_ON:
			BT_LED_ON;
			return;	
			break;
		
		case LED_SOLID_OFF:
			BT_LED_OFF;
			return;
			break;

		case LED_BLINK_05_05:
			delay = DELAY_05_05[index];	// 读延时
			break;
			
		case LED_BLINK_1_1:
			delay = DELAY_1_1[index];
			break;

		case LED_BLINK_1_3:
			delay = DELAY_1_3[index];
			break;
			
		case LED_BLINK_02_02:
			delay = DELAY_02_02[index];
			break;
			
		case LED_BLINK_02_3:
			delay = DELAY_02_3[index];
			break;
			
		case LED_BLINK_15_15:
			delay = DELAY_15_15[index];
			break;

		case LED_BLINK_025_025:
			delay = DELAY_025_025[index];
			break;

	}

		//if(delay !=0)
		if (Clock[BT_LED_BLINK_CLK] == 0)
		{
			if(BTLedOn == TRUE)				// 闪烁 LED
			{
				BT_LED_OFF;
				BTLedOn = FALSE;
			}
			else
			{
				BT_LED_ON;
				BTLedOn = TRUE;
			}
			Clock[BT_LED_BLINK_CLK] = delay;	// 置下次 LED 转换时间 (0:表示不闪烁)
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
void ChargeSingalHandle(void)
{ 
//u8 i;
	BatteryNTC_Value = GetBatteryADValue(BATTERY_NTC_CHECK_ADLine);
	Battery_Value = GetBatteryADValue(BATTERY_CHECK_ADLine);
	if ( Battery_Value < LOW_BATTERY_POWEROFF)
	{
		MainTask |= SYSTEM_POWEROFF_TASK;
	}
	else if (( Battery_Value < LOW_BATTERY_VALUE)&&(!bLowBattery))
	{
		bLowBattery = TRUE;
		BT_LOWBATTERY_H;
		Clock[BT_LOWBATTERY_CLK] = 2000;
	}
	if ((u8ChargeTimes >= 190)||(u8ChargeTimes ==0&&u8NoChargeTime >90))
	{
		bCharging = FALSE;
		if (IsPowerOn)
			POWER_LED_ON;
		else
			POWER_LED_OFF;
		ChargeLedBlink(LED_SOLID_OFF);
		if (!IsPowerOn)
		{
			MainTask |= SYSTEM_POWEROFF_TASK;
		}
	}
	else if (u8ChargeTimes > 10)
	{
		bLowBattery = TRUE;
		bCharging = TRUE;
		POWER_LED_OFF;
		if (IsPowerOn)
			ChargeLedBlink(LED_SOLID_ON);
		else
			ChargeLedBlink(LED_BLINK_1_1);
	}
	#if 0
 	if (BatteryNTC_Value > HIGH_NTC_VALUE)
	{
		MainTask |= SYSTEM_POWEROFF_TASK;
	}
	#endif
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
void ChargeLedBlink(u8 mode)
{
	u8 index;
	u16 delay;

	if(bRedLedOn == TRUE)					// 读灭的延时
	{
		index = 1;
	}
	else
	{
		index = 0;						// 读亮的延时
	}
	
	switch(mode)
	{
		case LED_SOLID_ON:
			RED_LED_ON;
			return;
			break;
		case LED_SOLID_OFF:
			RED_LED_OFF;
			return;
			break;	
		case LED_BLINK_1_1:
			delay = DELAY_1_1[index];
			break;
	}

	if(Clock[CHARGE_LED_BLINK_CLK] ==0)
	{
		if(bRedLedOn == TRUE)				// 闪烁 LED
		{
			RED_LED_OFF;
			bRedLedOn = FALSE;
		}
		else
		{
			RED_LED_ON;
			bRedLedOn = TRUE;
		}
		Clock[CHARGE_LED_BLINK_CLK] = delay;	// 置下次 LED 转换时间 (0:表示不闪烁)
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
u16 GetBatteryADValue(u8 adline)
{
	u16 AD_Value;
	u16 u16Temp = 0;
	u16 temp[5];
	u8  i,j;
	u16 max=0;
	u16 min=1000;
	
	for(i=0; i<5; i++)							
	{
		temp[i] = ADC_GetADValue(adline);					
		u16Temp += temp[i];						
		
		if(temp[i] < min)
		{
			min = temp[i]; 
		}
		if(temp[i] > max)
		{
			max = temp[i];
		}
	}	
	AD_Value = ((u16Temp - min - max)/3);		// 减去最大和最小值，在去平均值

	return AD_Value;


}


