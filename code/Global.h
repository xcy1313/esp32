#ifndef _GLOBAL_H_
	#define _GLOBAL_H_

#ifdef _GLOBAL_C_
	#define OS_EXT
#else
	#define OS_EXT extern
#endif


OS_EXT bool IsPowerOn;


OS_EXT void Global_GetADIOValueTask(void);
OS_EXT void SystemInitial(void);
OS_EXT void AllLed_OFF(void);
OS_EXT void DelayNms(u16 time);
OS_EXT void Global_AllKey_Prcoess(void);
OS_EXT void PowerON_Handle(void);
OS_EXT void PowerOFF_Handle(void);
OS_EXT void Charging_PowerOFF_Handle(void);
	#undef OS_EXT
#endif

