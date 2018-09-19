#ifndef _KEY_H_
	#define _KEY_H_

#ifdef _KEY_C_
	#define OS_EXT
#else
	#define OS_EXT extern
#endif

#define	MAX_KEY_PER_LINE	4			// 一个 AD 通道的状态 Source Pair Answer NoPress


#define KEY_LP				0x40

#define	KEY_NONE		0x00		
#define KEY_POWER		0x01		
#define KEY_BT			0x02		
#define KEY_ANSWER		0x03		
#define KEY_LINE_IN		0x04		
#define KEY_VOL_UP		0x05
#define KEY_VOL_DOWN	0x06
#define KEY_ANSWER_DOUBLE		0x07

#define KEY_BT_LONG			(KEY_BT|KEY_LP)
#define KEY_ANSER_LONG		(KEY_ANSWER|KEY_LP)
#define KEY_VOL_UP_LONG		(KEY_VOL_UP|KEY_LP)
#define KEY_VOL_DOWN_LOGN		(KEY_VOL_DOWN|KEY_LP)
#define KEY_POWER_LONG			(KEY_POWER|KEY_LP)		//susan+ 121112



OS_EXT const u16 DELAY_0_0[2]		// 单位 ms 亮 灭
#ifdef _KEY_C_
={0, 0}
#endif
;
OS_EXT const u16 DELAY_05_05[2]
#ifdef _KEY_C_
={500, 500}
#endif
;
OS_EXT const u16 DELAY_1_1[2]
#ifdef _KEY_C_
={1000, 1000}
#endif
;
OS_EXT const u16 DELAY_1_3[2]
#ifdef _KEY_C_
={1000, 3000}
#endif
;
OS_EXT const u16 DELAY_02_02[2]
#ifdef _KEY_C_
={200, 200}
#endif
;
OS_EXT const u16 DELAY_025_025[2]
#ifdef _KEY_C_
={250, 250}
#endif
;
OS_EXT const u16 DELAY_02_3[2]
#ifdef _KEY_C_
={100, 3000}
#endif
;
OS_EXT const u16 DELAY_15_15[2]
#ifdef _KEY_C_
={1500, 1500}
#endif
;

#define BT_ANSWER_TIMES	(500/KEY_SCAN_FREQ)		// 蓝牙接电话  进扫描的次数
#define BT_END_TIMES	(2000/KEY_SCAN_FREQ)	// 蓝牙挂电话  进扫描的次数







#define LED_SOLID_ON		0x01
#define LED_SOLID_OFF		0x02
#define LED_BLINK_05_05		0x03
#define LED_BLINK_1_1		0x04
#define LED_BLINK_1_3		0x05
#define LED_BLINK_02_02		0x06
#define LED_BLINK_02_3		0x07
#define LED_BLINK_15_15		0x08
#define LED_BLINK_025_025	0x09

#define BLUETOOTH_BLINK		0x40	// 蓝牙闪烁
#define LINE_IN_BLINK		0x80	// LINE_IN 闪烁
OS_EXT u8   LineInLedMode;			// LINE_IN LED灯的模式



OS_EXT void Key_Scan(void);
OS_EXT void Key_Timer100ms(void);
OS_EXT bool Key_IsRepeatStep(void);
OS_EXT u8  Key_GetCode(void);

	#undef OS_EXT
#endif


