   1                     ; C Compiler for STM8 (COSMIC Software)
   2                     ; Generator V4.2.8 - 03 Dec 2008
  32                     ; 17 void MCU_CLK_Init(void)
  32                     ; 18 {
  34                     	switch	.text
  35  0000               _MCU_CLK_Init:
  39                     ; 22 	CLK->CKDIVR = 0x10;			// Fmaster=4MHZ 4分频
  41  0000 351050c6      	mov	20678,#16
  42                     ; 25 	CLK->PCKENR1 = 0b10110000;	// TIM1,TIM3,TIM2,TIM4,UART2使用外设部的晶振
  44  0004 35b050c7      	mov	20679,#176
  45                     ; 26 	CLK->PCKENR2 = 0b00001000;	// ADC使用外设部的晶振
  47  0008 350850ca      	mov	20682,#8
  48                     ; 27 	CLK->PCKENR2 |= 0x08;		// Enable the clock for ADC
  50  000c 721650ca      	bset	20682,#3
  51                     ; 29 	CLK->SWCR |= 0x02;			// 使能时钟切换
  53  0010 721250c5      	bset	20677,#1
  54                     ; 30 	CLK->SWR = 0xE1;			// HSI为主时钟源
  56  0014 35e150c4      	mov	20676,#225
  57                     ; 33 }
  60  0018 81            	ret
 384                     ; 46 void GPIO_Init(GPIO_TypeDef* GPIOx, u8 GPIO_Pin, GPIO_Mode_TypeDef GPIO_Mode)
 384                     ; 47 {
 385                     	switch	.text
 386  0019               _GPIO_Init:
 388  0019 89            	pushw	x
 389       00000000      OFST:	set	0
 392                     ; 48 	if((((u8)(GPIO_Mode)) & (u8)0x80) != (u8)0x00)		// Output mode
 394  001a 7b06          	ld	a,(OFST+6,sp)
 395  001c a580          	bcp	a,#128
 396  001e 271d          	jreq	L312
 397                     ; 50 		if((((u8)(GPIO_Mode)) & (u8)0x10) != (u8)0x00)	// High level
 399  0020 7b06          	ld	a,(OFST+6,sp)
 400  0022 a510          	bcp	a,#16
 401  0024 2706          	jreq	L512
 402                     ; 52 			GPIOx->ODR.byte |= GPIO_Pin;
 404  0026 f6            	ld	a,(x)
 405  0027 1a05          	or	a,(OFST+5,sp)
 406  0029 f7            	ld	(x),a
 408  002a 2007          	jra	L712
 409  002c               L512:
 410                     ; 56 			GPIOx->ODR.byte &= (u8)(~(GPIO_Pin));
 412  002c 1e01          	ldw	x,(OFST+1,sp)
 413  002e 7b05          	ld	a,(OFST+5,sp)
 414  0030 43            	cpl	a
 415  0031 f4            	and	a,(x)
 416  0032 f7            	ld	(x),a
 417  0033               L712:
 418                     ; 59 		GPIOx->DDR.byte |= GPIO_Pin;					// Set Output mode
 420  0033 1e01          	ldw	x,(OFST+1,sp)
 421  0035 e602          	ld	a,(2,x)
 422  0037 1a05          	or	a,(OFST+5,sp)
 423  0039 e702          	ld	(2,x),a
 425  003b 2009          	jra	L122
 426  003d               L312:
 427                     ; 63 		GPIOx->DDR.byte &= (u8)(~(GPIO_Pin));			// Set Input mode
 429  003d 1e01          	ldw	x,(OFST+1,sp)
 430  003f 7b05          	ld	a,(OFST+5,sp)
 431  0041 43            	cpl	a
 432  0042 e402          	and	a,(2,x)
 433  0044 e702          	ld	(2,x),a
 434  0046               L122:
 435                     ; 66 	if((((u8)(GPIO_Mode)) & (u8)0x40) != (u8)0x00)		// Pull-Up or Push-Pull
 437  0046 7b06          	ld	a,(OFST+6,sp)
 438  0048 a540          	bcp	a,#64
 439  004a 270a          	jreq	L322
 440                     ; 68 		GPIOx->CR1.byte |= GPIO_Pin;
 442  004c 1e01          	ldw	x,(OFST+1,sp)
 443  004e e603          	ld	a,(3,x)
 444  0050 1a05          	or	a,(OFST+5,sp)
 445  0052 e703          	ld	(3,x),a
 447  0054 2009          	jra	L522
 448  0056               L322:
 449                     ; 72 		GPIOx->CR1.byte &= (u8)(~(GPIO_Pin));
 451  0056 1e01          	ldw	x,(OFST+1,sp)
 452  0058 7b05          	ld	a,(OFST+5,sp)
 453  005a 43            	cpl	a
 454  005b e403          	and	a,(3,x)
 455  005d e703          	ld	(3,x),a
 456  005f               L522:
 457                     ; 75 	if((((u8)(GPIO_Mode)) & (u8)0x20) != (u8)0x00)		// Interrupt or Slow slope
 459  005f 7b06          	ld	a,(OFST+6,sp)
 460  0061 a520          	bcp	a,#32
 461  0063 270a          	jreq	L722
 462                     ; 77 		GPIOx->CR2.byte |= GPIO_Pin;
 464  0065 1e01          	ldw	x,(OFST+1,sp)
 465  0067 e604          	ld	a,(4,x)
 466  0069 1a05          	or	a,(OFST+5,sp)
 467  006b e704          	ld	(4,x),a
 469  006d 2009          	jra	L132
 470  006f               L722:
 471                     ; 81 		GPIOx->CR2.byte &= (u8)(~(GPIO_Pin));
 473  006f 1e01          	ldw	x,(OFST+1,sp)
 474  0071 7b05          	ld	a,(OFST+5,sp)
 475  0073 43            	cpl	a
 476  0074 e404          	and	a,(4,x)
 477  0076 e704          	ld	(4,x),a
 478  0078               L132:
 479                     ; 83 }
 482  0078 85            	popw	x
 483  0079 81            	ret
 507                     ; 96 void GPIO_Configuration(void)
 507                     ; 97 {
 508                     	switch	.text
 509  007a               _GPIO_Configuration:
 513                     ; 98 	GPIO_Reset();
 515  007a cd0146        	call	_GPIO_Reset
 517                     ; 102  	POWER_LED_PDDR = 1;			
 519  007d 721e500c      	bset	20492,#7
 520                     ; 103  	POWER_LED_PCR1 = 1;			
 522  0081 721e500d      	bset	20493,#7
 523                     ; 105 	RED_LED_PDDR = 1;
 525  0085 721c500c      	bset	20492,#6
 526                     ; 106 	RED_LED_PCR1 = 1;
 528  0089 721c500d      	bset	20493,#6
 529                     ; 108 	LINE_IN_LED_PDDR = 1;		
 531  008d 721a500c      	bset	20492,#5
 532                     ; 109  	LINE_IN_LED_PCR1 = 1;
 534  0091 721a500d      	bset	20493,#5
 535                     ; 111 	BT_LED_PDDR = 1;			
 537  0095 7218500c      	bset	20492,#4
 538                     ; 112  	BT_LED_PCR1 = 1;	
 540  0099 7218500d      	bset	20493,#4
 541                     ; 114 	BT_PAIR_BUTTON_PDDR = 1;	
 543  009d 72185011      	bset	20497,#4
 544                     ; 115 	BT_PAIR_BUTTON_PCR1 = 1;	
 546  00a1 72185012      	bset	20498,#4
 547                     ; 117 	BT_ANSWER_BUTTON_PDDR = 1;
 549  00a5 72145011      	bset	20497,#2
 550                     ; 118 	BT_ANSWER_BUTTON_PCR1 = 1;	
 552  00a9 72145012      	bset	20498,#2
 553                     ; 120 	BT_VOLMAX_PDDR = 1;
 555  00ad 721e5011      	bset	20497,#7
 556                     ; 121 	BT_VOLMAX_PCR1 = 1;
 558  00b1 721e5012      	bset	20498,#7
 559                     ; 123 	BT_POWER_PDDR = 1;			
 561  00b5 72125002      	bset	20482,#1
 562                     ; 124 	BT_POWER_PCR1 = 1;	
 564  00b9 72125003      	bset	20483,#1
 565                     ; 126 	PDDR_IIC_SCL = 1;
 567  00bd 72185007      	bset	20487,#4
 568                     ; 127 	PCR1_IIC_SCL = 1;
 570  00c1 72185008      	bset	20488,#4
 571                     ; 129 	PDDR_IIC_SDA = 1;
 573  00c5 721a5007      	bset	20487,#5
 574                     ; 130 	PCR1_IIC_SDA = 1;
 576  00c9 721a5008      	bset	20488,#5
 577                     ; 133 	PDDR_BEEP_PIN1 = 1;
 579  00cd 7212500c      	bset	20492,#1
 580                     ; 134 	PCR1_BEEP_PIN1 = 1;
 582  00d1 7212500d      	bset	20493,#1
 583                     ; 136 	PDDR_BEEP_PIN2 = 1;
 585  00d5 721a5016      	bset	20502,#5
 586                     ; 137 	PCR1_BEEP_PIN2 = 1;
 588  00d9 721a5017      	bset	20503,#5
 589                     ; 139 	AMP_ON_PDDR = 1;
 591  00dd 72165002      	bset	20482,#3
 592                     ; 140 	AMP_ON_PCR1 = 1;
 594  00e1 72165003      	bset	20483,#3
 595                     ; 142 	BT_LOWBATTERY_PDDR = 1;
 597  00e5 72165011      	bset	20497,#3
 598                     ; 143 	BT_LOWBATTERY_PCR1 = 1;
 600  00e9 72165012      	bset	20498,#3
 601                     ; 147 	ADC_LINE0_PDDR = 0;
 603  00ed 72115007      	bres	20487,#0
 604                     ; 148 	ADC_LINE0_PCR1 = 0;	
 606  00f1 72115008      	bres	20488,#0
 607                     ; 150 	ADC_LINE1_PDDR = 0;			
 609  00f5 72135007      	bres	20487,#1
 610                     ; 151 	ADC_LINE1_PCR1 = 0;			
 612  00f9 72135008      	bres	20488,#1
 613                     ; 153 	ADC_LINE2_PDDR = 0;			
 615  00fd 72155007      	bres	20487,#2
 616                     ; 154 	ADC_LINE2_PCR1 = 0;	
 618  0101 72155008      	bres	20488,#2
 619                     ; 156 	ADC_LINE3_PDDR = 0;			
 621  0105 72175007      	bres	20487,#3
 622                     ; 157 	ADC_LINE3_PCR1 = 0;			
 624  0109 72175008      	bres	20488,#3
 625                     ; 159 	AUX_CHECK_PDDR = 0;
 627  010d 721d5007      	bres	20487,#6
 628                     ; 160 	AUX_CHECK_PCR1 = 0;
 630  0111 721d5008      	bres	20488,#6
 631                     ; 162 	SWITCH_CHECK_PDDR = 0;
 633  0115 721f5007      	bres	20487,#7
 634                     ; 163 	SWITCH_CHECK_PCR1 = 0;
 636  0119 721f5008      	bres	20488,#7
 637                     ; 166 	CHARGE_CHECK_PDDR = 0;
 639  011d 72155002      	bres	20482,#2
 640                     ; 167 	CHARGE_CHECK_PCR1 = 0;
 642  0121 72155003      	bres	20483,#2
 643                     ; 169 	BT_PAIRING_PDDR = 0;		
 645  0125 721d5011      	bres	20497,#6
 646                     ; 170 	BT_PAIRING_PCR1 = 0;		
 648  0129 721d5012      	bres	20498,#6
 649                     ; 172 	BT_PAIRED_PDDR = 0;			
 651  012d 721b5011      	bres	20497,#5
 652                     ; 173 	BT_PAIRED_PCR1 = 0;			
 654  0131 721b5012      	bres	20498,#5
 655                     ; 175 	BT_PHONE_PDDR = 0;			
 657  0135 72115011      	bres	20497,#0
 658                     ; 176 	BT_PHONE_PCR1 = 0;		
 660  0139 72115012      	bres	20498,#0
 661                     ; 179 	SLEEP_PDDR = 0;
 663  013d 7219501b      	bres	20507,#4
 664                     ; 180 	SLEEP_PCR1 = 0;
 666  0141 7219501c      	bres	20508,#4
 667                     ; 182 }
 670  0145 81            	ret
 694                     ; 195 void GPIO_Reset(void)
 694                     ; 196 {
 695                     	switch	.text
 696  0146               _GPIO_Reset:
 700                     ; 197 	GPIO_Init(GPIOA, GPIO_Pin_All, GPIO_Mode_In_FL_No_IT);
 702  0146 4b00          	push	#0
 703  0148 4bff          	push	#255
 704  014a ae5000        	ldw	x,#20480
 705  014d cd0019        	call	_GPIO_Init
 707  0150 85            	popw	x
 708                     ; 198 	GPIO_Init(GPIOB, GPIO_Pin_All, GPIO_Mode_In_FL_No_IT);
 710  0151 4b00          	push	#0
 711  0153 4bff          	push	#255
 712  0155 ae5005        	ldw	x,#20485
 713  0158 cd0019        	call	_GPIO_Init
 715  015b 85            	popw	x
 716                     ; 199 	GPIO_Init(GPIOC, GPIO_Pin_All, GPIO_Mode_In_FL_No_IT);
 718  015c 4b00          	push	#0
 719  015e 4bff          	push	#255
 720  0160 ae500a        	ldw	x,#20490
 721  0163 cd0019        	call	_GPIO_Init
 723  0166 85            	popw	x
 724                     ; 200 	GPIO_Init(GPIOD, GPIO_Pin_All, GPIO_Mode_In_FL_No_IT);
 726  0167 4b00          	push	#0
 727  0169 4bff          	push	#255
 728  016b ae500f        	ldw	x,#20495
 729  016e cd0019        	call	_GPIO_Init
 731  0171 85            	popw	x
 732                     ; 201 	GPIO_Init(GPIOE, GPIO_Pin_All, GPIO_Mode_In_FL_No_IT);
 734  0172 4b00          	push	#0
 735  0174 4bff          	push	#255
 736  0176 ae5014        	ldw	x,#20500
 737  0179 cd0019        	call	_GPIO_Init
 739  017c 85            	popw	x
 740                     ; 202 	GPIO_Init(GPIOF, GPIO_Pin_All, GPIO_Mode_In_FL_No_IT);
 742  017d 4b00          	push	#0
 743  017f 4bff          	push	#255
 744  0181 ae5019        	ldw	x,#20505
 745  0184 cd0019        	call	_GPIO_Init
 747  0187 85            	popw	x
 748                     ; 203 }
 751  0188 81            	ret
 764                     	xdef	_GPIO_Reset
 765                     	xdef	_GPIO_Configuration
 766                     	xdef	_MCU_CLK_Init
 767                     	xdef	_GPIO_Init
 786                     	end
