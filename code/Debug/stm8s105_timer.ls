   1                     ; C Compiler for STM8 (COSMIC Software)
   2                     ; Generator V4.2.8 - 03 Dec 2008
 103                     ; 18 void MCU_Timer_Init(void)
 103                     ; 19 {
 105                     	switch	.text
 106  0000               _MCU_Timer_Init:
 110                     ; 21 	TIM2_TimeBaseInit(TIM2_PRESCALER_16, 200);
 112  0000 ae00c8        	ldw	x,#200
 113  0003 89            	pushw	x
 114  0004 a604          	ld	a,#4
 115  0006 cd0195        	call	_TIM2_TimeBaseInit
 117  0009 85            	popw	x
 118                     ; 22 	TIM2->IER |= (u8)(TIM2_IER_UIE);
 120  000a 72105303      	bset	21251,#0
 121                     ; 23 	TIM2_Cmd(ENABLE);
 123  000e a601          	ld	a,#1
 124  0010 cd01fb        	call	_TIM2_Cmd
 126                     ; 26 	TIM4_TimeBaseInit(TIM4_PRESCALER_16, 250);	// Time base configuration
 128  0013 ae00fa        	ldw	x,#250
 129  0016 a604          	ld	a,#4
 130  0018 95            	ld	xh,a
 131  0019 cd0209        	call	_TIM4_TimeBaseInit
 133                     ; 27 	TIM4_ClearFlag(TIM4_FLAG_UPDATE);			// Clear TIM4 update flag
 135  001c a601          	ld	a,#1
 136  001e cd0212        	call	_TIM4_ClearFlag
 138                     ; 28 	TIM4_ITConfig(TIM4_IT_UPDATE, ENABLE);		// Enable update interrupt
 140  0021 ae0001        	ldw	x,#1
 141  0024 a601          	ld	a,#1
 142  0026 95            	ld	xh,a
 143  0027 cd0217        	call	_TIM4_ITConfig
 145                     ; 29 	TIM4_Cmd(ENABLE);							// Enable TIM4 (作為系統時期 1ms 中斷)
 147  002a a601          	ld	a,#1
 148  002c cd0230        	call	_TIM4_Cmd
 150                     ; 30 }
 153  002f 81            	ret
 179                     ; 43 @near @interrupt void TIM2_UPD_OVF_BRK_IRQHandler(void)
 179                     ; 44 {
 180                     	switch	.text
 181  0030               _TIM2_UPD_OVF_BRK_IRQHandler:
 185                     ; 45 	TIM2->SR1 = (u8)(~TIM2_SR1_UIF);
 187  0030 35fe5304      	mov	21252,#254
 188                     ; 46 	if (bBeepEnabled)
 190  0034 3d02          	tnz	_bBeepEnabled
 191  0036 271b          	jreq	L56
 192                     ; 48 		PODR_BEEP_PIN1 = !PODR_BEEP_PIN1;
 194  0038 9012500a      	bcpl	20490,#1
 195                     ; 49 		PODR_BEEP_PIN2 = !PODR_BEEP_PIN2;
 197  003c 901a5014      	bcpl	20500,#5
 198                     ; 50 		u16BeepTimer ++;
 200  0040 be00          	ldw	x,_u16BeepTimer
 201  0042 1c0001        	addw	x,#1
 202  0045 bf00          	ldw	_u16BeepTimer,x
 203                     ; 51 		if (u16BeepTimer == 200)
 205  0047 be00          	ldw	x,_u16BeepTimer
 206  0049 a300c8        	cpw	x,#200
 207  004c 2605          	jrne	L56
 208                     ; 53 			bBeepEnabled = FALSE;
 210  004e 3f02          	clr	_bBeepEnabled
 211                     ; 54 			u16BeepTimer = 0;
 213  0050 5f            	clrw	x
 214  0051 bf00          	ldw	_u16BeepTimer,x
 215  0053               L56:
 216                     ; 60 	return;
 219  0053 80            	iret
 243                     ; 74 @near @interrupt void TIM2_CAP_COM_IRQHandler(void)
 243                     ; 75 {
 244                     	switch	.text
 245  0054               _TIM2_CAP_COM_IRQHandler:
 249                     ; 76 	return;
 252  0054 80            	iret
 308                     ; 89 void Beeper_SetPlay(bool bPlay)
 308                     ; 90 {
 309                     	switch	.text
 310  0055               _Beeper_SetPlay:
 314                     ; 91 	bBeepEnabled = bPlay;
 316  0055 b702          	ld	_bBeepEnabled,a
 317                     ; 92 }
 320  0057 81            	ret
 351                     ; 106 @near @interrupt void TIM4_UPD_OVF_IRQHandler(void)
 351                     ; 107 {
 352                     	switch	.text
 353  0058               _TIM4_UPD_OVF_IRQHandler:
 355  0058 3b0002        	push	c_x+2
 356  005b be00          	ldw	x,c_x
 357  005d 89            	pushw	x
 358  005e 3b0002        	push	c_y+2
 359  0061 be00          	ldw	x,c_y
 360  0063 89            	pushw	x
 363                     ; 109 	TIM4->SR1 = (u8)(~TIM4_IT_UPDATE);				// 中断中尽量少占用时间
 365  0064 35fe5344      	mov	21316,#254
 366                     ; 111 	SysTickMSec++;									// 毫秒
 368  0068 be21          	ldw	x,_SysTickMSec
 369  006a 1c0001        	addw	x,#1
 370  006d bf21          	ldw	_SysTickMSec,x
 371                     ; 112 	if(SysTickMSec >= 1000)
 373  006f be21          	ldw	x,_SysTickMSec
 374  0071 a303e8        	cpw	x,#1000
 375  0074 252c          	jrult	L731
 376                     ; 114 		SysTickMSec = 0;
 378  0076 5f            	clrw	x
 379  0077 bf21          	ldw	_SysTickMSec,x
 380                     ; 116 		SysTickSecond++;							// 秒
 382  0079 3c20          	inc	_SysTickSecond
 383                     ; 117 		if(SysTickSecond >= 60)
 385  007b b620          	ld	a,_SysTickSecond
 386  007d a13c          	cp	a,#60
 387  007f 2516          	jrult	L141
 388                     ; 119 			SysTickSecond = 0;
 390  0081 3f20          	clr	_SysTickSecond
 391                     ; 121 			SysTickMinute++;						// 分
 393  0083 3c1f          	inc	_SysTickMinute
 394                     ; 123 			if((Clock[SYSTEM_SLEEP_CLK] < 20)&&(IsPowerOn))	// min级 系统睡眠
 396  0085 be07          	ldw	x,_Clock+4
 397  0087 a30014        	cpw	x,#20
 398  008a 240b          	jruge	L141
 400  008c 3d00          	tnz	_IsPowerOn
 401  008e 2707          	jreq	L141
 402                     ; 125 				Clock[SYSTEM_SLEEP_CLK]++;
 404  0090 be07          	ldw	x,_Clock+4
 405  0092 1c0001        	addw	x,#1
 406  0095 bf07          	ldw	_Clock+4,x
 407  0097               L141:
 408                     ; 129 		if(Clock[SOURCE_SWITCH_PAUSE_CLK] != 0)		// S级  发送暂停的频率
 410  0097 be15          	ldw	x,_Clock+18
 411  0099 2707          	jreq	L731
 412                     ; 131 			Clock[SOURCE_SWITCH_PAUSE_CLK]--;
 414  009b be15          	ldw	x,_Clock+18
 415  009d 1d0001        	subw	x,#1
 416  00a0 bf15          	ldw	_Clock+18,x
 417  00a2               L731:
 418                     ; 134 	if (SysTickMSec % 80 == 0)
 420  00a2 be21          	ldw	x,_SysTickMSec
 421  00a4 a650          	ld	a,#80
 422  00a6 62            	div	x,a
 423  00a7 5f            	clrw	x
 424  00a8 97            	ld	xl,a
 425  00a9 a30000        	cpw	x,#0
 426  00ac 2603          	jrne	L741
 427                     ; 136 		Key_Timer100ms();
 429  00ae cd0000        	call	_Key_Timer100ms
 431  00b1               L741:
 432                     ; 138 	if(Clock[GET_AD_IO_VALUE_CLK] != 0)				// ms级  获取AD值
 434  00b1 be05          	ldw	x,_Clock+2
 435  00b3 2717          	jreq	L151
 436                     ; 140 		Clock[GET_AD_IO_VALUE_CLK]--;
 438  00b5 be05          	ldw	x,_Clock+2
 439  00b7 1d0001        	subw	x,#1
 440  00ba bf05          	ldw	_Clock+2,x
 441                     ; 141 		if(Clock[GET_AD_IO_VALUE_CLK] == 0)
 443  00bc be05          	ldw	x,_Clock+2
 444  00be 260c          	jrne	L151
 445                     ; 143 			MainTask |= GET_AD_IO_VALUE_TASK;
 447  00c0 b600          	ld	a,_MainTask
 448  00c2 97            	ld	xl,a
 449  00c3 b601          	ld	a,_MainTask+1
 450  00c5 aa08          	or	a,#8
 451  00c7 b701          	ld	_MainTask+1,a
 452  00c9 9f            	ld	a,xl
 453  00ca b700          	ld	_MainTask,a
 454  00cc               L151:
 455                     ; 147 	if(Clock[KEY_SCAN_CLK] != 0)					// ms级  按键扫描
 457  00cc be03          	ldw	x,_Clock
 458  00ce 2717          	jreq	L551
 459                     ; 149 		Clock[KEY_SCAN_CLK]--;
 461  00d0 be03          	ldw	x,_Clock
 462  00d2 1d0001        	subw	x,#1
 463  00d5 bf03          	ldw	_Clock,x
 464                     ; 150 		if(Clock[KEY_SCAN_CLK] == 0)
 466  00d7 be03          	ldw	x,_Clock
 467  00d9 260c          	jrne	L551
 468                     ; 152 			MainTask |= KEY_SCAN_TASK;
 470  00db b600          	ld	a,_MainTask
 471  00dd 97            	ld	xl,a
 472  00de b601          	ld	a,_MainTask+1
 473  00e0 aa01          	or	a,#1
 474  00e2 b701          	ld	_MainTask+1,a
 475  00e4 9f            	ld	a,xl
 476  00e5 b700          	ld	_MainTask,a
 477  00e7               L551:
 478                     ; 156 	if(Clock[BT_LED_BLINK_CLK] != 0)				// ms级  蓝牙LED 闪烁
 480  00e7 be09          	ldw	x,_Clock+6
 481  00e9 2717          	jreq	L161
 482                     ; 158 		Clock[BT_LED_BLINK_CLK]--;
 484  00eb be09          	ldw	x,_Clock+6
 485  00ed 1d0001        	subw	x,#1
 486  00f0 bf09          	ldw	_Clock+6,x
 487                     ; 159 		if(Clock[BT_LED_BLINK_CLK] == 0)
 489  00f2 be09          	ldw	x,_Clock+6
 490  00f4 260c          	jrne	L161
 491                     ; 161 			MainTask |= LED_BLINK_TASK;
 493  00f6 b600          	ld	a,_MainTask
 494  00f8 97            	ld	xl,a
 495  00f9 b601          	ld	a,_MainTask+1
 496  00fb aa10          	or	a,#16
 497  00fd b701          	ld	_MainTask+1,a
 498  00ff 9f            	ld	a,xl
 499  0100 b700          	ld	_MainTask,a
 500  0102               L161:
 501                     ; 165 	if(Clock[BT_PAIRING_PULSE_CLK] != 0)			// ms级  BT 配对脉冲
 503  0102 be0b          	ldw	x,_Clock+8
 504  0104 2707          	jreq	L561
 505                     ; 167 		Clock[BT_PAIRING_PULSE_CLK]--;
 507  0106 be0b          	ldw	x,_Clock+8
 508  0108 1d0001        	subw	x,#1
 509  010b bf0b          	ldw	_Clock+8,x
 510  010d               L561:
 511                     ; 171 	if(Clock[BT_ANSWER_DOUBLE_PULSE_CLK] > 1)				// ms级  BT Answer 脉冲
 513  010d be0f          	ldw	x,_Clock+12
 514  010f a30002        	cpw	x,#2
 515  0112 2529          	jrult	L761
 516                     ; 173 		Clock[BT_ANSWER_DOUBLE_PULSE_CLK]--;
 518  0114 be0f          	ldw	x,_Clock+12
 519  0116 1d0001        	subw	x,#1
 520  0119 bf0f          	ldw	_Clock+12,x
 521                     ; 174 		if (Clock[BT_ANSWER_DOUBLE_PULSE_CLK] >= 500)
 523  011b be0f          	ldw	x,_Clock+12
 524  011d a301f4        	cpw	x,#500
 525  0120 2506          	jrult	L171
 526                     ; 176 			BT_ANSWER_BUTTON_H;
 528  0122 7214500f      	bset	20495,#2
 530  0126 2015          	jra	L761
 531  0128               L171:
 532                     ; 178 		else if (Clock[BT_ANSWER_DOUBLE_PULSE_CLK] >= 250)
 534  0128 be0f          	ldw	x,_Clock+12
 535  012a a300fa        	cpw	x,#250
 536  012d 2506          	jrult	L571
 537                     ; 180 			BT_ANSWER_BUTTON_L;
 539  012f 7215500f      	bres	20495,#2
 541  0133 2008          	jra	L761
 542  0135               L571:
 543                     ; 182 		else if (Clock[BT_ANSWER_DOUBLE_PULSE_CLK] > 0)
 545  0135 be0f          	ldw	x,_Clock+12
 546  0137 2704          	jreq	L761
 547                     ; 184 			BT_ANSWER_BUTTON_H;
 549  0139 7214500f      	bset	20495,#2
 550  013d               L761:
 551                     ; 187 	if (Clock[BT_ANSWER_SHORT_PULSE_CLK] >1)
 553  013d be0d          	ldw	x,_Clock+10
 554  013f a30002        	cpw	x,#2
 555  0142 2507          	jrult	L302
 556                     ; 188 		Clock[BT_ANSWER_SHORT_PULSE_CLK]--;
 558  0144 be0d          	ldw	x,_Clock+10
 559  0146 1d0001        	subw	x,#1
 560  0149 bf0d          	ldw	_Clock+10,x
 561  014b               L302:
 562                     ; 190 	if(Clock[CHARGE_LED_BLINK_CLK] >0)				// ms级
 564  014b be17          	ldw	x,_Clock+20
 565  014d 2707          	jreq	L502
 566                     ; 192 		Clock[CHARGE_LED_BLINK_CLK]--;
 568  014f be17          	ldw	x,_Clock+20
 569  0151 1d0001        	subw	x,#1
 570  0154 bf17          	ldw	_Clock+20,x
 571  0156               L502:
 572                     ; 195 	if(Clock[BT_POWER_PULSE_CLK] != 0)				// ms级  BT Power 脉冲
 574  0156 be13          	ldw	x,_Clock+16
 575  0158 2707          	jreq	L702
 576                     ; 197 		Clock[BT_POWER_PULSE_CLK]--;
 578  015a be13          	ldw	x,_Clock+16
 579  015c 1d0001        	subw	x,#1
 580  015f bf13          	ldw	_Clock+16,x
 581  0161               L702:
 582                     ; 200 	if (Clock[BT_ANSWER_HANDLE] > 1)
 584  0161 be19          	ldw	x,_Clock+22
 585  0163 a30002        	cpw	x,#2
 586  0166 2507          	jrult	L112
 587                     ; 201 		Clock[BT_ANSWER_HANDLE]--;
 589  0168 be19          	ldw	x,_Clock+22
 590  016a 1d0001        	subw	x,#1
 591  016d bf19          	ldw	_Clock+22,x
 592  016f               L112:
 593                     ; 203 	if(Clock[BT_VOLMAX_CLK] != 0)				// ms级  BT Power 脉冲
 595  016f be1b          	ldw	x,_Clock+24
 596  0171 2707          	jreq	L312
 597                     ; 205 		Clock[BT_VOLMAX_CLK]--;
 599  0173 be1b          	ldw	x,_Clock+24
 600  0175 1d0001        	subw	x,#1
 601  0178 bf1b          	ldw	_Clock+24,x
 602  017a               L312:
 603                     ; 208 	if (Clock[BT_LOWBATTERY_CLK] > 1)
 605  017a be1d          	ldw	x,_Clock+26
 606  017c a30002        	cpw	x,#2
 607  017f 2507          	jrult	L512
 608                     ; 210 		Clock[BT_LOWBATTERY_CLK] --;
 610  0181 be1d          	ldw	x,_Clock+26
 611  0183 1d0001        	subw	x,#1
 612  0186 bf1d          	ldw	_Clock+26,x
 613  0188               L512:
 614                     ; 212 	return;
 617  0188 85            	popw	x
 618  0189 bf00          	ldw	c_y,x
 619  018b 320002        	pop	c_y+2
 620  018e 85            	popw	x
 621  018f bf00          	ldw	c_x,x
 622  0191 320002        	pop	c_x+2
 623  0194 80            	iret
 791                     ; 222 void TIM2_TimeBaseInit(TIM2_Prescaler_TypeDef TIM2_Prescaler, u16 TIM2_Period)
 791                     ; 223 {
 792                     	switch	.text
 793  0195               _TIM2_TimeBaseInit:
 795  0195 88            	push	a
 796       00000000      OFST:	set	0
 799                     ; 224 	TIM2->PSCR = (u8)(TIM2_Prescaler);		// Set the Prescaler value
 801  0196 c7530e        	ld	21262,a
 802                     ; 225 	TIM2->ARRH = (u8)(TIM2_Period >> 8);	// Set the Autoreload value
 804  0199 7b04          	ld	a,(OFST+4,sp)
 805  019b c7530f        	ld	21263,a
 806                     ; 226 	TIM2->ARRL = (u8)(TIM2_Period);
 808  019e 7b05          	ld	a,(OFST+5,sp)
 809  01a0 c75310        	ld	21264,a
 810                     ; 227 }
 813  01a3 84            	pop	a
 814  01a4 81            	ret
 882                     ; 236 void TIM2_PrescalerConfig(TIM2_Prescaler_TypeDef Prescaler, TIM2_PSCReloadMode_TypeDef TIM2_PSCReloadMode)
 882                     ; 237 {
 883                     	switch	.text
 884  01a5               _TIM2_PrescalerConfig:
 888                     ; 238 	TIM2->PSCR = (u8)Prescaler;				// Set the Prescaler value
 890  01a5 9e            	ld	a,xh
 891  01a6 c7530e        	ld	21262,a
 892                     ; 239 	TIM2->EGR  = (u8)TIM2_PSCReloadMode;	// Set or reset the UG Bit
 894  01a9 9f            	ld	a,xl
 895  01aa c75306        	ld	21254,a
 896                     ; 240 }
 899  01ad 81            	ret
1056                     ; 251 void TIM2_OC1Init(TIM2_OCMode_TypeDef TIM2_OCMode, TIM2_OutputState_TypeDef TIM2_OutputState, u8 TIM2_Pulse, TIM2_OCPolarity_TypeDef TIM2_OCPolarity)
1056                     ; 252 {
1057                     	switch	.text
1058  01ae               _TIM2_OC1Init:
1060  01ae 89            	pushw	x
1061  01af 88            	push	a
1062       00000001      OFST:	set	1
1065                     ; 254 	TIM2->CCER1 &= (u8)(~( TIM2_CCER1_CC1E | TIM2_CCER1_CC1P));
1067  01b0 c6530a        	ld	a,21258
1068  01b3 a4fc          	and	a,#252
1069  01b5 c7530a        	ld	21258,a
1070                     ; 256 	TIM2->CCER1 |= (u8)((u8)(TIM2_OutputState & TIM2_CCER1_CC1E ) | (u8)(TIM2_OCPolarity & TIM2_CCER1_CC1P));
1072  01b8 7b07          	ld	a,(OFST+6,sp)
1073  01ba a402          	and	a,#2
1074  01bc 6b01          	ld	(OFST+0,sp),a
1075  01be 9f            	ld	a,xl
1076  01bf a401          	and	a,#1
1077  01c1 1a01          	or	a,(OFST+0,sp)
1078  01c3 ca530a        	or	a,21258
1079  01c6 c7530a        	ld	21258,a
1080                     ; 258 	TIM2->CCMR1 = (u8)((u8)(TIM2->CCMR1 & (u8)(~TIM2_CCMR_OCM)) | (u8)TIM2_OCMode);
1082  01c9 c65307        	ld	a,21255
1083  01cc a48f          	and	a,#143
1084  01ce 1a02          	or	a,(OFST+1,sp)
1085  01d0 c75307        	ld	21255,a
1086                     ; 260 	TIM2->CCR1H = (u8)(TIM2_Pulse >> 8);
1088  01d3 725f5311      	clr	21265
1089                     ; 261 	TIM2->CCR1L = (u8)(TIM2_Pulse);
1091  01d7 7b06          	ld	a,(OFST+5,sp)
1092  01d9 c75312        	ld	21266,a
1093                     ; 262 }
1096  01dc 5b03          	addw	sp,#3
1097  01de 81            	ret
1153                     ; 271 void TIM2_OC1PreloadConfig(FunctionalState NewState)
1153                     ; 272 {
1154                     	switch	.text
1155  01df               _TIM2_OC1PreloadConfig:
1159                     ; 274 	if (NewState != DISABLE)	// Set or Reset the OC1PE Bit
1161  01df 4d            	tnz	a
1162  01e0 2706          	jreq	L754
1163                     ; 276 		TIM2->CCMR1 |= (u8)TIM2_CCMR_OCxPE;
1165  01e2 72165307      	bset	21255,#3
1167  01e6 2004          	jra	L164
1168  01e8               L754:
1169                     ; 280 		TIM2->CCMR1 &= (u8)(~TIM2_CCMR_OCxPE);
1171  01e8 72175307      	bres	21255,#3
1172  01ec               L164:
1173                     ; 282 }
1176  01ec 81            	ret
1212                     ; 290 void TIM2_ARRPreloadConfig(FunctionalState NewState)
1212                     ; 291 {
1213                     	switch	.text
1214  01ed               _TIM2_ARRPreloadConfig:
1218                     ; 292 	if (NewState != DISABLE)	// Set or Reset the ARPE Bit
1220  01ed 4d            	tnz	a
1221  01ee 2706          	jreq	L105
1222                     ; 294 		TIM2->CR1 |= (u8)TIM2_CR1_ARPE;
1224  01f0 721e5300      	bset	21248,#7
1226  01f4 2004          	jra	L305
1227  01f6               L105:
1228                     ; 298 		TIM2->CR1 &= (u8)(~TIM2_CR1_ARPE);
1230  01f6 721f5300      	bres	21248,#7
1231  01fa               L305:
1232                     ; 300 }
1235  01fa 81            	ret
1270                     ; 309 void TIM2_Cmd(FunctionalState NewState)
1270                     ; 310 {
1271                     	switch	.text
1272  01fb               _TIM2_Cmd:
1276                     ; 311 	if (NewState != DISABLE)	// set or Reset the CEN Bit
1278  01fb 4d            	tnz	a
1279  01fc 2706          	jreq	L325
1280                     ; 313 		TIM2->CR1 |= (u8)TIM2_CR1_CEN;
1282  01fe 72105300      	bset	21248,#0
1284  0202 2004          	jra	L525
1285  0204               L325:
1286                     ; 317 		TIM2->CR1 &= (u8)(~TIM2_CR1_CEN);
1288  0204 72115300      	bres	21248,#0
1289  0208               L525:
1290                     ; 319 }
1293  0208 81            	ret
1399                     ; 332 void TIM4_TimeBaseInit(TIM4_Prescaler_TypeDef TIM4_Prescaler, u8 TIM4_Period)
1399                     ; 333 {
1400                     	switch	.text
1401  0209               _TIM4_TimeBaseInit:
1405                     ; 334 	TIM4->PSCR = (u8)(TIM4_Prescaler);	// Set the Prescaler value
1407  0209 9e            	ld	a,xh
1408  020a c75347        	ld	21319,a
1409                     ; 335 	TIM4->ARR = (u8)(TIM4_Period);		// Set the Autoreload value
1411  020d 9f            	ld	a,xl
1412  020e c75348        	ld	21320,a
1413                     ; 336 }
1416  0211 81            	ret
1464                     ; 349 void TIM4_ClearFlag(TIM4_FLAG_TypeDef TIM4_FLAG)
1464                     ; 350 {
1465                     	switch	.text
1466  0212               _TIM4_ClearFlag:
1470                     ; 351 	TIM4->SR1 = (u8)(~TIM4_FLAG);
1472  0212 43            	cpl	a
1473  0213 c75344        	ld	21316,a
1474                     ; 352 }
1477  0216 81            	ret
1535                     ; 365 void TIM4_ITConfig(TIM4_IT_TypeDef TIM4_IT, FunctionalState NewState)
1535                     ; 366 {
1536                     	switch	.text
1537  0217               _TIM4_ITConfig:
1539  0217 89            	pushw	x
1540       00000000      OFST:	set	0
1543                     ; 367 	if (NewState != DISABLE)
1545  0218 9f            	ld	a,xl
1546  0219 4d            	tnz	a
1547  021a 2709          	jreq	L156
1548                     ; 369 		TIM4->IER |= (u8)TIM4_IT;		// Enable the Interrupt sources
1550  021c 9e            	ld	a,xh
1551  021d ca5343        	or	a,21315
1552  0220 c75343        	ld	21315,a
1554  0223 2009          	jra	L356
1555  0225               L156:
1556                     ; 373 		TIM4->IER &= (u8)(~TIM4_IT);	// Disable the Interrupt sources
1558  0225 7b01          	ld	a,(OFST+1,sp)
1559  0227 43            	cpl	a
1560  0228 c45343        	and	a,21315
1561  022b c75343        	ld	21315,a
1562  022e               L356:
1563                     ; 375 }
1566  022e 85            	popw	x
1567  022f 81            	ret
1602                     ; 388 void TIM4_Cmd(FunctionalState NewState)
1602                     ; 389 {
1603                     	switch	.text
1604  0230               _TIM4_Cmd:
1608                     ; 390 	if (NewState != DISABLE)	// set or Reset the CEN Bit
1610  0230 4d            	tnz	a
1611  0231 2706          	jreq	L376
1612                     ; 392 		TIM4->CR1 |= TIM4_CR1_CEN;
1614  0233 72105340      	bset	21312,#0
1616  0237 2004          	jra	L576
1617  0239               L376:
1618                     ; 396 		TIM4->CR1 &= (u8)(~TIM4_CR1_CEN);
1620  0239 72115340      	bres	21312,#0
1621  023d               L576:
1622                     ; 398 }
1625  023d 81            	ret
1661                     ; 411 void TIM4_ClearITPendingBit(TIM4_IT_TypeDef TIM4_IT)
1661                     ; 412 {
1662                     	switch	.text
1663  023e               _TIM4_ClearITPendingBit:
1667                     ; 413 	TIM4->SR1 = (u8)(~TIM4_IT);	// Clear the IT pending Bit
1669  023e 43            	cpl	a
1670  023f c75344        	ld	21316,a
1671                     ; 414 }
1674  0242 81            	ret
1708                     	switch	.ubsct
1709  0000               _u16BeepTimer:
1710  0000 0000          	ds.b	2
1711                     	xdef	_u16BeepTimer
1712  0002               _bBeepEnabled:
1713  0002 00            	ds.b	1
1714                     	xdef	_bBeepEnabled
1715                     	xref	_Key_Timer100ms
1716                     	xref.b	_IsPowerOn
1717                     	xref.b	_MainTask
1718                     	xdef	_Beeper_SetPlay
1719                     	xdef	_TIM4_ClearITPendingBit
1720                     	xdef	_TIM4_Cmd
1721                     	xdef	_TIM4_ITConfig
1722                     	xdef	_TIM4_ClearFlag
1723                     	xdef	_TIM4_TimeBaseInit
1724                     	xdef	_TIM2_Cmd
1725                     	xdef	_TIM2_ARRPreloadConfig
1726                     	xdef	_TIM2_OC1PreloadConfig
1727                     	xdef	_TIM2_OC1Init
1728                     	xdef	_TIM2_PrescalerConfig
1729                     	xdef	_TIM2_TimeBaseInit
1730                     	xdef	_TIM4_UPD_OVF_IRQHandler
1731                     	xdef	_TIM2_CAP_COM_IRQHandler
1732                     	xdef	_TIM2_UPD_OVF_BRK_IRQHandler
1733                     	xdef	_MCU_Timer_Init
1734  0003               _Clock:
1735  0003 000000000000  	ds.b	28
1736                     	xdef	_Clock
1737  001f               _SysTickMinute:
1738  001f 00            	ds.b	1
1739                     	xdef	_SysTickMinute
1740  0020               _SysTickSecond:
1741  0020 00            	ds.b	1
1742                     	xdef	_SysTickSecond
1743  0021               _SysTickMSec:
1744  0021 0000          	ds.b	2
1745                     	xdef	_SysTickMSec
1746  0023               _SysTickTimer:
1747  0023 0000          	ds.b	2
1748                     	xdef	_SysTickTimer
1749                     	xref.b	c_x
1750                     	xref.b	c_y
1770                     	end
