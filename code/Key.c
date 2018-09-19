#define _KEY_C_
#include "include.h"

u8 scnCnt[2];
u16 u16KeyScanCount[2];
bool	bKeyActive[2];
u8 u8TempKeyCode[2];
u8 u8KeyRepeatTimer100ms;
 u8   u8KeyCode;		// 按键标识
u16 AD_Value;

#define DITHERTIME		7
#define LONGTIME		100
 const u8 KEY_MAP_TAB[2][5]=
{
	{KEY_POWER,KEY_LINE_IN,KEY_NONE,KEY_NONE,KEY_NONE},
	{KEY_ANSWER,KEY_BT,KEY_VOL_UP,KEY_VOL_DOWN,KEY_NONE}
	
};
 const u16 KEY_AD_TAB[5]=
{
	50,250,430,600,1000
};
u8 ADChannel[ADC_CH_NUM]		// 通道序号
={1, 0}
;
void Key_LongShortProcess(u8 u8ScanType, u8 u8key);
void Key_Scan(void);
u8 Key_GetADKey(u8 u8AdChLine);
u8 Key_GetKeyCode(u8 u8Line, u16 u16Value);

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
u8  Key_GetCode(void)
{
	return u8KeyCode;
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
void Key_Scan(void)
{	
	u8 KeyCode;
	u8 i;
	MainTask &= ~KEY_SCAN_TASK;
	Clock[KEY_SCAN_CLK] = KEY_SCAN_FREQ;
	for (i = 0;i<2;i++)
	{
		KeyCode = Key_GetADKey(i);
		Key_LongShortProcess(i,KeyCode);
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
void Key_LongShortProcess(u8 u8ScanType, u8 u8key)
{

	if (u8key)
	{
		scnCnt[u8ScanType]++;//adjust dither
		if(scnCnt[u8ScanType] == 0xFF)
			scnCnt[u8ScanType]--;
		if (u8key == u8TempKeyCode[u8ScanType])
		{
			if (scnCnt[u8ScanType] > DITHERTIME &&!bKeyActive[u8ScanType])
				bKeyActive[u8ScanType] = TRUE;
			if (bKeyActive[u8ScanType])
			{
				u16KeyScanCount[u8ScanType]++;
				if (u16KeyScanCount[u8ScanType] == 0xffff)
					u16KeyScanCount[u8ScanType] --;
				if (u16KeyScanCount[u8ScanType] >= LONGTIME)
				{
					u8KeyCode = u8TempKeyCode[u8ScanType] | KEY_LP;
					bKeyActive[u8ScanType] = TRUE;
					scnCnt[u8ScanType] = 0;
					MainTask |= KEY_HANDLE_TASK;
				}
			}
		}
		else
		{	
			bMaxVolume = TRUE;
			u8TempKeyCode[u8ScanType] = u8key;
			scnCnt[u8ScanType] = 0;
		}
		
	}
	else
	{
		if (bKeyActive[u8ScanType])
		{
			
			if (u16KeyScanCount[u8ScanType] >= LONGTIME)
			{
				u8KeyCode= KEY_NONE;
				u16KeyScanCount[u8ScanType] = 0;
				bKeyActive[u8ScanType] = FALSE;
				scnCnt[u8ScanType] = 0;
				u8TempKeyCode[u8ScanType] = KEY_NONE;
				return;
			}
			else
			{	
				u8KeyCode = u8TempKeyCode[u8ScanType];
				u16KeyScanCount[u8ScanType] = 0;
				bKeyActive[u8ScanType] = FALSE;
				scnCnt[u8ScanType] = 0;
				u8TempKeyCode[u8ScanType] = KEY_NONE;
				MainTask |= KEY_HANDLE_TASK;
				return;
			}
		}			
		bKeyActive[u8ScanType]  = FALSE;
		scnCnt[u8ScanType] = 0;
		u8TempKeyCode[u8ScanType]  = KEY_NONE;
		return;
	}



}

/********************************************************************
Function Name:	Key_Timer100ms			
Description:		Key timer for repeating step
Create Date:		
Created by:		
Modify Date:		
Modified by:		
Input:			
Output:			
Return:			
********************************************************************/
void Key_Timer100ms(void)
{
	u8KeyRepeatTimer100ms = (u8KeyRepeatTimer100ms + 1) % 3;
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
bool Key_IsRepeatStep(void)
{
	if (u8KeyRepeatTimer100ms == 0)
	{
		u8KeyRepeatTimer100ms = 1;
		return TRUE;
	}
	return FALSE;
}

/********************************************************************
Function Name:	
Description:	获取 按键信息
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
u8 Key_GetADKey(u8 u8AdChLine)
{
	//u16 AD_Value;
	u16 u16Temp = 0;
	u16 temp[5];
	u8  i,j;
	u16 max=0;
	u16 min=1000;
	
	for(i=0; i<5; i++)							// 取5次转化结果
	{
		temp[i] = ADC_GetADValue(ADChannel[u8AdChLine]);					// 取值
		u16Temp += temp[i];						// 值依次累加
		
		if(temp[i] < min)
		{
			min = temp[i]; 
		}
		if(temp[i] > max)
		{
			max = temp[i];
		}
	}	
	AD_Value= ((u16Temp - min - max)/3);		// 减去最大和最小值，在去平均值
	
	if((max-min > 20) || (AD_Value  >= 1000))	// 如果最大和最小值差20以上就直接返回,否则会串键
	{
		return KEY_NONE;
	}
	
	return Key_GetKeyCode(u8AdChLine,AD_Value );
}

/********************************************************************
Function Name:	Key_GetKeyCode			
Description:		Get key position 
Create Date:		
Created by:		
Modify Date:		
Modified by:		
Input:			
Output:			
Return:			
********************************************************************/
u8 Key_GetKeyCode(u8 u8Line, u16 u16Value)
{
	u8 i;
	
	for (i = 0; i < MAX_KEY_PER_LINE; i++)
	{
		if (u16Value <= KEY_AD_TAB[i])
		{
			break;
		}
	}

	return KEY_MAP_TAB[u8Line][i];
}	




