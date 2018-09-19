#ifndef _BLUETOOTH_H_
	#define _BLUETOOTH_H_

#ifdef _BLUETOOTH_C_
	#define OS_EXT
#else
	#define OS_EXT extern
#endif


OS_EXT u8   BTLedMode;				
OS_EXT u8  BT_PairingTimes;			
OS_EXT u8  BT_PairedTimes;			
OS_EXT u8  BT_PhoneTimes;			
OS_EXT u8  u8ChargeTimes;	
OS_EXT bool bCharging;
OS_EXT bool bLowBattery;
OS_EXT void Key_LedBlinkTask(void);
OS_EXT void BT_GetBluetoothSignal(void);
OS_EXT void BT_BTSingalHandle(void);
OS_EXT void BluetoothHandle(void);
OS_EXT void ChargeSingalHandle(void);

	#undef OS_EXT
#endif

