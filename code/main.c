#define _MAIN_C_
#include "include.h"

/********************************************************************
Function Name:	MW550
Description:	mapinfo $(OutputPath)$(TargetSName).map 用来显示占用的FLASH
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
void main(void)
{
	SystemInitial();
	
	while(1)
	{
		if (MainTask & SYSTEM_POWERON_TASK)
		{
			PowerON_Handle();
		}
		if(MainTask & GET_AD_IO_VALUE_TASK)	
		{
			Global_GetADIOValueTask();
			ChargeSingalHandle();
		}
		if(MainTask & KEY_SCAN_TASK)	
		{
			Key_Scan();
			BT_BTSingalHandle();
			
		}
		if(MainTask &KEY_HANDLE_TASK)
		{
			Global_AllKey_Prcoess();
		}
		if(MainTask & LED_BLINK_TASK)			
		{
			Key_LedBlinkTask();
		}
		if(MainTask & SYSTEM_POWEROFF_TASK)		// 系统睡眠
		{
			PowerOFF_Handle();
		}
		if (MainTask & CHARGING_POWEROFF_TASK)
		{
			Charging_PowerOFF_Handle();
		}
	
	}
}

