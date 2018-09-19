   1                     ; C Compiler for STM8 (COSMIC Software)
   2                     ; Generator V4.2.8 - 03 Dec 2008
  32                     ; 15 @near @interrupt void AWU_IRQHandler (void)
  32                     ; 16 {
  34                     	switch	.text
  35  0000               _AWU_IRQHandler:
  39                     ; 23 	return;
  42  0000 80            	iret
  86                     ; 37 FlagStatus AWU_GetFlagStatus(void)
  86                     ; 38 {
  87                     	switch	.text
  88  0001               _AWU_GetFlagStatus:
  92                     ; 39 	return((FlagStatus)(((u8)(AWU->CSR & AWU_CSR_AWUF) == (u8)0x00) ? RESET : SET));
  94  0001 c650f0        	ld	a,20720
  95  0004 a520          	bcp	a,#32
  96  0006 2603          	jrne	L01
  97  0008 4f            	clr	a
  98  0009 2002          	jra	L21
  99  000b               L01:
 100  000b a601          	ld	a,#1
 101  000d               L21:
 104  000d 81            	ret
 147                     ; 53 void AWU_Config(u8 TBR, u8 APR)
 147                     ; 54 {
 148                     	switch	.text
 149  000e               _AWU_Config:
 153                     ; 55 	CLK->PCKENR2 |= 0x04;	// Enable the clock PCG for AWU
 155  000e 721450ca      	bset	20682,#2
 156                     ; 57 	AWU->APR = APR;
 158  0012 9f            	ld	a,xl
 159  0013 c750f1        	ld	20721,a
 160                     ; 58 	AWU->TBR = TBR;
 162  0016 9e            	ld	a,xh
 163  0017 c750f2        	ld	20722,a
 164                     ; 60 	AWU->CSR |= 0x10;		// Enable the AWU feature
 166  001a 721850f0      	bset	20720,#4
 167                     ; 61 }
 170  001e 81            	ret
 193                     ; 74 void AWU_DeInit(void)
 193                     ; 75 {
 194                     	switch	.text
 195  001f               _AWU_DeInit:
 199                     ; 76 	AWU->CSR = AWU_CSR_RESET_VALUE;
 201  001f 725f50f0      	clr	20720
 202                     ; 77 	AWU->APR = AWU_APR_RESET_VALUE;
 204  0023 353f50f1      	mov	20721,#63
 205                     ; 78 	AWU->TBR = AWU_TBR_RESET_VALUE;
 207  0027 725f50f2      	clr	20722
 208                     ; 79 }
 211  002b 81            	ret
 224                     	xdef	_AWU_DeInit
 225                     	xdef	_AWU_Config
 226                     	xdef	_AWU_GetFlagStatus
 227                     	xdef	_AWU_IRQHandler
 246                     	end
