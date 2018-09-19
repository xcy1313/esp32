   1                     ; C Compiler for STM8 (COSMIC Software)
   2                     ; Generator V4.2.8 - 03 Dec 2008
 101                     ; 22 void Global_AllKey_Prcoess(void)
 101                     ; 23 {
 103                     	switch	.text
 104  0000               _Global_AllKey_Prcoess:
 106  0000 88            	push	a
 107       00000001      OFST:	set	1
 110                     ; 25 	u8keycode = Key_GetCode();
 112  0001 cd0000        	call	_Key_GetCode
 114  0004 6b01          	ld	(OFST+0,sp),a
 115                     ; 26 	MainTask &= ~KEY_HANDLE_TASK;
 117  0006 be00          	ldw	x,_MainTask
 118  0008 01            	rrwa	x,a
 119  0009 a4df          	and	a,#223
 120  000b 01            	rrwa	x,a
 121  000c a4ff          	and	a,#255
 122  000e 01            	rrwa	x,a
 123  000f bf00          	ldw	_MainTask,x
 124                     ; 27 	if (u8keycode == KEY_NONE)
 126  0011 0d01          	tnz	(OFST+0,sp)
 127  0013 2602          	jrne	L75
 128                     ; 28 		return;
 131  0015 84            	pop	a
 132  0016 81            	ret
 133  0017               L75:
 134                     ; 29 	if ((u8keycode != KEY_POWER_LONG)&&(IsPowerOn))
 136  0017 7b01          	ld	a,(OFST+0,sp)
 137  0019 a141          	cp	a,#65
 138  001b 2707          	jreq	L16
 140  001d 3d02          	tnz	_IsPowerOn
 141  001f 2703          	jreq	L16
 142                     ; 31 		u16PowerHoldTime = 0;
 144  0021 5f            	clrw	x
 145  0022 bf00          	ldw	_u16PowerHoldTime,x
 146  0024               L16:
 147                     ; 33 	MenuHandler();
 149  0024 ad0e          	call	_MenuHandler
 151                     ; 34 	AudioHandler();
 153  0026 cd0000        	call	_AudioHandler
 155                     ; 35 	switch(SourceSelect)
 157  0029 b600          	ld	a,_SourceSelect
 158  002b a102          	cp	a,#2
 159  002d 2603          	jrne	L56
 162  002f               L72:
 163                     ; 37 		case BLUETOOTH_AUDIO:
 163                     ; 38 			BluetoothHandle();
 165  002f cd0000        	call	_BluetoothHandle
 167                     ; 39 			break;
 169  0032               L13:
 170                     ; 40 		case LINE_IN_AUDIO:
 170                     ; 41 
 170                     ; 42 			break;
 172  0032               L56:
 173                     ; 44 }
 176  0032 84            	pop	a
 177  0033 81            	ret
 222                     ; 57 void MenuHandler(void)
 222                     ; 58 {
 223                     	switch	.text
 224  0034               _MenuHandler:
 226  0034 88            	push	a
 227       00000001      OFST:	set	1
 230                     ; 61 	u8keycode = Key_GetCode();
 232  0035 cd0000        	call	_Key_GetCode
 234  0038 6b01          	ld	(OFST+0,sp),a
 235                     ; 62 	switch(u8keycode)
 237  003a 7b01          	ld	a,(OFST+0,sp)
 239                     ; 100 			break;
 240  003c 4a            	dec	a
 241  003d 270d          	jreq	L76
 242  003f 4a            	dec	a
 243  0040 2741          	jreq	L37
 244  0042 a002          	sub	a,#2
 245  0044 2759          	jreq	L57
 246  0046 a03d          	sub	a,#61
 247  0048 271f          	jreq	L17
 248  004a 2077          	jra	L711
 249  004c               L76:
 250                     ; 65 		case KEY_POWER:
 250                     ; 66 			if (IsPowerOn)
 252  004c 3d02          	tnz	_IsPowerOn
 253  004e 270e          	jreq	L121
 254                     ; 67 				MainTask |= SYSTEM_POWEROFF_TASK;
 256  0050 b600          	ld	a,_MainTask
 257  0052 97            	ld	xl,a
 258  0053 b601          	ld	a,_MainTask+1
 259  0055 aa02          	or	a,#2
 260  0057 b701          	ld	_MainTask+1,a
 261  0059 9f            	ld	a,xl
 262  005a b700          	ld	_MainTask,a
 264  005c 2065          	jra	L711
 265  005e               L121:
 266                     ; 69 				EXTI_INT_Enable(GPIOB, GPIO_Pin_1);
 268  005e 4b02          	push	#2
 269  0060 ae5005        	ldw	x,#20485
 270  0063 cd0000        	call	_EXTI_INT_Enable
 272  0066 84            	pop	a
 273  0067 205a          	jra	L711
 274  0069               L17:
 275                     ; 71 		case KEY_POWER_LONG:
 275                     ; 72 			if (!IsPowerOn)
 277  0069 3d02          	tnz	_IsPowerOn
 278  006b 2756          	jreq	L711
 279                     ; 73 				break;
 281                     ; 74 			u16PowerHoldTime++;
 283  006d be00          	ldw	x,_u16PowerHoldTime
 284  006f 1c0001        	addw	x,#1
 285  0072 bf00          	ldw	_u16PowerHoldTime,x
 286                     ; 75 			if (u16PowerHoldTime >450)
 288  0074 be00          	ldw	x,_u16PowerHoldTime
 289  0076 a301c3        	cpw	x,#451
 290  0079 2548          	jrult	L711
 291                     ; 77 				u16PowerHoldTime = 0;
 293  007b 5f            	clrw	x
 294  007c bf00          	ldw	_u16PowerHoldTime,x
 295                     ; 78 				Golobal_Reset();	
 297  007e cd0198        	call	_Golobal_Reset
 299  0081 2040          	jra	L711
 300  0083               L37:
 301                     ; 81 		case KEY_BT:
 301                     ; 82 			if (!IsPowerOn)
 303  0083 3d02          	tnz	_IsPowerOn
 304  0085 273c          	jreq	L711
 305                     ; 83 				break;
 307                     ; 84 			SourceSelect = BLUETOOTH_AUDIO;
 309  0087 35020000      	mov	_SourceSelect,#2
 310                     ; 85 			Audio_SetAudioSource(SourceSelect);
 312  008b a602          	ld	a,#2
 313  008d cd0000        	call	_Audio_SetAudioSource
 315                     ; 86 			Clock[BT_ANSWER_SHORT_PULSE_CLK] = BT_ANSWER_PULSE_VERYLONG;
 317  0090 ae044c        	ldw	x,#1100
 318  0093 bf0a          	ldw	_Clock+10,x
 319                     ; 87 			BT_ANSWER_BUTTON_H;
 321  0095 7214500f      	bset	20495,#2
 322                     ; 88 			LINE_IN_LED_OFF;
 324  0099 721b500a      	bres	20490,#5
 325                     ; 89 			break;
 327  009d 2024          	jra	L711
 328  009f               L57:
 329                     ; 90 		case KEY_LINE_IN:
 329                     ; 91 			if ((!IsPowerOn)||(BT_PhoneTimes>5))
 331  009f 3d02          	tnz	_IsPowerOn
 332  00a1 2720          	jreq	L711
 334  00a3 b600          	ld	a,_BT_PhoneTimes
 335  00a5 a106          	cp	a,#6
 336  00a7 241a          	jruge	L711
 337                     ; 93 			SourceSelect = LINE_IN_AUDIO;
 339  00a9 35010000      	mov	_SourceSelect,#1
 340                     ; 94 			Audio_SetAudioSource(SourceSelect);
 342  00ad a601          	ld	a,#1
 343  00af cd0000        	call	_Audio_SetAudioSource
 345                     ; 95 			LINE_IN_LED_ON;
 347  00b2 721a500a      	bset	20490,#5
 348                     ; 96 			Clock[BT_ANSWER_SHORT_PULSE_CLK] = BT_ANSWER_PULSE_VERYLONG;
 350  00b6 ae044c        	ldw	x,#1100
 351  00b9 bf0a          	ldw	_Clock+10,x
 352                     ; 97 			BT_ANSWER_BUTTON_H;
 354  00bb 7214500f      	bset	20495,#2
 355                     ; 98 			BT_LED_OFF;
 357  00bf 7219500a      	bres	20490,#4
 358                     ; 100 			break;
 360  00c3               L711:
 361                     ; 103 	DelayNms(20);	
 363  00c3 ae0014        	ldw	x,#20
 364  00c6 cd01d5        	call	_DelayNms
 366                     ; 105 }
 369  00c9 84            	pop	a
 370  00ca 81            	ret
 405                     ; 107 void PowerOFF_Handle(void)
 405                     ; 108 {
 406                     	switch	.text
 407  00cb               _PowerOFF_Handle:
 411                     ; 109 	MainTask &= ~SYSTEM_POWEROFF_TASK;
 413  00cb be00          	ldw	x,_MainTask
 414  00cd 01            	rrwa	x,a
 415  00ce a4fd          	and	a,#253
 416  00d0 01            	rrwa	x,a
 417  00d1 a4ff          	and	a,#255
 418  00d3 01            	rrwa	x,a
 419  00d4 bf00          	ldw	_MainTask,x
 420                     ; 110 	IsPowerOn = FALSE;
 422  00d6 3f02          	clr	_IsPowerOn
 423                     ; 111 	AllLed_OFF();
 425  00d8 cd0174        	call	_AllLed_OFF
 427                     ; 112 	BT_POWER_L;	
 429  00db 72135000      	bres	20480,#1
 430                     ; 113 	if (SourceSelect == BLUETOOTH_AUDIO)
 432  00df b600          	ld	a,_SourceSelect
 433  00e1 a102          	cp	a,#2
 434  00e3 2606          	jrne	L741
 435                     ; 114 		DelayNms(1100);
 437  00e5 ae044c        	ldw	x,#1100
 438  00e8 cd01d5        	call	_DelayNms
 440  00eb               L741:
 441                     ; 115 	IDT_MuteOn();
 443  00eb cd0000        	call	_IDT_MuteOn
 445                     ; 116 	if (!bCharging)
 447  00ee 3d00          	tnz	_bCharging
 448  00f0 261d          	jrne	L151
 449                     ; 118 		ADC1_DeInit();
 451  00f2 cd0000        	call	_ADC1_DeInit
 453                     ; 119 		EXTI_INT_Enable(GPIOB, GPIO_Pin_1);
 455  00f5 4b02          	push	#2
 456  00f7 ae5005        	ldw	x,#20485
 457  00fa cd0000        	call	_EXTI_INT_Enable
 459  00fd 84            	pop	a
 460                     ; 120 		EXTI_INT_Enable(GPIOA, GPIO_Pin_2);
 462  00fe 4b04          	push	#4
 463  0100 ae5000        	ldw	x,#20480
 464  0103 cd0000        	call	_EXTI_INT_Enable
 466  0106 84            	pop	a
 467                     ; 121 		TIM4_Cmd(DISABLE);	
 469  0107 4f            	clr	a
 470  0108 cd0000        	call	_TIM4_Cmd
 472                     ; 122 		TIM2_Cmd(DISABLE);	
 474  010b 4f            	clr	a
 475  010c cd0000        	call	_TIM2_Cmd
 477  010f               L151:
 478                     ; 124 	Write_P_IIC_SDA_0;
 480  010f 721b5005      	bres	20485,#5
 481                     ; 125 	Write_P_IIC_SCL_0;
 483  0113 72195005      	bres	20485,#4
 484                     ; 126 	AMP_ON_L;
 486  0117 72175000      	bres	20480,#3
 487                     ; 127 	if (!bCharging)
 489  011b 3d00          	tnz	_bCharging
 490  011d 2601          	jrne	L351
 491                     ; 128 		HALT;
 494  011f 8e            halt
 496  0120               L351:
 497                     ; 129 }
 500  0120 81            	ret
 532                     ; 132 void PowerON_Handle(void)
 532                     ; 133 {
 533                     	switch	.text
 534  0121               _PowerON_Handle:
 538                     ; 134 	MainTask &= ~SYSTEM_POWERON_TASK;
 540  0121 be00          	ldw	x,_MainTask
 541  0123 01            	rrwa	x,a
 542  0124 a4fb          	and	a,#251
 543  0126 01            	rrwa	x,a
 544  0127 a4ff          	and	a,#255
 545  0129 01            	rrwa	x,a
 546  012a bf00          	ldw	_MainTask,x
 547                     ; 135 	IsPowerOn = TRUE;
 549  012c 35010002      	mov	_IsPowerOn,#1
 550                     ; 136 	MCU_ADC_Init();
 552  0130 cd0000        	call	_MCU_ADC_Init
 554                     ; 137 	TIM4_Cmd(ENABLE);	
 556  0133 a601          	ld	a,#1
 557  0135 cd0000        	call	_TIM4_Cmd
 559                     ; 138 	TIM2_Cmd(ENABLE);	
 561  0138 a601          	ld	a,#1
 562  013a cd0000        	call	_TIM2_Cmd
 564                     ; 139 	if (!bCharging)
 566  013d 3d00          	tnz	_bCharging
 567  013f 2604          	jrne	L561
 568                     ; 140 		POWER_LED_ON;
 570  0141 721e500a      	bset	20490,#7
 571  0145               L561:
 572                     ; 141 	if (SourceSelect == LINE_IN_AUDIO)
 574  0145 b600          	ld	a,_SourceSelect
 575  0147 a101          	cp	a,#1
 576  0149 2604          	jrne	L761
 577                     ; 142 		LINE_IN_LED_ON;
 579  014b 721a500a      	bset	20490,#5
 580  014f               L761:
 581                     ; 143 	AMP_ON_H;
 583  014f 72165000      	bset	20480,#3
 584                     ; 144 	IDT_Init();
 586  0153 cd0000        	call	_IDT_Init
 588                     ; 145 	BT_POWER_H;										
 590  0156 72125000      	bset	20480,#1
 591                     ; 146 	Clock[BT_POWER_PULSE_CLK] = 1000;
 593  015a ae03e8        	ldw	x,#1000
 594  015d bf10          	ldw	_Clock+16,x
 595                     ; 147 }
 598  015f 81            	ret
 625                     ; 149 void Charging_PowerOFF_Handle(void)
 625                     ; 150 {
 626                     	switch	.text
 627  0160               _Charging_PowerOFF_Handle:
 631                     ; 151 	MainTask &= ~CHARGING_POWEROFF_TASK;
 633  0160 be00          	ldw	x,_MainTask
 634  0162 01            	rrwa	x,a
 635  0163 a4bf          	and	a,#191
 636  0165 01            	rrwa	x,a
 637  0166 a4ff          	and	a,#255
 638  0168 01            	rrwa	x,a
 639  0169 bf00          	ldw	_MainTask,x
 640                     ; 152 	MCU_ADC_Init();
 642  016b cd0000        	call	_MCU_ADC_Init
 644                     ; 153 	TIM4_Cmd(ENABLE);	
 646  016e a601          	ld	a,#1
 647  0170 cd0000        	call	_TIM4_Cmd
 649                     ; 154 }
 652  0173 81            	ret
 675                     ; 166 void AllLed_OFF(void)
 675                     ; 167 {
 676                     	switch	.text
 677  0174               _AllLed_OFF:
 681                     ; 168 	POWER_LED_OFF;
 683  0174 721f500a      	bres	20490,#7
 684                     ; 169 	LINE_IN_LED_OFF;
 686  0178 721b500a      	bres	20490,#5
 687                     ; 170 	BT_LED_OFF;
 689  017c 7219500a      	bres	20490,#4
 690                     ; 172 }
 693  0180 81            	ret
 721                     ; 186 void Global_GetADIOValueTask(void)
 721                     ; 187 {
 722                     	switch	.text
 723  0181               _Global_GetADIOValueTask:
 727                     ; 188 	MainTask &= ~GET_AD_IO_VALUE_TASK;
 729  0181 be00          	ldw	x,_MainTask
 730  0183 01            	rrwa	x,a
 731  0184 a4f7          	and	a,#247
 732  0186 01            	rrwa	x,a
 733  0187 a4ff          	and	a,#255
 734  0189 01            	rrwa	x,a
 735  018a bf00          	ldw	_MainTask,x
 736                     ; 189 	Clock[GET_AD_IO_VALUE_CLK] = GET_AD_IO_VALUE_FREQ;
 738  018c ae0004        	ldw	x,#4
 739  018f bf02          	ldw	_Clock+2,x
 740                     ; 190 	Audio_AudioSleepCheck();	//susan 121024
 742  0191 cd0000        	call	_Audio_AudioSleepCheck
 744                     ; 191 	BT_GetBluetoothSignal();
 746  0194 cd0000        	call	_BT_GetBluetoothSignal
 748                     ; 192 }
 751  0197 81            	ret
 774                     ; 204 void Golobal_Reset(void)
 774                     ; 205 {
 775                     	switch	.text
 776  0198               _Golobal_Reset:
 780                     ; 206 	WWDG->CR |= WWDG_CR_WDGA;
 782  0198 721e50d1      	bset	20689,#7
 783                     ; 207 }
 786  019c 81            	ret
 815                     ; 221 void SystemInitial(void)
 815                     ; 222 {
 816                     	switch	.text
 817  019d               _SystemInitial:
 821                     ; 223 	MCU_CLK_Init();
 823  019d cd0000        	call	_MCU_CLK_Init
 825                     ; 224 	GPIO_Configuration();
 827  01a0 cd0000        	call	_GPIO_Configuration
 829                     ; 225 	MCU_Timer_Init();
 831  01a3 cd0000        	call	_MCU_Timer_Init
 833                     ; 227 	ParameterInit();
 835  01a6 ad05          	call	_ParameterInit
 837                     ; 228 	MCU_EXTI_Init();
 839  01a8 cd0000        	call	_MCU_EXTI_Init
 841                     ; 229 	RIM;
 844  01ab 9a            rim
 846                     ; 230 }
 849  01ac 81            	ret
 878                     ; 243 void ParameterInit(void)
 878                     ; 244 {
 879                     	switch	.text
 880  01ad               _ParameterInit:
 884                     ; 245 	Clock[GET_AD_IO_VALUE_CLK] = GET_AD_IO_VALUE_FREQ;
 886  01ad ae0004        	ldw	x,#4
 887  01b0 bf02          	ldw	_Clock+2,x
 888                     ; 246 	Clock[KEY_SCAN_CLK] = KEY_SCAN_FREQ;
 890  01b2 ae000a        	ldw	x,#10
 891  01b5 bf00          	ldw	_Clock,x
 892                     ; 247 	Clock[SYSTEM_SLEEP_CLK] = 0;		
 894  01b7 5f            	clrw	x
 895  01b8 bf04          	ldw	_Clock+4,x
 896                     ; 249 	u8ChargeTimes = 100;
 898  01ba 35640000      	mov	_u8ChargeTimes,#100
 899                     ; 250 	bLowBattery = FALSE;
 901  01be 3f00          	clr	_bLowBattery
 902                     ; 251 	u8Volume = DEFLAUT_VOLUME;	
 904  01c0 35140000      	mov	_u8Volume,#20
 905                     ; 252 	SourceSelect = BLUETOOTH_AUDIO;	
 907  01c4 35020000      	mov	_SourceSelect,#2
 908                     ; 253 	MainTask |= SYSTEM_POWEROFF_TASK;
 910  01c8 b600          	ld	a,_MainTask
 911  01ca 97            	ld	xl,a
 912  01cb b601          	ld	a,_MainTask+1
 913  01cd aa02          	or	a,#2
 914  01cf b701          	ld	_MainTask+1,a
 915  01d1 9f            	ld	a,xl
 916  01d2 b700          	ld	_MainTask,a
 917                     ; 254 }
 920  01d4 81            	ret
 972                     ; 267 void DelayNms(u16 time)
 972                     ; 268 {
 973                     	switch	.text
 974  01d5               _DelayNms:
 976  01d5 89            	pushw	x
 977  01d6 5204          	subw	sp,#4
 978       00000004      OFST:	set	4
 981                     ; 270 	for(i=0;i<time;i++)
 983  01d8 5f            	clrw	x
 984  01d9 1f01          	ldw	(OFST-3,sp),x
 986  01db 2018          	jra	L303
 987  01dd               L772:
 988                     ; 272 		for(j=0; j<264; j++)
 990  01dd 5f            	clrw	x
 991  01de 1f03          	ldw	(OFST-1,sp),x
 992  01e0               L703:
 995  01e0 1e03          	ldw	x,(OFST-1,sp)
 996  01e2 1c0001        	addw	x,#1
 997  01e5 1f03          	ldw	(OFST-1,sp),x
1000  01e7 1e03          	ldw	x,(OFST-1,sp)
1001  01e9 a30108        	cpw	x,#264
1002  01ec 25f2          	jrult	L703
1003                     ; 270 	for(i=0;i<time;i++)
1005  01ee 1e01          	ldw	x,(OFST-3,sp)
1006  01f0 1c0001        	addw	x,#1
1007  01f3 1f01          	ldw	(OFST-3,sp),x
1008  01f5               L303:
1011  01f5 1e01          	ldw	x,(OFST-3,sp)
1012  01f7 1305          	cpw	x,(OFST+1,sp)
1013  01f9 25e2          	jrult	L772
1014                     ; 276 }
1017  01fb 5b06          	addw	sp,#6
1018  01fd 81            	ret
1072                     	xdef	_ParameterInit
1073                     	xdef	_Golobal_Reset
1074                     	xdef	_MenuHandler
1075                     	switch	.ubsct
1076  0000               _u16PowerHoldTime:
1077  0000 0000          	ds.b	2
1078                     	xdef	_u16PowerHoldTime
1079                     	xref	_IDT_MuteOn
1080                     	xref	_IDT_Init
1081                     	xref	_AudioHandler
1082                     	xref	_Audio_AudioSleepCheck
1083                     	xref	_Audio_SetAudioSource
1084                     	xref.b	_u8Volume
1085                     	xref.b	_SourceSelect
1086                     	xref	_BluetoothHandle
1087                     	xref	_BT_GetBluetoothSignal
1088                     	xref.b	_bLowBattery
1089                     	xref.b	_bCharging
1090                     	xref.b	_u8ChargeTimes
1091                     	xref.b	_BT_PhoneTimes
1092                     	xref	_Key_GetCode
1093                     	xdef	_Charging_PowerOFF_Handle
1094                     	xdef	_PowerOFF_Handle
1095                     	xdef	_PowerON_Handle
1096                     	xdef	_Global_AllKey_Prcoess
1097                     	xdef	_DelayNms
1098                     	xdef	_AllLed_OFF
1099                     	xdef	_SystemInitial
1100                     	xdef	_Global_GetADIOValueTask
1101  0002               _IsPowerOn:
1102  0002 00            	ds.b	1
1103                     	xdef	_IsPowerOn
1104                     	xref.b	_MainTask
1105                     	xref	_ADC1_DeInit
1106                     	xref	_MCU_ADC_Init
1107                     	xref	_TIM4_Cmd
1108                     	xref	_TIM2_Cmd
1109                     	xref	_MCU_Timer_Init
1110                     	xref.b	_Clock
1111                     	xref	_EXTI_INT_Enable
1112                     	xref	_MCU_EXTI_Init
1113                     	xref	_GPIO_Configuration
1114                     	xref	_MCU_CLK_Init
1134                     	end
