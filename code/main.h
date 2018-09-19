#ifndef _MAIN_H_
	#define _MAIN_H_

#ifdef _MAIN_C_
	#define OS_EXT
#else
	#define OS_EXT extern
#endif

OS_EXT volatile u16 MainTask;	


#define IDLE_TASK				0x0000	
#define KEY_SCAN_TASK			0x0001	
#define SYSTEM_POWEROFF_TASK		0x0002	
#define SYSTEM_POWERON_TASK		0x0004	

#define GET_AD_IO_VALUE_TASK	0x0008	
#define LED_BLINK_TASK			0x0010	
#define KEY_HANDLE_TASK			0x0020	
#define CHARGING_POWEROFF_TASK			0x0040


#define KEY_SCAN_FREQ				10		
#define GET_AD_IO_VALUE_FREQ		4		
#define SYSTEM_SLEEP_FREQ			20		


#define BT_PAIRING_PULSE			100		
#define BT_ANSWER_PULSE_VERYLONG		1100		
#define BT_ANSWER_PULSE_MIDDLE		510
#define BT_ANSWER_PULSE_LONG		750
#define BT_PLAY_PULSE				500		


	#undef OS_EXT
#endif

