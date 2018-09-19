#define _AUDIO_C_
#include "include.h"


bool bMaxVolume;
u8 u8Volume;
u8 u8PerVolume;
bool bSleepStau;
/********************************************************************
Function Name:	Audio_AudioSourceSelect
Description:	选择 Source
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
void Audio_SetAudioSource(u8 source)
{

	if(source == LINE_IN_AUDIO)
	{
		IDT_InputChannel(INPUT2_CHANNEL);	// Source Line-in Audio
	}
	else
	{
		IDT_InputChannel(INPUT1_CHANNEL);		// Source Bluetooth Audio
	}
}



/********************************************************************
Function Name:	Audio_AudioSleepCheck
Description:	检测声道模拟值
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
void Audio_AudioSleepCheck(void)
{
	if (!IsPowerOn)
		return;
	if(SLEEP_PIDR_READ == 0)
	{
		Clock[SYSTEM_SLEEP_CLK] = 0;
	}
	if (Clock[SYSTEM_SLEEP_CLK] == 20)
	{
		Clock[SYSTEM_SLEEP_CLK] = 0;
		MainTask |= SYSTEM_POWEROFF_TASK;
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
void AudioHandler(void)
{

	u8 u8keycode;
	if (!IsPowerOn)
		return;
	u8keycode = Key_GetCode();
	u8PerVolume = u8Volume;
	switch(u8keycode)
	{
		case KEY_VOL_UP:
		case KEY_VOL_UP_LONG:
			if (((u8keycode == KEY_VOL_UP_LONG)&&Key_IsRepeatStep())
				||(u8keycode == KEY_VOL_UP))
			{
				if (u8Volume < MAX_VOLUME)
					u8Volume++;
				else
					u8Volume = MAX_VOLUME;
			}
		#if 1
			if ((u8Volume == MAX_VOLUME)&&(bMaxVolume))
			{
			//s-	IDT_InputChannel(INPUT2_CHANNEL);
				bMaxVolume = FALSE;
			//s-	Beeper_SetPlay(TRUE);
				
				if (SourceSelect == LINE_IN_AUDIO)
				{
					Beeper_SetPlay(TRUE);
				}
				else
				{
					BT_VOLMAX_H;
					Clock[BT_VOLMAX_CLK] = 1100;
				}
			}
		#endif
			if (u8PerVolume != u8Volume)
				IDT_SetVolume(u8Volume);
			break;
		case KEY_VOL_DOWN:
		case KEY_VOL_DOWN_LOGN:
			if (((u8keycode == KEY_VOL_DOWN_LOGN)&&Key_IsRepeatStep())
				||(u8keycode == KEY_VOL_DOWN))
			{
				if (u8Volume > MIN_VOLUME)
					u8Volume--;
				else
					u8Volume = MIN_VOLUME;
			}
			if (u8PerVolume != u8Volume)
				IDT_SetVolume(u8Volume);
			break;
	}

}




