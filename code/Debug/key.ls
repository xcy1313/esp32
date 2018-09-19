   1                     ; C Compiler for STM8 (COSMIC Software)
   2                     ; Generator V4.2.8 - 03 Dec 2008
   4                     .const:	section	.text
   5  0000               _DELAY_0_0:
   6  0000 0000          	dc.w	0
   7  0002 0000          	dc.w	0
   8  0004               _DELAY_05_05:
   9  0004 01f4          	dc.w	500
  10  0006 01f4          	dc.w	500
  11  0008               _DELAY_1_1:
  12  0008 03e8          	dc.w	1000
  13  000a 03e8          	dc.w	1000
  14  000c               _DELAY_1_3:
  15  000c 03e8          	dc.w	1000
  16  000e 0bb8          	dc.w	3000
  17  0010               _DELAY_02_02:
  18  0010 00c8          	dc.w	200
  19  0012 00c8          	dc.w	200
  20  0014               _DELAY_025_025:
  21  0014 00fa          	dc.w	250
  22  0016 00fa          	dc.w	250
  23  0018               _DELAY_02_3:
  24  0018 0064          	dc.w	100
  25  001a 0bb8          	dc.w	3000
  26  001c               _DELAY_15_15:
  27  001c 05dc          	dc.w	1500
  28  001e 05dc          	dc.w	1500
 138                     	switch	.const
 139  0020               _KEY_MAP_TAB:
 140  0020 01            	dc.b	1
 141  0021 04            	dc.b	4
 142  0022 00            	dc.b	0
 143  0023 00            	dc.b	0
 144  0024 00            	dc.b	0
 145  0025 03            	dc.b	3
 146  0026 02            	dc.b	2
 147  0027 05            	dc.b	5
 148  0028 06            	dc.b	6
 149  0029 00            	dc.b	0
 150  002a               _KEY_AD_TAB:
 151  002a 0032          	dc.w	50
 152  002c 00fa          	dc.w	250
 153  002e 01ae          	dc.w	430
 154  0030 0258          	dc.w	600
 155  0032 03e8          	dc.w	1000
 156                     	bsct
 157  0000               _ADChannel:
 158  0000 01            	dc.b	1
 159  0001 00            	dc.b	0
 189                     ; 43 u8  Key_GetCode(void)
 189                     ; 44 {
 191                     	switch	.text
 192  0000               _Key_GetCode:
 196                     ; 45 	return u8KeyCode;
 198  0000 b602          	ld	a,_u8KeyCode
 201  0002 81            	ret
 248                     ; 59 void Key_Scan(void)
 248                     ; 60 {	
 249                     	switch	.text
 250  0003               _Key_Scan:
 252  0003 89            	pushw	x
 253       00000002      OFST:	set	2
 256                     ; 63 	MainTask &= ~KEY_SCAN_TASK;
 258  0004 be00          	ldw	x,_MainTask
 259  0006 01            	rrwa	x,a
 260  0007 a4fe          	and	a,#254
 261  0009 01            	rrwa	x,a
 262  000a a4ff          	and	a,#255
 263  000c 01            	rrwa	x,a
 264  000d bf00          	ldw	_MainTask,x
 265                     ; 64 	Clock[KEY_SCAN_CLK] = KEY_SCAN_FREQ;
 267  000f ae000a        	ldw	x,#10
 268  0012 bf00          	ldw	_Clock,x
 269                     ; 65 	for (i = 0;i<2;i++)
 271  0014 0f02          	clr	(OFST+0,sp)
 272  0016               L711:
 273                     ; 67 		KeyCode = Key_GetADKey(i);
 275  0016 7b02          	ld	a,(OFST+0,sp)
 276  0018 cd018a        	call	_Key_GetADKey
 278  001b 6b01          	ld	(OFST-1,sp),a
 279                     ; 68 		Key_LongShortProcess(i,KeyCode);
 281  001d 7b01          	ld	a,(OFST-1,sp)
 282  001f 97            	ld	xl,a
 283  0020 7b02          	ld	a,(OFST+0,sp)
 284  0022 95            	ld	xh,a
 285  0023 ad0a          	call	_Key_LongShortProcess
 287                     ; 65 	for (i = 0;i<2;i++)
 289  0025 0c02          	inc	(OFST+0,sp)
 292  0027 7b02          	ld	a,(OFST+0,sp)
 293  0029 a102          	cp	a,#2
 294  002b 25e9          	jrult	L711
 295                     ; 70 }
 298  002d 85            	popw	x
 299  002e 81            	ret
 350                     ; 83 void Key_LongShortProcess(u8 u8ScanType, u8 u8key)
 350                     ; 84 {
 351                     	switch	.text
 352  002f               _Key_LongShortProcess:
 354  002f 89            	pushw	x
 355       00000000      OFST:	set	0
 358                     ; 86 	if (u8key)
 360  0030 9f            	ld	a,xl
 361  0031 4d            	tnz	a
 362  0032 2603          	jrne	L41
 363  0034 cc00f3        	jp	L741
 364  0037               L41:
 365                     ; 88 		scnCnt[u8ScanType]++;//adjust dither
 367  0037 9e            	ld	a,xh
 368  0038 5f            	clrw	x
 369  0039 97            	ld	xl,a
 370  003a 6c0c          	inc	(_scnCnt,x)
 371                     ; 89 		if(scnCnt[u8ScanType] == 0xFF)
 373  003c 7b01          	ld	a,(OFST+1,sp)
 374  003e 5f            	clrw	x
 375  003f 97            	ld	xl,a
 376  0040 e60c          	ld	a,(_scnCnt,x)
 377  0042 a1ff          	cp	a,#255
 378  0044 2606          	jrne	L151
 379                     ; 90 			scnCnt[u8ScanType]--;
 381  0046 7b01          	ld	a,(OFST+1,sp)
 382  0048 5f            	clrw	x
 383  0049 97            	ld	xl,a
 384  004a 6a0c          	dec	(_scnCnt,x)
 385  004c               L151:
 386                     ; 91 		if (u8key == u8TempKeyCode[u8ScanType])
 388  004c 7b01          	ld	a,(OFST+1,sp)
 389  004e 5f            	clrw	x
 390  004f 97            	ld	xl,a
 391  0050 e604          	ld	a,(_u8TempKeyCode,x)
 392  0052 1102          	cp	a,(OFST+2,sp)
 393  0054 2703cc00df    	jrne	L351
 394                     ; 93 			if (scnCnt[u8ScanType] > DITHERTIME &&!bKeyActive[u8ScanType])
 396  0059 7b01          	ld	a,(OFST+1,sp)
 397  005b 5f            	clrw	x
 398  005c 97            	ld	xl,a
 399  005d e60c          	ld	a,(_scnCnt,x)
 400  005f a108          	cp	a,#8
 401  0061 2510          	jrult	L551
 403  0063 7b01          	ld	a,(OFST+1,sp)
 404  0065 5f            	clrw	x
 405  0066 97            	ld	xl,a
 406  0067 6d06          	tnz	(_bKeyActive,x)
 407  0069 2608          	jrne	L551
 408                     ; 94 				bKeyActive[u8ScanType] = TRUE;
 410  006b 7b01          	ld	a,(OFST+1,sp)
 411  006d 5f            	clrw	x
 412  006e 97            	ld	xl,a
 413  006f a601          	ld	a,#1
 414  0071 e706          	ld	(_bKeyActive,x),a
 415  0073               L551:
 416                     ; 95 			if (bKeyActive[u8ScanType])
 418  0073 7b01          	ld	a,(OFST+1,sp)
 419  0075 5f            	clrw	x
 420  0076 97            	ld	xl,a
 421  0077 6d06          	tnz	(_bKeyActive,x)
 422  0079 2776          	jreq	L761
 423                     ; 97 				u16KeyScanCount[u8ScanType]++;
 425  007b 7b01          	ld	a,(OFST+1,sp)
 426  007d 5f            	clrw	x
 427  007e 97            	ld	xl,a
 428  007f 58            	sllw	x
 429  0080 9093          	ldw	y,x
 430  0082 ee08          	ldw	x,(_u16KeyScanCount,x)
 431  0084 1c0001        	addw	x,#1
 432  0087 90ef08        	ldw	(_u16KeyScanCount,y),x
 433                     ; 98 				if (u16KeyScanCount[u8ScanType] == 0xffff)
 435  008a 7b01          	ld	a,(OFST+1,sp)
 436  008c 5f            	clrw	x
 437  008d 97            	ld	xl,a
 438  008e 58            	sllw	x
 439  008f 9093          	ldw	y,x
 440  0091 90ee08        	ldw	y,(_u16KeyScanCount,y)
 441  0094 90a3ffff      	cpw	y,#65535
 442  0098 260f          	jrne	L161
 443                     ; 99 					u16KeyScanCount[u8ScanType] --;
 445  009a 7b01          	ld	a,(OFST+1,sp)
 446  009c 5f            	clrw	x
 447  009d 97            	ld	xl,a
 448  009e 58            	sllw	x
 449  009f 9093          	ldw	y,x
 450  00a1 ee08          	ldw	x,(_u16KeyScanCount,x)
 451  00a3 1d0001        	subw	x,#1
 452  00a6 90ef08        	ldw	(_u16KeyScanCount,y),x
 453  00a9               L161:
 454                     ; 100 				if (u16KeyScanCount[u8ScanType] >= LONGTIME)
 456  00a9 7b01          	ld	a,(OFST+1,sp)
 457  00ab 5f            	clrw	x
 458  00ac 97            	ld	xl,a
 459  00ad 58            	sllw	x
 460  00ae 9093          	ldw	y,x
 461  00b0 90ee08        	ldw	y,(_u16KeyScanCount,y)
 462  00b3 90a30064      	cpw	y,#100
 463  00b7 2538          	jrult	L761
 464                     ; 102 					u8KeyCode = u8TempKeyCode[u8ScanType] | KEY_LP;
 466  00b9 7b01          	ld	a,(OFST+1,sp)
 467  00bb 5f            	clrw	x
 468  00bc 97            	ld	xl,a
 469  00bd e604          	ld	a,(_u8TempKeyCode,x)
 470  00bf aa40          	or	a,#64
 471  00c1 b702          	ld	_u8KeyCode,a
 472                     ; 103 					bKeyActive[u8ScanType] = TRUE;
 474  00c3 7b01          	ld	a,(OFST+1,sp)
 475  00c5 5f            	clrw	x
 476  00c6 97            	ld	xl,a
 477  00c7 a601          	ld	a,#1
 478  00c9 e706          	ld	(_bKeyActive,x),a
 479                     ; 104 					scnCnt[u8ScanType] = 0;
 481  00cb 7b01          	ld	a,(OFST+1,sp)
 482  00cd 5f            	clrw	x
 483  00ce 97            	ld	xl,a
 484  00cf 6f0c          	clr	(_scnCnt,x)
 485                     ; 105 					MainTask |= KEY_HANDLE_TASK;
 487  00d1 b600          	ld	a,_MainTask
 488  00d3 97            	ld	xl,a
 489  00d4 b601          	ld	a,_MainTask+1
 490  00d6 aa20          	or	a,#32
 491  00d8 b701          	ld	_MainTask+1,a
 492  00da 9f            	ld	a,xl
 493  00db b700          	ld	_MainTask,a
 494  00dd 2012          	jra	L761
 495  00df               L351:
 496                     ; 111 			bMaxVolume = TRUE;
 498  00df 35010000      	mov	_bMaxVolume,#1
 499                     ; 112 			u8TempKeyCode[u8ScanType] = u8key;
 501  00e3 7b01          	ld	a,(OFST+1,sp)
 502  00e5 5f            	clrw	x
 503  00e6 97            	ld	xl,a
 504  00e7 7b02          	ld	a,(OFST+2,sp)
 505  00e9 e704          	ld	(_u8TempKeyCode,x),a
 506                     ; 113 			scnCnt[u8ScanType] = 0;
 508  00eb 7b01          	ld	a,(OFST+1,sp)
 509  00ed 5f            	clrw	x
 510  00ee 97            	ld	xl,a
 511  00ef 6f0c          	clr	(_scnCnt,x)
 512  00f1               L761:
 513                     ; 150 }
 515  00f1 2035          	jra	L21
 516  00f3               L741:
 517                     ; 119 		if (bKeyActive[u8ScanType])
 519  00f3 7b01          	ld	a,(OFST+1,sp)
 520  00f5 5f            	clrw	x
 521  00f6 97            	ld	xl,a
 522  00f7 6d06          	tnz	(_bKeyActive,x)
 523  00f9 2760          	jreq	L171
 524                     ; 122 			if (u16KeyScanCount[u8ScanType] >= LONGTIME)
 526  00fb 7b01          	ld	a,(OFST+1,sp)
 527  00fd 5f            	clrw	x
 528  00fe 97            	ld	xl,a
 529  00ff 58            	sllw	x
 530  0100 9093          	ldw	y,x
 531  0102 90ee08        	ldw	y,(_u16KeyScanCount,y)
 532  0105 90a30064      	cpw	y,#100
 533  0109 251f          	jrult	L371
 534                     ; 124 				u8KeyCode= KEY_NONE;
 536  010b 3f02          	clr	_u8KeyCode
 537                     ; 125 				u16KeyScanCount[u8ScanType] = 0;
 539  010d 7b01          	ld	a,(OFST+1,sp)
 540  010f 5f            	clrw	x
 541  0110 97            	ld	xl,a
 542  0111 58            	sllw	x
 543  0112 905f          	clrw	y
 544  0114 ef08          	ldw	(_u16KeyScanCount,x),y
 545                     ; 126 				bKeyActive[u8ScanType] = FALSE;
 547  0116 7b01          	ld	a,(OFST+1,sp)
 548  0118 5f            	clrw	x
 549  0119 97            	ld	xl,a
 550  011a 6f06          	clr	(_bKeyActive,x)
 551                     ; 127 				scnCnt[u8ScanType] = 0;
 553  011c 7b01          	ld	a,(OFST+1,sp)
 554  011e 5f            	clrw	x
 555  011f 97            	ld	xl,a
 556  0120 6f0c          	clr	(_scnCnt,x)
 557                     ; 128 				u8TempKeyCode[u8ScanType] = KEY_NONE;
 559  0122 7b01          	ld	a,(OFST+1,sp)
 560  0124 5f            	clrw	x
 561  0125 97            	ld	xl,a
 562  0126 6f04          	clr	(_u8TempKeyCode,x)
 563                     ; 129 				return;
 564  0128               L21:
 567  0128 85            	popw	x
 568  0129 81            	ret
 569  012a               L371:
 570                     ; 133 				u8KeyCode = u8TempKeyCode[u8ScanType];
 572  012a 7b01          	ld	a,(OFST+1,sp)
 573  012c 5f            	clrw	x
 574  012d 97            	ld	xl,a
 575  012e e604          	ld	a,(_u8TempKeyCode,x)
 576  0130 b702          	ld	_u8KeyCode,a
 577                     ; 134 				u16KeyScanCount[u8ScanType] = 0;
 579  0132 7b01          	ld	a,(OFST+1,sp)
 580  0134 5f            	clrw	x
 581  0135 97            	ld	xl,a
 582  0136 58            	sllw	x
 583  0137 905f          	clrw	y
 584  0139 ef08          	ldw	(_u16KeyScanCount,x),y
 585                     ; 135 				bKeyActive[u8ScanType] = FALSE;
 587  013b 7b01          	ld	a,(OFST+1,sp)
 588  013d 5f            	clrw	x
 589  013e 97            	ld	xl,a
 590  013f 6f06          	clr	(_bKeyActive,x)
 591                     ; 136 				scnCnt[u8ScanType] = 0;
 593  0141 7b01          	ld	a,(OFST+1,sp)
 594  0143 5f            	clrw	x
 595  0144 97            	ld	xl,a
 596  0145 6f0c          	clr	(_scnCnt,x)
 597                     ; 137 				u8TempKeyCode[u8ScanType] = KEY_NONE;
 599  0147 7b01          	ld	a,(OFST+1,sp)
 600  0149 5f            	clrw	x
 601  014a 97            	ld	xl,a
 602  014b 6f04          	clr	(_u8TempKeyCode,x)
 603                     ; 138 				MainTask |= KEY_HANDLE_TASK;
 605  014d b600          	ld	a,_MainTask
 606  014f 97            	ld	xl,a
 607  0150 b601          	ld	a,_MainTask+1
 608  0152 aa20          	or	a,#32
 609  0154 b701          	ld	_MainTask+1,a
 610  0156 9f            	ld	a,xl
 611  0157 b700          	ld	_MainTask,a
 612                     ; 139 				return;
 614  0159 20cd          	jra	L21
 615  015b               L171:
 616                     ; 142 		bKeyActive[u8ScanType]  = FALSE;
 618  015b 7b01          	ld	a,(OFST+1,sp)
 619  015d 5f            	clrw	x
 620  015e 97            	ld	xl,a
 621  015f 6f06          	clr	(_bKeyActive,x)
 622                     ; 143 		scnCnt[u8ScanType] = 0;
 624  0161 7b01          	ld	a,(OFST+1,sp)
 625  0163 5f            	clrw	x
 626  0164 97            	ld	xl,a
 627  0165 6f0c          	clr	(_scnCnt,x)
 628                     ; 144 		u8TempKeyCode[u8ScanType]  = KEY_NONE;
 630  0167 7b01          	ld	a,(OFST+1,sp)
 631  0169 5f            	clrw	x
 632  016a 97            	ld	xl,a
 633  016b 6f04          	clr	(_u8TempKeyCode,x)
 634                     ; 145 		return;
 636  016d 20b9          	jra	L21
 660                     ; 163 void Key_Timer100ms(void)
 660                     ; 164 {
 661                     	switch	.text
 662  016f               _Key_Timer100ms:
 666                     ; 165 	u8KeyRepeatTimer100ms = (u8KeyRepeatTimer100ms + 1) % 3;
 668  016f b603          	ld	a,_u8KeyRepeatTimer100ms
 669  0171 5f            	clrw	x
 670  0172 97            	ld	xl,a
 671  0173 5c            	incw	x
 672  0174 a603          	ld	a,#3
 673  0176 cd0000        	call	c_smodx
 675  0179 9f            	ld	a,xl
 676  017a b703          	ld	_u8KeyRepeatTimer100ms,a
 677                     ; 166 }
 680  017c 81            	ret
 725                     ; 178 bool Key_IsRepeatStep(void)
 725                     ; 179 {
 726                     	switch	.text
 727  017d               _Key_IsRepeatStep:
 731                     ; 180 	if (u8KeyRepeatTimer100ms == 0)
 733  017d 3d03          	tnz	_u8KeyRepeatTimer100ms
 734  017f 2607          	jrne	L722
 735                     ; 182 		u8KeyRepeatTimer100ms = 1;
 737  0181 35010003      	mov	_u8KeyRepeatTimer100ms,#1
 738                     ; 183 		return TRUE;
 740  0185 a601          	ld	a,#1
 743  0187 81            	ret
 744  0188               L722:
 745                     ; 185 	return FALSE;
 747  0188 4f            	clr	a
 750  0189 81            	ret
 834                     ; 199 u8 Key_GetADKey(u8 u8AdChLine)
 834                     ; 200 {
 835                     	switch	.text
 836  018a               _Key_GetADKey:
 838  018a 88            	push	a
 839  018b 5213          	subw	sp,#19
 840       00000013      OFST:	set	19
 843                     ; 202 	u16 u16Temp = 0;
 845  018d 5f            	clrw	x
 846  018e 1f03          	ldw	(OFST-16,sp),x
 847                     ; 205 	u16 max=0;
 849  0190 5f            	clrw	x
 850  0191 1f05          	ldw	(OFST-14,sp),x
 851                     ; 206 	u16 min=1000;
 853  0193 ae03e8        	ldw	x,#1000
 854  0196 1f07          	ldw	(OFST-12,sp),x
 855                     ; 208 	for(i=0; i<5; i++)							// 取5次转化结果
 857  0198 0f13          	clr	(OFST+0,sp)
 858  019a               L372:
 859                     ; 210 		temp[i] = ADC_GetADValue(ADChannel[u8AdChLine]);					// 取值
 861  019a 7b14          	ld	a,(OFST+1,sp)
 862  019c 5f            	clrw	x
 863  019d 97            	ld	xl,a
 864  019e e600          	ld	a,(_ADChannel,x)
 865  01a0 cd0000        	call	_ADC_GetADValue
 867  01a3 9096          	ldw	y,sp
 868  01a5 72a90009      	addw	y,#OFST-10
 869  01a9 1701          	ldw	(OFST-18,sp),y
 870  01ab 7b13          	ld	a,(OFST+0,sp)
 871  01ad 905f          	clrw	y
 872  01af 9097          	ld	yl,a
 873  01b1 9058          	sllw	y
 874  01b3 72f901        	addw	y,(OFST-18,sp)
 875  01b6 90ff          	ldw	(y),x
 876                     ; 211 		u16Temp += temp[i];						// 值依次累加
 878  01b8 96            	ldw	x,sp
 879  01b9 1c0009        	addw	x,#OFST-10
 880  01bc 1f01          	ldw	(OFST-18,sp),x
 881  01be 7b13          	ld	a,(OFST+0,sp)
 882  01c0 5f            	clrw	x
 883  01c1 97            	ld	xl,a
 884  01c2 58            	sllw	x
 885  01c3 72fb01        	addw	x,(OFST-18,sp)
 886  01c6 fe            	ldw	x,(x)
 887  01c7 72fb03        	addw	x,(OFST-16,sp)
 888  01ca 1f03          	ldw	(OFST-16,sp),x
 889                     ; 213 		if(temp[i] < min)
 891  01cc 96            	ldw	x,sp
 892  01cd 1c0009        	addw	x,#OFST-10
 893  01d0 1f01          	ldw	(OFST-18,sp),x
 894  01d2 7b13          	ld	a,(OFST+0,sp)
 895  01d4 5f            	clrw	x
 896  01d5 97            	ld	xl,a
 897  01d6 58            	sllw	x
 898  01d7 72fb01        	addw	x,(OFST-18,sp)
 899  01da 9093          	ldw	y,x
 900  01dc 51            	exgw	x,y
 901  01dd fe            	ldw	x,(x)
 902  01de 1307          	cpw	x,(OFST-12,sp)
 903  01e0 51            	exgw	x,y
 904  01e1 2411          	jruge	L103
 905                     ; 215 			min = temp[i]; 
 907  01e3 96            	ldw	x,sp
 908  01e4 1c0009        	addw	x,#OFST-10
 909  01e7 1f01          	ldw	(OFST-18,sp),x
 910  01e9 7b13          	ld	a,(OFST+0,sp)
 911  01eb 5f            	clrw	x
 912  01ec 97            	ld	xl,a
 913  01ed 58            	sllw	x
 914  01ee 72fb01        	addw	x,(OFST-18,sp)
 915  01f1 fe            	ldw	x,(x)
 916  01f2 1f07          	ldw	(OFST-12,sp),x
 917  01f4               L103:
 918                     ; 217 		if(temp[i] > max)
 920  01f4 96            	ldw	x,sp
 921  01f5 1c0009        	addw	x,#OFST-10
 922  01f8 1f01          	ldw	(OFST-18,sp),x
 923  01fa 7b13          	ld	a,(OFST+0,sp)
 924  01fc 5f            	clrw	x
 925  01fd 97            	ld	xl,a
 926  01fe 58            	sllw	x
 927  01ff 72fb01        	addw	x,(OFST-18,sp)
 928  0202 9093          	ldw	y,x
 929  0204 51            	exgw	x,y
 930  0205 fe            	ldw	x,(x)
 931  0206 1305          	cpw	x,(OFST-14,sp)
 932  0208 51            	exgw	x,y
 933  0209 2311          	jrule	L303
 934                     ; 219 			max = temp[i];
 936  020b 96            	ldw	x,sp
 937  020c 1c0009        	addw	x,#OFST-10
 938  020f 1f01          	ldw	(OFST-18,sp),x
 939  0211 7b13          	ld	a,(OFST+0,sp)
 940  0213 5f            	clrw	x
 941  0214 97            	ld	xl,a
 942  0215 58            	sllw	x
 943  0216 72fb01        	addw	x,(OFST-18,sp)
 944  0219 fe            	ldw	x,(x)
 945  021a 1f05          	ldw	(OFST-14,sp),x
 946  021c               L303:
 947                     ; 208 	for(i=0; i<5; i++)							// 取5次转化结果
 949  021c 0c13          	inc	(OFST+0,sp)
 952  021e 7b13          	ld	a,(OFST+0,sp)
 953  0220 a105          	cp	a,#5
 954  0222 2403          	jruge	L62
 955  0224 cc019a        	jp	L372
 956  0227               L62:
 957                     ; 222 	AD_Value= ((u16Temp - min - max)/3);		// 减去最大和最小值，在去平均值
 959  0227 1e03          	ldw	x,(OFST-16,sp)
 960  0229 72f007        	subw	x,(OFST-12,sp)
 961  022c 72f005        	subw	x,(OFST-14,sp)
 962  022f a603          	ld	a,#3
 963  0231 62            	div	x,a
 964  0232 bf00          	ldw	_AD_Value,x
 965                     ; 224 	if((max-min > 20) || (AD_Value  >= 1000))	// 如果最大和最小值差20以上就直接返回,否则会串键
 967  0234 1e05          	ldw	x,(OFST-14,sp)
 968  0236 72f007        	subw	x,(OFST-12,sp)
 969  0239 a30015        	cpw	x,#21
 970  023c 2407          	jruge	L703
 972  023e be00          	ldw	x,_AD_Value
 973  0240 a303e8        	cpw	x,#1000
 974  0243 2503          	jrult	L503
 975  0245               L703:
 976                     ; 226 		return KEY_NONE;
 978  0245 4f            	clr	a
 980  0246 2008          	jra	L42
 981  0248               L503:
 982                     ; 229 	return Key_GetKeyCode(u8AdChLine,AD_Value );
 984  0248 be00          	ldw	x,_AD_Value
 985  024a 89            	pushw	x
 986  024b 7b16          	ld	a,(OFST+3,sp)
 987  024d ad04          	call	_Key_GetKeyCode
 989  024f 85            	popw	x
 991  0250               L42:
 993  0250 5b14          	addw	sp,#20
 994  0252 81            	ret
1048                     ; 243 u8 Key_GetKeyCode(u8 u8Line, u16 u16Value)
1048                     ; 244 {
1049                     	switch	.text
1050  0253               _Key_GetKeyCode:
1052  0253 88            	push	a
1053  0254 88            	push	a
1054       00000001      OFST:	set	1
1057                     ; 247 	for (i = 0; i < MAX_KEY_PER_LINE; i++)
1059  0255 0f01          	clr	(OFST+0,sp)
1060  0257               L733:
1061                     ; 249 		if (u16Value <= KEY_AD_TAB[i])
1063  0257 7b01          	ld	a,(OFST+0,sp)
1064  0259 5f            	clrw	x
1065  025a 97            	ld	xl,a
1066  025b 58            	sllw	x
1067  025c 9093          	ldw	y,x
1068  025e 51            	exgw	x,y
1069  025f de002a        	ldw	x,(_KEY_AD_TAB,x)
1070  0262 1305          	cpw	x,(OFST+4,sp)
1071  0264 51            	exgw	x,y
1072  0265 2408          	jruge	L343
1073                     ; 251 			break;
1075                     ; 247 	for (i = 0; i < MAX_KEY_PER_LINE; i++)
1077  0267 0c01          	inc	(OFST+0,sp)
1080  0269 7b01          	ld	a,(OFST+0,sp)
1081  026b a104          	cp	a,#4
1082  026d 25e8          	jrult	L733
1083  026f               L343:
1084                     ; 255 	return KEY_MAP_TAB[u8Line][i];
1086  026f 7b02          	ld	a,(OFST+1,sp)
1087  0271 97            	ld	xl,a
1088  0272 a605          	ld	a,#5
1089  0274 42            	mul	x,a
1090  0275 01            	rrwa	x,a
1091  0276 1b01          	add	a,(OFST+0,sp)
1092  0278 2401          	jrnc	L23
1093  027a 5c            	incw	x
1094  027b               L23:
1095  027b 02            	rlwa	x,a
1096  027c d60020        	ld	a,(_KEY_MAP_TAB,x)
1099  027f 85            	popw	x
1100  0280 81            	ret
1216                     	xdef	_Key_GetKeyCode
1217                     	xdef	_Key_GetADKey
1218                     	xdef	_Key_LongShortProcess
1219                     	xdef	_ADChannel
1220                     	xdef	_KEY_AD_TAB
1221                     	xdef	_KEY_MAP_TAB
1222                     	switch	.ubsct
1223  0000               _AD_Value:
1224  0000 0000          	ds.b	2
1225                     	xdef	_AD_Value
1226  0002               _u8KeyCode:
1227  0002 00            	ds.b	1
1228                     	xdef	_u8KeyCode
1229  0003               _u8KeyRepeatTimer100ms:
1230  0003 00            	ds.b	1
1231                     	xdef	_u8KeyRepeatTimer100ms
1232  0004               _u8TempKeyCode:
1233  0004 0000          	ds.b	2
1234                     	xdef	_u8TempKeyCode
1235  0006               _bKeyActive:
1236  0006 0000          	ds.b	2
1237                     	xdef	_bKeyActive
1238  0008               _u16KeyScanCount:
1239  0008 00000000      	ds.b	4
1240                     	xdef	_u16KeyScanCount
1241  000c               _scnCnt:
1242  000c 0000          	ds.b	2
1243                     	xdef	_scnCnt
1244                     	xref.b	_bMaxVolume
1245                     	xdef	_Key_GetCode
1246                     	xdef	_Key_IsRepeatStep
1247                     	xdef	_Key_Timer100ms
1248                     	xdef	_Key_Scan
1249  000e               _LineInLedMode:
1250  000e 00            	ds.b	1
1251                     	xdef	_LineInLedMode
1252                     	xdef	_DELAY_15_15
1253                     	xdef	_DELAY_02_3
1254                     	xdef	_DELAY_025_025
1255                     	xdef	_DELAY_02_02
1256                     	xdef	_DELAY_1_3
1257                     	xdef	_DELAY_1_1
1258                     	xdef	_DELAY_05_05
1259                     	xdef	_DELAY_0_0
1260                     	xref.b	_MainTask
1261                     	xref	_ADC_GetADValue
1262                     	xref.b	_Clock
1263                     	xref.b	c_x
1283                     	xref	c_smodx
1284                     	end
