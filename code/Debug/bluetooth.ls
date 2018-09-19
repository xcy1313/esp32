   1                     ; C Compiler for STM8 (COSMIC Software)
   2                     ; Generator V4.2.8 - 03 Dec 2008
 109                     	bsct
 110  0000               _KeyTimes:
 111  0000 00            	dc.b	0
 155                     ; 35 void BluetoothHandle(void)
 155                     ; 36 {
 157                     	switch	.text
 158  0000               _BluetoothHandle:
 160  0000 88            	push	a
 161       00000001      OFST:	set	1
 164                     ; 39 	if (!IsPowerOn)
 166  0001 3d00          	tnz	_IsPowerOn
 167  0003 2602          	jrne	L111
 168                     ; 40 		return;
 171  0005 84            	pop	a
 172  0006 81            	ret
 173  0007               L111:
 174                     ; 41 	u8keycode = Key_GetCode();
 176  0007 cd0000        	call	_Key_GetCode
 178  000a 6b01          	ld	(OFST+0,sp),a
 179                     ; 42 	switch(u8keycode)
 181  000c 7b01          	ld	a,(OFST+0,sp)
 183                     ; 56 			break;
 184  000e a003          	sub	a,#3
 185  0010 2714          	jreq	L16
 186  0012 a03f          	sub	a,#63
 187  0014 2705          	jreq	L75
 188  0016 4a            	dec	a
 189  0017 271c          	jreq	L36
 190  0019 2023          	jra	L511
 191  001b               L75:
 192                     ; 44 		case KEY_BT_LONG:
 192                     ; 45 			Clock[BT_PAIRING_PULSE_CLK] = BT_PAIRING_PULSE;	// 置脉冲宽度
 194  001b ae0064        	ldw	x,#100
 195  001e bf08          	ldw	_Clock+8,x
 196                     ; 46 			BT_PAIR_BUTTON_H;
 198  0020 7218500f      	bset	20495,#4
 199                     ; 47 			break;
 201  0024 2018          	jra	L511
 202  0026               L16:
 203                     ; 48 		case KEY_ANSWER:
 203                     ; 49 			KeyTimes++;
 205  0026 3c00          	inc	_KeyTimes
 206                     ; 50 			if (KeyTimes ==1)
 208  0028 b600          	ld	a,_KeyTimes
 209  002a a101          	cp	a,#1
 210  002c 2610          	jrne	L511
 211                     ; 51 				Clock[BT_ANSWER_HANDLE] = 700;
 213  002e ae02bc        	ldw	x,#700
 214  0031 bf16          	ldw	_Clock+22,x
 215  0033 2009          	jra	L511
 216  0035               L36:
 217                     ; 53 		case KEY_ANSER_LONG:
 217                     ; 54 			Clock[BT_ANSWER_SHORT_PULSE_CLK] = BT_ANSWER_PULSE_MIDDLE;
 219  0035 ae01fe        	ldw	x,#510
 220  0038 bf0a          	ldw	_Clock+10,x
 221                     ; 55 			BT_ANSWER_BUTTON_H;
 223  003a 7214500f      	bset	20495,#2
 224                     ; 56 			break;
 226  003e               L511:
 227                     ; 58 }
 230  003e 84            	pop	a
 231  003f 81            	ret
 260                     ; 72 void BT_GetBluetoothSignal(void)
 260                     ; 73 {
 261                     	switch	.text
 262  0040               _BT_GetBluetoothSignal:
 266                     ; 74 	if(BT_PAIRING_READ)						// 读连接中 状态
 268  0040 c65010        	ld	a,20496
 269  0043 a540          	bcp	a,#64
 270  0045 270a          	jreq	L131
 271                     ; 76 		if(BT_PairingTimes<10)				// 消抖
 273  0047 b60c          	ld	a,_BT_PairingTimes
 274  0049 a10a          	cp	a,#10
 275  004b 240a          	jruge	L531
 276                     ; 78 			BT_PairingTimes++;
 278  004d 3c0c          	inc	_BT_PairingTimes
 279  004f 2006          	jra	L531
 280  0051               L131:
 281                     ; 83 		if(BT_PairingTimes)
 283  0051 3d0c          	tnz	_BT_PairingTimes
 284  0053 2702          	jreq	L531
 285                     ; 85 			BT_PairingTimes--;
 287  0055 3a0c          	dec	_BT_PairingTimes
 288  0057               L531:
 289                     ; 89 	if(BT_PAIRED_READ)						// 读已配对 状态
 291  0057 c65010        	ld	a,20496
 292  005a a520          	bcp	a,#32
 293  005c 270a          	jreq	L141
 294                     ; 91 		if(BT_PairedTimes<10)				// 消抖
 296  005e b60b          	ld	a,_BT_PairedTimes
 297  0060 a10a          	cp	a,#10
 298  0062 240a          	jruge	L541
 299                     ; 93 			BT_PairedTimes++;
 301  0064 3c0b          	inc	_BT_PairedTimes
 302  0066 2006          	jra	L541
 303  0068               L141:
 304                     ; 98 		if(BT_PairedTimes)
 306  0068 3d0b          	tnz	_BT_PairedTimes
 307  006a 2702          	jreq	L541
 308                     ; 100 			BT_PairedTimes--;
 310  006c 3a0b          	dec	_BT_PairedTimes
 311  006e               L541:
 312                     ; 103 	if(BT_PHONE_READ)						// 读来电话 状态
 314  006e c65010        	ld	a,20496
 315  0071 a501          	bcp	a,#1
 316  0073 270a          	jreq	L151
 317                     ; 105 		if(BT_PhoneTimes<10)				// 消抖
 319  0075 b60a          	ld	a,_BT_PhoneTimes
 320  0077 a10a          	cp	a,#10
 321  0079 240a          	jruge	L551
 322                     ; 107 			BT_PhoneTimes++;
 324  007b 3c0a          	inc	_BT_PhoneTimes
 325  007d 2006          	jra	L551
 326  007f               L151:
 327                     ; 112 		if(BT_PhoneTimes)
 329  007f 3d0a          	tnz	_BT_PhoneTimes
 330  0081 2702          	jreq	L551
 331                     ; 114 			BT_PhoneTimes--;
 333  0083 3a0a          	dec	_BT_PhoneTimes
 334  0085               L551:
 335                     ; 117 	if(READ_CHARGE_STATU)						
 337  0085 c65001        	ld	a,20481
 338  0088 a504          	bcp	a,#4
 339  008a 270c          	jreq	L161
 340                     ; 119 		u8NoChargeTime = 0;
 342  008c 3f00          	clr	_u8NoChargeTime
 343                     ; 120 		if(u8ChargeTimes < 200)				
 345  008e b609          	ld	a,_u8ChargeTimes
 346  0090 a1c8          	cp	a,#200
 347  0092 240e          	jruge	L561
 348                     ; 122 			u8ChargeTimes++;
 350  0094 3c09          	inc	_u8ChargeTimes
 351  0096 200a          	jra	L561
 352  0098               L161:
 353                     ; 127 		if (u8NoChargeTime < 100)
 355  0098 b600          	ld	a,_u8NoChargeTime
 356  009a a164          	cp	a,#100
 357  009c 2402          	jruge	L761
 358                     ; 128 			u8NoChargeTime++;
 360  009e 3c00          	inc	_u8NoChargeTime
 361  00a0               L761:
 362                     ; 129 		u8ChargeTimes = 0;
 364  00a0 3f09          	clr	_u8ChargeTimes
 365  00a2               L561:
 366                     ; 131 }
 369  00a2 81            	ret
 439                     ; 146 void BT_BTSingalHandle(void)
 439                     ; 147 {
 440                     	switch	.text
 441  00a3               _BT_BTSingalHandle:
 443  00a3 5203          	subw	sp,#3
 444       00000003      OFST:	set	3
 447                     ; 149 	u8 m1=0, m2=0;
 449  00a5 0f02          	clr	(OFST-1,sp)
 452  00a7 0f01          	clr	(OFST-2,sp)
 453                     ; 151 	if(Clock[BT_PAIRING_PULSE_CLK]==0)	// 时间到
 455  00a9 be08          	ldw	x,_Clock+8
 456  00ab 2604          	jrne	L532
 457                     ; 153 		BT_PAIR_BUTTON_L;
 459  00ad 7219500f      	bres	20495,#4
 460  00b1               L532:
 461                     ; 155 	if((Clock[BT_ANSWER_SHORT_PULSE_CLK]==1)||(Clock[BT_ANSWER_DOUBLE_PULSE_CLK]==1))	// 时间到
 463  00b1 be0a          	ldw	x,_Clock+10
 464  00b3 a30001        	cpw	x,#1
 465  00b6 2707          	jreq	L142
 467  00b8 be0c          	ldw	x,_Clock+12
 468  00ba a30001        	cpw	x,#1
 469  00bd 260a          	jrne	L732
 470  00bf               L142:
 471                     ; 157 		Clock[BT_ANSWER_SHORT_PULSE_CLK] = 0;
 473  00bf 5f            	clrw	x
 474  00c0 bf0a          	ldw	_Clock+10,x
 475                     ; 158 		Clock[BT_ANSWER_DOUBLE_PULSE_CLK] = 0;
 477  00c2 5f            	clrw	x
 478  00c3 bf0c          	ldw	_Clock+12,x
 479                     ; 159 		BT_ANSWER_BUTTON_L;
 481  00c5 7215500f      	bres	20495,#2
 482  00c9               L732:
 483                     ; 161 	if((Clock[BT_POWER_PULSE_CLK]==0)&&(!IsPowerOn))			// 模式 = Power
 485  00c9 be10          	ldw	x,_Clock+16
 486  00cb 2602          	jrne	L342
 488  00cd 3d00          	tnz	_IsPowerOn
 489  00cf               L342:
 490                     ; 165 	 if ((Clock[BT_ANSWER_HANDLE] >200)&&(KeyTimes==2))
 492  00cf be16          	ldw	x,_Clock+22
 493  00d1 a300c9        	cpw	x,#201
 494  00d4 2516          	jrult	L542
 496  00d6 b600          	ld	a,_KeyTimes
 497  00d8 a102          	cp	a,#2
 498  00da 2610          	jrne	L542
 499                     ; 167 		Clock[BT_ANSWER_DOUBLE_PULSE_CLK] = BT_ANSWER_PULSE_LONG;
 501  00dc ae02ee        	ldw	x,#750
 502  00df bf0c          	ldw	_Clock+12,x
 503                     ; 168 		BT_ANSWER_BUTTON_H;
 505  00e1 7214500f      	bset	20495,#2
 506                     ; 169 		KeyTimes = 0;
 508  00e5 3f00          	clr	_KeyTimes
 509                     ; 170 		Clock[BT_ANSWER_HANDLE] = 0; 
 511  00e7 5f            	clrw	x
 512  00e8 bf16          	ldw	_Clock+22,x
 514  00ea 2015          	jra	L742
 515  00ec               L542:
 516                     ; 173 	if (Clock[BT_ANSWER_HANDLE]==1)
 518  00ec be16          	ldw	x,_Clock+22
 519  00ee a30001        	cpw	x,#1
 520  00f1 260e          	jrne	L742
 521                     ; 175      		Clock[BT_ANSWER_HANDLE]=0;
 523  00f3 5f            	clrw	x
 524  00f4 bf16          	ldw	_Clock+22,x
 525                     ; 176 		Clock[BT_ANSWER_SHORT_PULSE_CLK] = BT_ANSWER_PULSE_MIDDLE;
 527  00f6 ae01fe        	ldw	x,#510
 528  00f9 bf0a          	ldw	_Clock+10,x
 529                     ; 177 		BT_ANSWER_BUTTON_H;
 531  00fb 7214500f      	bset	20495,#2
 532                     ; 178 		KeyTimes = 0;
 534  00ff 3f00          	clr	_KeyTimes
 535  0101               L742:
 536                     ; 181 	if (Clock[BT_VOLMAX_CLK] == 0)
 538  0101 be18          	ldw	x,_Clock+24
 539  0103 2604          	jrne	L352
 540                     ; 182 		BT_VOLMAX_L;
 542  0105 721f500f      	bres	20495,#7
 543  0109               L352:
 544                     ; 183 	if (Clock[BT_LOWBATTERY_CLK] ==1)
 546  0109 be1a          	ldw	x,_Clock+26
 547  010b a30001        	cpw	x,#1
 548  010e 2607          	jrne	L552
 549                     ; 185 		Clock[BT_LOWBATTERY_CLK]  = 0;
 551  0110 5f            	clrw	x
 552  0111 bf1a          	ldw	_Clock+26,x
 553                     ; 186 		BT_LOWBATTERY_L;
 555  0113 7217500f      	bres	20495,#3
 556  0117               L552:
 557                     ; 189 	if ((SourceSelect == BLUETOOTH_AUDIO)&&(IsPowerOn))
 559  0117 b600          	ld	a,_SourceSelect
 560  0119 a102          	cp	a,#2
 561  011b 2649          	jrne	L752
 563  011d 3d00          	tnz	_IsPowerOn
 564  011f 2745          	jreq	L752
 565                     ; 191 		if(BT_PairedTimes>5)
 567  0121 b60b          	ld	a,_BT_PairedTimes
 568  0123 a106          	cp	a,#6
 569  0125 2504          	jrult	L162
 570                     ; 193 			m1 = 0x10;
 572  0127 a610          	ld	a,#16
 573  0129 6b02          	ld	(OFST-1,sp),a
 574  012b               L162:
 575                     ; 195 		if(BT_PairingTimes>5)
 577  012b b60c          	ld	a,_BT_PairingTimes
 578  012d a106          	cp	a,#6
 579  012f 2504          	jrult	L362
 580                     ; 197 			m2 = 0x01;
 582  0131 a601          	ld	a,#1
 583  0133 6b01          	ld	(OFST-2,sp),a
 584  0135               L362:
 585                     ; 199 		tMode = m1 | m2;
 587  0135 7b02          	ld	a,(OFST-1,sp)
 588  0137 1a01          	or	a,(OFST-2,sp)
 589  0139 6b02          	ld	(OFST-1,sp),a
 590                     ; 200 		switch(tMode)
 592  013b 7b02          	ld	a,(OFST-1,sp)
 594                     ; 214 			default:
 594                     ; 215 				break;
 595  013d 4d            	tnz	a
 596  013e 270c          	jreq	L171
 597  0140 4a            	dec	a
 598  0141 270f          	jreq	L371
 599  0143 a00f          	sub	a,#15
 600  0145 2711          	jreq	L571
 601  0147 4a            	dec	a
 602  0148 2714          	jreq	L771
 603  014a 2016          	jra	L762
 604  014c               L171:
 605                     ; 202 			case 0x00:		// 断开
 605                     ; 203 				tLedMode = LED_BLINK_1_1;
 607  014c a604          	ld	a,#4
 608  014e 6b03          	ld	(OFST+0,sp),a
 609                     ; 204 				break;
 611  0150 2010          	jra	L762
 612  0152               L371:
 613                     ; 205 			case 0x01:		// pairing
 613                     ; 206 				tLedMode = LED_BLINK_025_025;
 615  0152 a609          	ld	a,#9
 616  0154 6b03          	ld	(OFST+0,sp),a
 617                     ; 207 				break;
 619  0156 200a          	jra	L762
 620  0158               L571:
 621                     ; 208 			case 0x10:		// paired
 621                     ; 209 				tLedMode = LED_SOLID_ON;
 623  0158 a601          	ld	a,#1
 624  015a 6b03          	ld	(OFST+0,sp),a
 625                     ; 210 				break;
 627  015c 2004          	jra	L762
 628  015e               L771:
 629                     ; 211 			case 0x11:		// 异常
 629                     ; 212 				tLedMode = LED_BLINK_02_02;
 631  015e a606          	ld	a,#6
 632  0160 6b03          	ld	(OFST+0,sp),a
 633                     ; 213 				break;
 635  0162               L102:
 636                     ; 214 			default:
 636                     ; 215 				break;
 638  0162               L762:
 639                     ; 218 			Key_SetBTLedMode(tLedMode);		
 641  0162 7b03          	ld	a,(OFST+0,sp)
 642  0164 ad38          	call	_Key_SetBTLedMode
 644  0166               L752:
 645                     ; 223 		if (IsPowerOn)
 647  0166 3d00          	tnz	_IsPowerOn
 648  0168 2719          	jreq	L172
 649                     ; 225 			if((BT_PhoneTimes>5)&&(SourceSelect == LINE_IN_AUDIO))
 651  016a b60a          	ld	a,_BT_PhoneTimes
 652  016c a106          	cp	a,#6
 653  016e 2513          	jrult	L172
 655  0170 b600          	ld	a,_SourceSelect
 656  0172 a101          	cp	a,#1
 657  0174 260d          	jrne	L172
 658                     ; 227 				SourceSelect = BLUETOOTH_AUDIO;
 660  0176 35020000      	mov	_SourceSelect,#2
 661                     ; 228 				Audio_SetAudioSource(SourceSelect);
 663  017a a602          	ld	a,#2
 664  017c cd0000        	call	_Audio_SetAudioSource
 666                     ; 229 				LINE_IN_LED_OFF;
 668  017f 721b500a      	bres	20490,#5
 669  0183               L172:
 670                     ; 234 }
 673  0183 5b03          	addw	sp,#3
 674  0185 81            	ret
 700                     ; 247 void Key_LedBlinkTask(void)
 700                     ; 248 {
 701                     	switch	.text
 702  0186               _Key_LedBlinkTask:
 706                     ; 249 	MainTask &= ~LED_BLINK_TASK;
 708  0186 be00          	ldw	x,_MainTask
 709  0188 01            	rrwa	x,a
 710  0189 a4ef          	and	a,#239
 711  018b 01            	rrwa	x,a
 712  018c a4ff          	and	a,#255
 713  018e 01            	rrwa	x,a
 714  018f bf00          	ldw	_MainTask,x
 715                     ; 251 	if(SourceSelect == BLUETOOTH_AUDIO) 			// 蓝牙 闪烁
 717  0191 b600          	ld	a,_SourceSelect
 718  0193 a102          	cp	a,#2
 719  0195 2602          	jrne	L503
 720                     ; 253 		BTLedBlink();
 722  0197 ad14          	call	_BTLedBlink
 724  0199               L503:
 725                     ; 256 	if(SourceSelect == LINE_IN_AUDIO) 		// Line-in 闪烁
 727  0199 b600          	ld	a,_SourceSelect
 728  019b a101          	cp	a,#1
 729                     ; 260 }
 732  019d 81            	ret
 768                     ; 273 void Key_SetBTLedMode(u8 mode)
 768                     ; 274 {
 769                     	switch	.text
 770  019e               _Key_SetBTLedMode:
 774                     ; 275 	BTLedMode = mode;
 776  019e b70d          	ld	_BTLedMode,a
 777                     ; 276 	MainTask |= LED_BLINK_TASK;
 779  01a0 b600          	ld	a,_MainTask
 780  01a2 97            	ld	xl,a
 781  01a3 b601          	ld	a,_MainTask+1
 782  01a5 aa10          	or	a,#16
 783  01a7 b701          	ld	_MainTask+1,a
 784  01a9 9f            	ld	a,xl
 785  01aa b700          	ld	_MainTask,a
 786                     ; 277 }
 789  01ac 81            	ret
 843                     ; 290 void BTLedBlink(void)
 843                     ; 291 {
 844                     	switch	.text
 845  01ad               _BTLedBlink:
 847  01ad 5203          	subw	sp,#3
 848       00000003      OFST:	set	3
 851                     ; 296 	if (!IsPowerOn)
 853  01af 3d00          	tnz	_IsPowerOn
 854  01b1 2731          	jreq	L02
 855                     ; 297 		return;
 857                     ; 299 	if(BTLedOn == TRUE)					// 读灭的延时
 859  01b3 b606          	ld	a,_BTLedOn
 860  01b5 a101          	cp	a,#1
 861  01b7 2606          	jrne	L573
 862                     ; 301 		index = 1;
 864  01b9 a601          	ld	a,#1
 865  01bb 6b03          	ld	(OFST+0,sp),a
 867  01bd 2002          	jra	L773
 868  01bf               L573:
 869                     ; 305 		index = 0;						// 读亮的延时
 871  01bf 0f03          	clr	(OFST+0,sp)
 872  01c1               L773:
 873                     ; 308 	switch(BTLedMode)
 875  01c1 b60d          	ld	a,_BTLedMode
 877                     ; 346 			break;
 878  01c3 4a            	dec	a
 879  01c4 271a          	jreq	L723
 880  01c6 4a            	dec	a
 881  01c7 271e          	jreq	L133
 882  01c9 4a            	dec	a
 883  01ca 2721          	jreq	L333
 884  01cc 4a            	dec	a
 885  01cd 272a          	jreq	L533
 886  01cf 4a            	dec	a
 887  01d0 2733          	jreq	L733
 888  01d2 4a            	dec	a
 889  01d3 273c          	jreq	L143
 890  01d5 4a            	dec	a
 891  01d6 2745          	jreq	L343
 892  01d8 4a            	dec	a
 893  01d9 274e          	jreq	L543
 894  01db 4a            	dec	a
 895  01dc 2757          	jreq	L743
 896  01de 205f          	jra	L304
 897  01e0               L723:
 898                     ; 310 		case LED_SOLID_ON:
 898                     ; 311 			BT_LED_ON;
 900  01e0 7218500a      	bset	20490,#4
 901                     ; 312 			return;	
 902  01e4               L02:
 905  01e4 5b03          	addw	sp,#3
 906  01e6 81            	ret
 907  01e7               L133:
 908                     ; 315 		case LED_SOLID_OFF:
 908                     ; 316 			BT_LED_OFF;
 911  01e7 7219500a      	bres	20490,#4
 912                     ; 317 			return;
 914  01eb 20f7          	jra	L02
 915  01ed               L333:
 916                     ; 320 		case LED_BLINK_05_05:
 916                     ; 321 			delay = DELAY_05_05[index];	// 读延时
 919  01ed 7b03          	ld	a,(OFST+0,sp)
 920  01ef 5f            	clrw	x
 921  01f0 97            	ld	xl,a
 922  01f1 58            	sllw	x
 923  01f2 de0000        	ldw	x,(_DELAY_05_05,x)
 924  01f5 1f01          	ldw	(OFST-2,sp),x
 925                     ; 322 			break;
 927  01f7 2046          	jra	L304
 928  01f9               L533:
 929                     ; 324 		case LED_BLINK_1_1:
 929                     ; 325 			delay = DELAY_1_1[index];
 931  01f9 7b03          	ld	a,(OFST+0,sp)
 932  01fb 5f            	clrw	x
 933  01fc 97            	ld	xl,a
 934  01fd 58            	sllw	x
 935  01fe de0000        	ldw	x,(_DELAY_1_1,x)
 936  0201 1f01          	ldw	(OFST-2,sp),x
 937                     ; 326 			break;
 939  0203 203a          	jra	L304
 940  0205               L733:
 941                     ; 328 		case LED_BLINK_1_3:
 941                     ; 329 			delay = DELAY_1_3[index];
 943  0205 7b03          	ld	a,(OFST+0,sp)
 944  0207 5f            	clrw	x
 945  0208 97            	ld	xl,a
 946  0209 58            	sllw	x
 947  020a de0000        	ldw	x,(_DELAY_1_3,x)
 948  020d 1f01          	ldw	(OFST-2,sp),x
 949                     ; 330 			break;
 951  020f 202e          	jra	L304
 952  0211               L143:
 953                     ; 332 		case LED_BLINK_02_02:
 953                     ; 333 			delay = DELAY_02_02[index];
 955  0211 7b03          	ld	a,(OFST+0,sp)
 956  0213 5f            	clrw	x
 957  0214 97            	ld	xl,a
 958  0215 58            	sllw	x
 959  0216 de0000        	ldw	x,(_DELAY_02_02,x)
 960  0219 1f01          	ldw	(OFST-2,sp),x
 961                     ; 334 			break;
 963  021b 2022          	jra	L304
 964  021d               L343:
 965                     ; 336 		case LED_BLINK_02_3:
 965                     ; 337 			delay = DELAY_02_3[index];
 967  021d 7b03          	ld	a,(OFST+0,sp)
 968  021f 5f            	clrw	x
 969  0220 97            	ld	xl,a
 970  0221 58            	sllw	x
 971  0222 de0000        	ldw	x,(_DELAY_02_3,x)
 972  0225 1f01          	ldw	(OFST-2,sp),x
 973                     ; 338 			break;
 975  0227 2016          	jra	L304
 976  0229               L543:
 977                     ; 340 		case LED_BLINK_15_15:
 977                     ; 341 			delay = DELAY_15_15[index];
 979  0229 7b03          	ld	a,(OFST+0,sp)
 980  022b 5f            	clrw	x
 981  022c 97            	ld	xl,a
 982  022d 58            	sllw	x
 983  022e de0000        	ldw	x,(_DELAY_15_15,x)
 984  0231 1f01          	ldw	(OFST-2,sp),x
 985                     ; 342 			break;
 987  0233 200a          	jra	L304
 988  0235               L743:
 989                     ; 344 		case LED_BLINK_025_025:
 989                     ; 345 			delay = DELAY_025_025[index];
 991  0235 7b03          	ld	a,(OFST+0,sp)
 992  0237 5f            	clrw	x
 993  0238 97            	ld	xl,a
 994  0239 58            	sllw	x
 995  023a de0000        	ldw	x,(_DELAY_025_025,x)
 996  023d 1f01          	ldw	(OFST-2,sp),x
 997                     ; 346 			break;
 999  023f               L304:
1000                     ; 351 		if (Clock[BT_LED_BLINK_CLK] == 0)
1002  023f be06          	ldw	x,_Clock+6
1003  0241 261a          	jrne	L504
1004                     ; 353 			if(BTLedOn == TRUE)				// 闪烁 LED
1006  0243 b606          	ld	a,_BTLedOn
1007  0245 a101          	cp	a,#1
1008  0247 2608          	jrne	L704
1009                     ; 355 				BT_LED_OFF;
1011  0249 7219500a      	bres	20490,#4
1012                     ; 356 				BTLedOn = FALSE;
1014  024d 3f06          	clr	_BTLedOn
1016  024f 2008          	jra	L114
1017  0251               L704:
1018                     ; 360 				BT_LED_ON;
1020  0251 7218500a      	bset	20490,#4
1021                     ; 361 				BTLedOn = TRUE;
1023  0255 35010006      	mov	_BTLedOn,#1
1024  0259               L114:
1025                     ; 363 			Clock[BT_LED_BLINK_CLK] = delay;	// 置下次 LED 转换时间 (0:表示不闪烁)
1027  0259 1e01          	ldw	x,(OFST-2,sp)
1028  025b bf06          	ldw	_Clock+6,x
1029  025d               L504:
1030                     ; 367 }
1032  025d 2085          	jra	L02
1066                     ; 379 void ChargeSingalHandle(void)
1066                     ; 380 { 
1067                     	switch	.text
1068  025f               _ChargeSingalHandle:
1072                     ; 382 	BatteryNTC_Value = GetBatteryADValue(BATTERY_NTC_CHECK_ADLine);
1074  025f a603          	ld	a,#3
1075  0261 cd0347        	call	_GetBatteryADValue
1077  0264 bf03          	ldw	_BatteryNTC_Value,x
1078                     ; 383 	Battery_Value = GetBatteryADValue(BATTERY_CHECK_ADLine);
1080  0266 a602          	ld	a,#2
1081  0268 cd0347        	call	_GetBatteryADValue
1083  026b bf01          	ldw	_Battery_Value,x
1084                     ; 384 	if ( Battery_Value < LOW_BATTERY_POWEROFF)
1086  026d be01          	ldw	x,_Battery_Value
1087  026f a302aa        	cpw	x,#682
1088  0272 240e          	jruge	L324
1089                     ; 386 		MainTask |= SYSTEM_POWEROFF_TASK;
1091  0274 b600          	ld	a,_MainTask
1092  0276 97            	ld	xl,a
1093  0277 b601          	ld	a,_MainTask+1
1094  0279 aa02          	or	a,#2
1095  027b b701          	ld	_MainTask+1,a
1096  027d 9f            	ld	a,xl
1097  027e b700          	ld	_MainTask,a
1099  0280 2018          	jra	L524
1100  0282               L324:
1101                     ; 388 	else if (( Battery_Value < LOW_BATTERY_VALUE)&&(!bLowBattery))
1103  0282 be01          	ldw	x,_Battery_Value
1104  0284 a3033e        	cpw	x,#830
1105  0287 2411          	jruge	L524
1107  0289 3d07          	tnz	_bLowBattery
1108  028b 260d          	jrne	L524
1109                     ; 390 		bLowBattery = TRUE;
1111  028d 35010007      	mov	_bLowBattery,#1
1112                     ; 391 		BT_LOWBATTERY_H;
1114  0291 7216500f      	bset	20495,#3
1115                     ; 392 		Clock[BT_LOWBATTERY_CLK] = 2000;
1117  0295 ae07d0        	ldw	x,#2000
1118  0298 bf1a          	ldw	_Clock+26,x
1119  029a               L524:
1120                     ; 394 	if ((u8ChargeTimes >= 190)||(u8ChargeTimes ==0&&u8NoChargeTime >90))
1122  029a b609          	ld	a,_u8ChargeTimes
1123  029c a1be          	cp	a,#190
1124  029e 240a          	jruge	L334
1126  02a0 3d09          	tnz	_u8ChargeTimes
1127  02a2 262c          	jrne	L134
1129  02a4 b600          	ld	a,_u8NoChargeTime
1130  02a6 a15b          	cp	a,#91
1131  02a8 2526          	jrult	L134
1132  02aa               L334:
1133                     ; 396 		bCharging = FALSE;
1135  02aa 3f08          	clr	_bCharging
1136                     ; 397 		if (IsPowerOn)
1138  02ac 3d00          	tnz	_IsPowerOn
1139  02ae 2706          	jreq	L534
1140                     ; 398 			POWER_LED_ON;
1142  02b0 721e500a      	bset	20490,#7
1144  02b4 2004          	jra	L734
1145  02b6               L534:
1146                     ; 400 			POWER_LED_OFF;
1148  02b6 721f500a      	bres	20490,#7
1149  02ba               L734:
1150                     ; 401 		ChargeLedBlink(LED_SOLID_OFF);
1152  02ba a602          	ld	a,#2
1153  02bc ad33          	call	_ChargeLedBlink
1155                     ; 402 		if (!IsPowerOn)
1157  02be 3d00          	tnz	_IsPowerOn
1158  02c0 262e          	jrne	L344
1159                     ; 404 			MainTask |= SYSTEM_POWEROFF_TASK;
1161  02c2 b600          	ld	a,_MainTask
1162  02c4 97            	ld	xl,a
1163  02c5 b601          	ld	a,_MainTask+1
1164  02c7 aa02          	or	a,#2
1165  02c9 b701          	ld	_MainTask+1,a
1166  02cb 9f            	ld	a,xl
1167  02cc b700          	ld	_MainTask,a
1168  02ce 2020          	jra	L344
1169  02d0               L134:
1170                     ; 407 	else if (u8ChargeTimes > 10)
1172  02d0 b609          	ld	a,_u8ChargeTimes
1173  02d2 a10b          	cp	a,#11
1174  02d4 251a          	jrult	L344
1175                     ; 409 		bLowBattery = TRUE;
1177  02d6 35010007      	mov	_bLowBattery,#1
1178                     ; 410 		bCharging = TRUE;
1180  02da 35010008      	mov	_bCharging,#1
1181                     ; 411 		POWER_LED_OFF;
1183  02de 721f500a      	bres	20490,#7
1184                     ; 412 		if (IsPowerOn)
1186  02e2 3d00          	tnz	_IsPowerOn
1187  02e4 2706          	jreq	L744
1188                     ; 413 			ChargeLedBlink(LED_SOLID_ON);
1190  02e6 a601          	ld	a,#1
1191  02e8 ad07          	call	_ChargeLedBlink
1194  02ea 2004          	jra	L344
1195  02ec               L744:
1196                     ; 415 			ChargeLedBlink(LED_BLINK_1_1);
1198  02ec a604          	ld	a,#4
1199  02ee ad01          	call	_ChargeLedBlink
1201  02f0               L344:
1202                     ; 423 }
1205  02f0 81            	ret
1260                     ; 437 void ChargeLedBlink(u8 mode)
1260                     ; 438 {
1261                     	switch	.text
1262  02f1               _ChargeLedBlink:
1264  02f1 88            	push	a
1265  02f2 5203          	subw	sp,#3
1266       00000003      OFST:	set	3
1269                     ; 442 	if(bRedLedOn == TRUE)					// 读灭的延时
1271  02f4 b605          	ld	a,_bRedLedOn
1272  02f6 a101          	cp	a,#1
1273  02f8 2606          	jrne	L705
1274                     ; 444 		index = 1;
1276  02fa a601          	ld	a,#1
1277  02fc 6b03          	ld	(OFST+0,sp),a
1279  02fe 2002          	jra	L115
1280  0300               L705:
1281                     ; 448 		index = 0;						// 读亮的延时
1283  0300 0f03          	clr	(OFST+0,sp)
1284  0302               L115:
1285                     ; 451 	switch(mode)
1287  0302 7b04          	ld	a,(OFST+1,sp)
1289                     ; 463 			break;
1290  0304 4a            	dec	a
1291  0305 2709          	jreq	L354
1292  0307 4a            	dec	a
1293  0308 270c          	jreq	L554
1294  030a a002          	sub	a,#2
1295  030c 270f          	jreq	L754
1296  030e 2017          	jra	L515
1297  0310               L354:
1298                     ; 453 		case LED_SOLID_ON:
1298                     ; 454 			RED_LED_ON;
1300  0310 721c500a      	bset	20490,#6
1301                     ; 455 			return;
1303  0314 2004          	jra	L62
1304  0316               L554:
1305                     ; 457 		case LED_SOLID_OFF:
1305                     ; 458 			RED_LED_OFF;
1308  0316 721d500a      	bres	20490,#6
1309                     ; 459 			return;
1310  031a               L62:
1313  031a 5b04          	addw	sp,#4
1314  031c 81            	ret
1315  031d               L754:
1316                     ; 461 		case LED_BLINK_1_1:
1316                     ; 462 			delay = DELAY_1_1[index];
1319  031d 7b03          	ld	a,(OFST+0,sp)
1320  031f 5f            	clrw	x
1321  0320 97            	ld	xl,a
1322  0321 58            	sllw	x
1323  0322 de0000        	ldw	x,(_DELAY_1_1,x)
1324  0325 1f01          	ldw	(OFST-2,sp),x
1325                     ; 463 			break;
1327  0327               L515:
1328                     ; 466 	if(Clock[CHARGE_LED_BLINK_CLK] ==0)
1330  0327 be14          	ldw	x,_Clock+20
1331  0329 261a          	jrne	L715
1332                     ; 468 		if(bRedLedOn == TRUE)				// 闪烁 LED
1334  032b b605          	ld	a,_bRedLedOn
1335  032d a101          	cp	a,#1
1336  032f 2608          	jrne	L125
1337                     ; 470 			RED_LED_OFF;
1339  0331 721d500a      	bres	20490,#6
1340                     ; 471 			bRedLedOn = FALSE;
1342  0335 3f05          	clr	_bRedLedOn
1344  0337 2008          	jra	L325
1345  0339               L125:
1346                     ; 475 			RED_LED_ON;
1348  0339 721c500a      	bset	20490,#6
1349                     ; 476 			bRedLedOn = TRUE;
1351  033d 35010005      	mov	_bRedLedOn,#1
1352  0341               L325:
1353                     ; 478 		Clock[CHARGE_LED_BLINK_CLK] = delay;	// 置下次 LED 转换时间 (0:表示不闪烁)
1355  0341 1e01          	ldw	x,(OFST-2,sp)
1356  0343 bf14          	ldw	_Clock+20,x
1357  0345               L715:
1358                     ; 481 }
1360  0345 20d3          	jra	L62
1450                     ; 494 u16 GetBatteryADValue(u8 adline)
1450                     ; 495 {
1451                     	switch	.text
1452  0347               _GetBatteryADValue:
1454  0347 88            	push	a
1455  0348 5213          	subw	sp,#19
1456       00000013      OFST:	set	19
1459                     ; 497 	u16 u16Temp = 0;
1461  034a 5f            	clrw	x
1462  034b 1f03          	ldw	(OFST-16,sp),x
1463                     ; 500 	u16 max=0;
1465  034d 5f            	clrw	x
1466  034e 1f05          	ldw	(OFST-14,sp),x
1467                     ; 501 	u16 min=1000;
1469  0350 ae03e8        	ldw	x,#1000
1470  0353 1f07          	ldw	(OFST-12,sp),x
1471                     ; 503 	for(i=0; i<5; i++)							
1473  0355 0f13          	clr	(OFST+0,sp)
1474  0357               L375:
1475                     ; 505 		temp[i] = ADC_GetADValue(adline);					
1477  0357 7b14          	ld	a,(OFST+1,sp)
1478  0359 cd0000        	call	_ADC_GetADValue
1480  035c 9096          	ldw	y,sp
1481  035e 72a90009      	addw	y,#OFST-10
1482  0362 1701          	ldw	(OFST-18,sp),y
1483  0364 7b13          	ld	a,(OFST+0,sp)
1484  0366 905f          	clrw	y
1485  0368 9097          	ld	yl,a
1486  036a 9058          	sllw	y
1487  036c 72f901        	addw	y,(OFST-18,sp)
1488  036f 90ff          	ldw	(y),x
1489                     ; 506 		u16Temp += temp[i];						
1491  0371 96            	ldw	x,sp
1492  0372 1c0009        	addw	x,#OFST-10
1493  0375 1f01          	ldw	(OFST-18,sp),x
1494  0377 7b13          	ld	a,(OFST+0,sp)
1495  0379 5f            	clrw	x
1496  037a 97            	ld	xl,a
1497  037b 58            	sllw	x
1498  037c 72fb01        	addw	x,(OFST-18,sp)
1499  037f fe            	ldw	x,(x)
1500  0380 72fb03        	addw	x,(OFST-16,sp)
1501  0383 1f03          	ldw	(OFST-16,sp),x
1502                     ; 508 		if(temp[i] < min)
1504  0385 96            	ldw	x,sp
1505  0386 1c0009        	addw	x,#OFST-10
1506  0389 1f01          	ldw	(OFST-18,sp),x
1507  038b 7b13          	ld	a,(OFST+0,sp)
1508  038d 5f            	clrw	x
1509  038e 97            	ld	xl,a
1510  038f 58            	sllw	x
1511  0390 72fb01        	addw	x,(OFST-18,sp)
1512  0393 9093          	ldw	y,x
1513  0395 51            	exgw	x,y
1514  0396 fe            	ldw	x,(x)
1515  0397 1307          	cpw	x,(OFST-12,sp)
1516  0399 51            	exgw	x,y
1517  039a 2411          	jruge	L106
1518                     ; 510 			min = temp[i]; 
1520  039c 96            	ldw	x,sp
1521  039d 1c0009        	addw	x,#OFST-10
1522  03a0 1f01          	ldw	(OFST-18,sp),x
1523  03a2 7b13          	ld	a,(OFST+0,sp)
1524  03a4 5f            	clrw	x
1525  03a5 97            	ld	xl,a
1526  03a6 58            	sllw	x
1527  03a7 72fb01        	addw	x,(OFST-18,sp)
1528  03aa fe            	ldw	x,(x)
1529  03ab 1f07          	ldw	(OFST-12,sp),x
1530  03ad               L106:
1531                     ; 512 		if(temp[i] > max)
1533  03ad 96            	ldw	x,sp
1534  03ae 1c0009        	addw	x,#OFST-10
1535  03b1 1f01          	ldw	(OFST-18,sp),x
1536  03b3 7b13          	ld	a,(OFST+0,sp)
1537  03b5 5f            	clrw	x
1538  03b6 97            	ld	xl,a
1539  03b7 58            	sllw	x
1540  03b8 72fb01        	addw	x,(OFST-18,sp)
1541  03bb 9093          	ldw	y,x
1542  03bd 51            	exgw	x,y
1543  03be fe            	ldw	x,(x)
1544  03bf 1305          	cpw	x,(OFST-14,sp)
1545  03c1 51            	exgw	x,y
1546  03c2 2311          	jrule	L306
1547                     ; 514 			max = temp[i];
1549  03c4 96            	ldw	x,sp
1550  03c5 1c0009        	addw	x,#OFST-10
1551  03c8 1f01          	ldw	(OFST-18,sp),x
1552  03ca 7b13          	ld	a,(OFST+0,sp)
1553  03cc 5f            	clrw	x
1554  03cd 97            	ld	xl,a
1555  03ce 58            	sllw	x
1556  03cf 72fb01        	addw	x,(OFST-18,sp)
1557  03d2 fe            	ldw	x,(x)
1558  03d3 1f05          	ldw	(OFST-14,sp),x
1559  03d5               L306:
1560                     ; 503 	for(i=0; i<5; i++)							
1562  03d5 0c13          	inc	(OFST+0,sp)
1565  03d7 7b13          	ld	a,(OFST+0,sp)
1566  03d9 a105          	cp	a,#5
1567  03db 2403cc0357    	jrult	L375
1568                     ; 517 	AD_Value = ((u16Temp - min - max)/3);		// 减去最大和最小值，在去平均值
1570  03e0 1e03          	ldw	x,(OFST-16,sp)
1571  03e2 72f007        	subw	x,(OFST-12,sp)
1572  03e5 72f005        	subw	x,(OFST-14,sp)
1573  03e8 a603          	ld	a,#3
1574  03ea 62            	div	x,a
1575  03eb 1f03          	ldw	(OFST-16,sp),x
1576                     ; 519 	return AD_Value;
1578  03ed 1e03          	ldw	x,(OFST-16,sp)
1581  03ef 5b14          	addw	sp,#20
1582  03f1 81            	ret
1682                     	xdef	_ChargeLedBlink
1683                     	xdef	_BTLedBlink
1684                     	xdef	_GetBatteryADValue
1685                     	xdef	_Key_SetBTLedMode
1686                     	switch	.ubsct
1687  0000               _u8NoChargeTime:
1688  0000 00            	ds.b	1
1689                     	xdef	_u8NoChargeTime
1690  0001               _Battery_Value:
1691  0001 0000          	ds.b	2
1692                     	xdef	_Battery_Value
1693  0003               _BatteryNTC_Value:
1694  0003 0000          	ds.b	2
1695                     	xdef	_BatteryNTC_Value
1696  0005               _bRedLedOn:
1697  0005 00            	ds.b	1
1698                     	xdef	_bRedLedOn
1699  0006               _BTLedOn:
1700  0006 00            	ds.b	1
1701                     	xdef	_BTLedOn
1702                     	xdef	_KeyTimes
1703                     	xref	_Audio_SetAudioSource
1704                     	xref.b	_SourceSelect
1705                     	xdef	_ChargeSingalHandle
1706                     	xdef	_BluetoothHandle
1707                     	xdef	_BT_BTSingalHandle
1708                     	xdef	_BT_GetBluetoothSignal
1709                     	xdef	_Key_LedBlinkTask
1710  0007               _bLowBattery:
1711  0007 00            	ds.b	1
1712                     	xdef	_bLowBattery
1713  0008               _bCharging:
1714  0008 00            	ds.b	1
1715                     	xdef	_bCharging
1716  0009               _u8ChargeTimes:
1717  0009 00            	ds.b	1
1718                     	xdef	_u8ChargeTimes
1719  000a               _BT_PhoneTimes:
1720  000a 00            	ds.b	1
1721                     	xdef	_BT_PhoneTimes
1722  000b               _BT_PairedTimes:
1723  000b 00            	ds.b	1
1724                     	xdef	_BT_PairedTimes
1725  000c               _BT_PairingTimes:
1726  000c 00            	ds.b	1
1727                     	xdef	_BT_PairingTimes
1728  000d               _BTLedMode:
1729  000d 00            	ds.b	1
1730                     	xdef	_BTLedMode
1731                     	xref	_Key_GetCode
1732                     	xref	_DELAY_15_15
1733                     	xref	_DELAY_02_3
1734                     	xref	_DELAY_025_025
1735                     	xref	_DELAY_02_02
1736                     	xref	_DELAY_1_3
1737                     	xref	_DELAY_1_1
1738                     	xref	_DELAY_05_05
1739                     	xref.b	_IsPowerOn
1740                     	xref.b	_MainTask
1741                     	xref	_ADC_GetADValue
1742                     	xref.b	_Clock
1762                     	end
