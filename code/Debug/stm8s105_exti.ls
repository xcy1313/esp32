   1                     ; C Compiler for STM8 (COSMIC Software)
   2                     ; Generator V4.2.8 - 03 Dec 2008
  34                     ; 15 void MCU_EXTI_Init(void)
  34                     ; 16 {
  36                     	switch	.text
  37  0000               _MCU_EXTI_Init:
  41                     ; 17 	EXTI_DeInit();
  43  0000 cd009d        	call	_EXTI_DeInit
  45                     ; 19 	EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOB, EXTI_SENSITIVITY_FALL_ONLY);
  47  0003 ae0002        	ldw	x,#2
  48  0006 a601          	ld	a,#1
  49  0008 95            	ld	xh,a
  50  0009 cd00a6        	call	_EXTI_SetExtIntSensitivity
  52                     ; 21 	EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOA, EXTI_SENSITIVITY_RISE_ONLY);
  54  000c ae0001        	ldw	x,#1
  55  000f 4f            	clr	a
  56  0010 95            	ld	xh,a
  57  0011 cd00a6        	call	_EXTI_SetExtIntSensitivity
  59                     ; 23 }
  62  0014 81            	ret
  89                     ; 36 @near @interrupt void EXTI_PORTA_IRQHandler(void)
  89                     ; 37 {
  90                     	switch	.text
  91  0015               _EXTI_PORTA_IRQHandler:
  93  0015 3b0002        	push	c_x+2
  94  0018 be00          	ldw	x,c_x
  95  001a 89            	pushw	x
  96  001b 3b0002        	push	c_y+2
  97  001e be00          	ldw	x,c_y
  98  0020 89            	pushw	x
 101                     ; 38 	if (!IsPowerOn)
 103  0021 3d00          	tnz	_IsPowerOn
 104  0023 261c          	jrne	L13
 105                     ; 40 		EXTI_INT_Disable(GPIOA, GPIO_Pin_2);
 107  0025 4b04          	push	#4
 108  0027 ae5000        	ldw	x,#20480
 109  002a ad5e          	call	_EXTI_INT_Disable
 111  002c 84            	pop	a
 112                     ; 41 		EXTI_INT_Disable(GPIOB, GPIO_Pin_1);
 114  002d 4b02          	push	#2
 115  002f ae5005        	ldw	x,#20485
 116  0032 ad56          	call	_EXTI_INT_Disable
 118  0034 84            	pop	a
 119                     ; 42 		MainTask |= CHARGING_POWEROFF_TASK;
 121  0035 b600          	ld	a,_MainTask
 122  0037 97            	ld	xl,a
 123  0038 b601          	ld	a,_MainTask+1
 124  003a aa40          	or	a,#64
 125  003c b701          	ld	_MainTask+1,a
 126  003e 9f            	ld	a,xl
 127  003f b700          	ld	_MainTask,a
 128  0041               L13:
 129                     ; 44 	return;
 132  0041 85            	popw	x
 133  0042 bf00          	ldw	c_y,x
 134  0044 320002        	pop	c_y+2
 135  0047 85            	popw	x
 136  0048 bf00          	ldw	c_x,x
 137  004a 320002        	pop	c_x+2
 138  004d 80            	iret
 165                     ; 58 @near @interrupt void EXTI_PORTB_IRQHandler(void)
 165                     ; 59 {
 166                     	switch	.text
 167  004e               _EXTI_PORTB_IRQHandler:
 169  004e 3b0002        	push	c_x+2
 170  0051 be00          	ldw	x,c_x
 171  0053 89            	pushw	x
 172  0054 3b0002        	push	c_y+2
 173  0057 be00          	ldw	x,c_y
 174  0059 89            	pushw	x
 177                     ; 60 	if (!IsPowerOn)
 179  005a 3d00          	tnz	_IsPowerOn
 180  005c 261c          	jrne	L34
 181                     ; 62 		EXTI_INT_Disable(GPIOA, GPIO_Pin_2);
 183  005e 4b04          	push	#4
 184  0060 ae5000        	ldw	x,#20480
 185  0063 ad25          	call	_EXTI_INT_Disable
 187  0065 84            	pop	a
 188                     ; 63 		EXTI_INT_Disable(GPIOB, GPIO_Pin_1);
 190  0066 4b02          	push	#2
 191  0068 ae5005        	ldw	x,#20485
 192  006b ad1d          	call	_EXTI_INT_Disable
 194  006d 84            	pop	a
 195                     ; 64 		MainTask |= SYSTEM_POWERON_TASK;
 197  006e b600          	ld	a,_MainTask
 198  0070 97            	ld	xl,a
 199  0071 b601          	ld	a,_MainTask+1
 200  0073 aa04          	or	a,#4
 201  0075 b701          	ld	_MainTask+1,a
 202  0077 9f            	ld	a,xl
 203  0078 b700          	ld	_MainTask,a
 204  007a               L34:
 205                     ; 66 	return;
 208  007a 85            	popw	x
 209  007b bf00          	ldw	c_y,x
 210  007d 320002        	pop	c_y+2
 211  0080 85            	popw	x
 212  0081 bf00          	ldw	c_x,x
 213  0083 320002        	pop	c_x+2
 214  0086 80            	iret
 238                     ; 80 @near @interrupt void EXTI_PORTC_IRQHandler(void)
 238                     ; 81 {
 239                     	switch	.text
 240  0087               _EXTI_PORTC_IRQHandler:
 244                     ; 82 	return;
 247  0087 80            	iret
 271                     ; 96 @near @interrupt void EXTI_PORTD_IRQHandler(void)
 271                     ; 97 {
 272                     	switch	.text
 273  0088               _EXTI_PORTD_IRQHandler:
 277                     ; 98 	return;
 280  0088 80            	iret
 304                     ; 112 @near @interrupt void EXTI_PORTE_IRQHandler(void)
 304                     ; 113 {
 305                     	switch	.text
 306  0089               _EXTI_PORTE_IRQHandler:
 310                     ; 114 	return;
 313  0089 80            	iret
 527                     ; 128 void EXTI_INT_Disable(GPIO_TypeDef* GPIOx, u8 GPIO_Pin)
 527                     ; 129 {
 528                     	switch	.text
 529  008a               _EXTI_INT_Disable:
 531  008a 89            	pushw	x
 532       00000000      OFST:	set	0
 535                     ; 130 	GPIOx->CR2.byte &= (u8)(~(GPIO_Pin));
 537  008b 7b05          	ld	a,(OFST+5,sp)
 538  008d 43            	cpl	a
 539  008e e404          	and	a,(4,x)
 540  0090 e704          	ld	(4,x),a
 541                     ; 131 }
 544  0092 85            	popw	x
 545  0093 81            	ret
 591                     ; 144 void EXTI_INT_Enable(GPIO_TypeDef* GPIOx, u8 GPIO_Pin)
 591                     ; 145 {
 592                     	switch	.text
 593  0094               _EXTI_INT_Enable:
 595  0094 89            	pushw	x
 596       00000000      OFST:	set	0
 599                     ; 146 	GPIOx->CR2.byte |= GPIO_Pin;
 601  0095 e604          	ld	a,(4,x)
 602  0097 1a05          	or	a,(OFST+5,sp)
 603  0099 e704          	ld	(4,x),a
 604                     ; 147 }
 607  009b 85            	popw	x
 608  009c 81            	ret
 631                     ; 160 void EXTI_DeInit(void)
 631                     ; 161 {
 632                     	switch	.text
 633  009d               _EXTI_DeInit:
 637                     ; 162 	EXTI->CR1 = EXTI_CR1_RESET_VALUE;
 639  009d 725f50a0      	clr	20640
 640                     ; 163 	EXTI->CR2 = EXTI_CR2_RESET_VALUE;
 642  00a1 725f50a1      	clr	20641
 643                     ; 164 }
 646  00a5 81            	ret
 771                     ; 177 void EXTI_SetExtIntSensitivity(EXTI_Port_TypeDef Port, EXTI_Sensitivity_TypeDef SensitivityValue)
 771                     ; 178 {
 772                     	switch	.text
 773  00a6               _EXTI_SetExtIntSensitivity:
 775  00a6 89            	pushw	x
 776       00000000      OFST:	set	0
 779                     ; 179 	switch(Port)	// Set external interrupt sensitivity
 781  00a7 9e            	ld	a,xh
 783                     ; 201 		default:
 783                     ; 202 			break;
 784  00a8 4d            	tnz	a
 785  00a9 270e          	jreq	L362
 786  00ab 4a            	dec	a
 787  00ac 271d          	jreq	L562
 788  00ae 4a            	dec	a
 789  00af 272e          	jreq	L762
 790  00b1 4a            	dec	a
 791  00b2 2742          	jreq	L172
 792  00b4 4a            	dec	a
 793  00b5 2756          	jreq	L372
 794  00b7 2064          	jra	L553
 795  00b9               L362:
 796                     ; 181 		case EXTI_PORT_GPIOA:
 796                     ; 182 			EXTI->CR1 &= (u8)(~EXTI_CR1_PAIS);
 798  00b9 c650a0        	ld	a,20640
 799  00bc a4fc          	and	a,#252
 800  00be c750a0        	ld	20640,a
 801                     ; 183 			EXTI->CR1 |= (u8)(SensitivityValue);
 803  00c1 c650a0        	ld	a,20640
 804  00c4 1a02          	or	a,(OFST+2,sp)
 805  00c6 c750a0        	ld	20640,a
 806                     ; 184 			break;
 808  00c9 2052          	jra	L553
 809  00cb               L562:
 810                     ; 185 		case EXTI_PORT_GPIOB:
 810                     ; 186 			EXTI->CR1 &= (u8)(~EXTI_CR1_PBIS);
 812  00cb c650a0        	ld	a,20640
 813  00ce a4f3          	and	a,#243
 814  00d0 c750a0        	ld	20640,a
 815                     ; 187 			EXTI->CR1 |= (u8)((u8)(SensitivityValue) << 2);
 817  00d3 7b02          	ld	a,(OFST+2,sp)
 818  00d5 48            	sll	a
 819  00d6 48            	sll	a
 820  00d7 ca50a0        	or	a,20640
 821  00da c750a0        	ld	20640,a
 822                     ; 188 			break;
 824  00dd 203e          	jra	L553
 825  00df               L762:
 826                     ; 189 		case EXTI_PORT_GPIOC:
 826                     ; 190 			EXTI->CR1 &= (u8)(~EXTI_CR1_PCIS);
 828  00df c650a0        	ld	a,20640
 829  00e2 a4cf          	and	a,#207
 830  00e4 c750a0        	ld	20640,a
 831                     ; 191 			EXTI->CR1 |= (u8)((u8)(SensitivityValue) << 4);
 833  00e7 7b02          	ld	a,(OFST+2,sp)
 834  00e9 97            	ld	xl,a
 835  00ea a610          	ld	a,#16
 836  00ec 42            	mul	x,a
 837  00ed 9f            	ld	a,xl
 838  00ee ca50a0        	or	a,20640
 839  00f1 c750a0        	ld	20640,a
 840                     ; 192 			break;
 842  00f4 2027          	jra	L553
 843  00f6               L172:
 844                     ; 193 		case EXTI_PORT_GPIOD:
 844                     ; 194 			EXTI->CR1 &= (u8)(~EXTI_CR1_PDIS);
 846  00f6 c650a0        	ld	a,20640
 847  00f9 a43f          	and	a,#63
 848  00fb c750a0        	ld	20640,a
 849                     ; 195 			EXTI->CR1 |= (u8)((u8)(SensitivityValue) << 6);
 851  00fe 7b02          	ld	a,(OFST+2,sp)
 852  0100 97            	ld	xl,a
 853  0101 a640          	ld	a,#64
 854  0103 42            	mul	x,a
 855  0104 9f            	ld	a,xl
 856  0105 ca50a0        	or	a,20640
 857  0108 c750a0        	ld	20640,a
 858                     ; 196 			break;
 860  010b 2010          	jra	L553
 861  010d               L372:
 862                     ; 197 		case EXTI_PORT_GPIOE:
 862                     ; 198 			EXTI->CR2 &= (u8)(~EXTI_CR2_PEIS);
 864  010d c650a1        	ld	a,20641
 865  0110 a4fc          	and	a,#252
 866  0112 c750a1        	ld	20641,a
 867                     ; 199 			EXTI->CR2 |= (u8)(SensitivityValue);
 869  0115 c650a1        	ld	a,20641
 870  0118 1a02          	or	a,(OFST+2,sp)
 871  011a c750a1        	ld	20641,a
 872                     ; 200 			break;
 874  011d               L572:
 875                     ; 201 		default:
 875                     ; 202 			break;
 877  011d               L553:
 878                     ; 204 }
 881  011d 85            	popw	x
 882  011e 81            	ret
 940                     ; 217 void EXTI_SetTLISensitivity(EXTI_TLISensitivity_TypeDef SensitivityValue)
 940                     ; 218 {
 941                     	switch	.text
 942  011f               _EXTI_SetTLISensitivity:
 946                     ; 219 	EXTI->CR2 &= (u8)(~EXTI_CR2_TLIS);	// Set TLI interrupt sensitivity
 948  011f 721550a1      	bres	20641,#2
 949                     ; 220 	EXTI->CR2 |= (u8)(SensitivityValue);
 951  0123 ca50a1        	or	a,20641
 952  0126 c750a1        	ld	20641,a
 953                     ; 221 }
 956  0129 81            	ret
 969                     	xref.b	_IsPowerOn
 970                     	xref.b	_MainTask
 971                     	xdef	_EXTI_SetTLISensitivity
 972                     	xdef	_EXTI_SetExtIntSensitivity
 973                     	xdef	_EXTI_DeInit
 974                     	xdef	_EXTI_INT_Enable
 975                     	xdef	_EXTI_INT_Disable
 976                     	xdef	_EXTI_PORTE_IRQHandler
 977                     	xdef	_EXTI_PORTD_IRQHandler
 978                     	xdef	_EXTI_PORTC_IRQHandler
 979                     	xdef	_EXTI_PORTB_IRQHandler
 980                     	xdef	_EXTI_PORTA_IRQHandler
 981                     	xdef	_MCU_EXTI_Init
 982                     	xref.b	c_x
 983                     	xref.b	c_y
1002                     	end
