   1                     ; C Compiler for STM8 (COSMIC Software)
   2                     ; Generator V4.2.8 - 03 Dec 2008
  71                     ; 15 void main(void)
  71                     ; 16 {
  73                     	switch	.text
  74  0000               _main:
  78                     ; 17 	SystemInitial();
  80  0000 cd0000        	call	_SystemInitial
  82  0003               L53:
  83                     ; 21 		if (MainTask & SYSTEM_POWERON_TASK)
  85  0003 b601          	ld	a,_MainTask+1
  86  0005 a504          	bcp	a,#4
  87  0007 2703          	jreq	L14
  88                     ; 23 			PowerON_Handle();
  90  0009 cd0000        	call	_PowerON_Handle
  92  000c               L14:
  93                     ; 25 		if(MainTask & GET_AD_IO_VALUE_TASK)	
  95  000c b601          	ld	a,_MainTask+1
  96  000e a508          	bcp	a,#8
  97  0010 2706          	jreq	L34
  98                     ; 27 			Global_GetADIOValueTask();
 100  0012 cd0000        	call	_Global_GetADIOValueTask
 102                     ; 28 			ChargeSingalHandle();
 104  0015 cd0000        	call	_ChargeSingalHandle
 106  0018               L34:
 107                     ; 30 		if(MainTask & KEY_SCAN_TASK)	
 109  0018 b601          	ld	a,_MainTask+1
 110  001a a501          	bcp	a,#1
 111  001c 2706          	jreq	L54
 112                     ; 32 			Key_Scan();
 114  001e cd0000        	call	_Key_Scan
 116                     ; 33 			BT_BTSingalHandle();
 118  0021 cd0000        	call	_BT_BTSingalHandle
 120  0024               L54:
 121                     ; 36 		if(MainTask &KEY_HANDLE_TASK)
 123  0024 b601          	ld	a,_MainTask+1
 124  0026 a520          	bcp	a,#32
 125  0028 2703          	jreq	L74
 126                     ; 38 			Global_AllKey_Prcoess();
 128  002a cd0000        	call	_Global_AllKey_Prcoess
 130  002d               L74:
 131                     ; 40 		if(MainTask & LED_BLINK_TASK)			
 133  002d b601          	ld	a,_MainTask+1
 134  002f a510          	bcp	a,#16
 135  0031 2703          	jreq	L15
 136                     ; 42 			Key_LedBlinkTask();
 138  0033 cd0000        	call	_Key_LedBlinkTask
 140  0036               L15:
 141                     ; 44 		if(MainTask & SYSTEM_POWEROFF_TASK)		// ÏµÍ³Ë¯Ãß
 143  0036 b601          	ld	a,_MainTask+1
 144  0038 a502          	bcp	a,#2
 145  003a 2703          	jreq	L35
 146                     ; 46 			PowerOFF_Handle();
 148  003c cd0000        	call	_PowerOFF_Handle
 150  003f               L35:
 151                     ; 48 		if (MainTask & CHARGING_POWEROFF_TASK)
 153  003f b601          	ld	a,_MainTask+1
 154  0041 a540          	bcp	a,#64
 155  0043 27be          	jreq	L53
 156                     ; 50 			Charging_PowerOFF_Handle();
 158  0045 cd0000        	call	_Charging_PowerOFF_Handle
 160  0048 20b9          	jra	L53
 173                     	xdef	_main
 174                     	xref	_ChargeSingalHandle
 175                     	xref	_BT_BTSingalHandle
 176                     	xref	_Key_LedBlinkTask
 177                     	xref	_Key_Scan
 178                     	xref	_Charging_PowerOFF_Handle
 179                     	xref	_PowerOFF_Handle
 180                     	xref	_PowerON_Handle
 181                     	xref	_Global_AllKey_Prcoess
 182                     	xref	_SystemInitial
 183                     	xref	_Global_GetADIOValueTask
 184                     	switch	.ubsct
 185  0000               _MainTask:
 186  0000 0000          	ds.b	2
 187                     	xdef	_MainTask
 207                     	end
