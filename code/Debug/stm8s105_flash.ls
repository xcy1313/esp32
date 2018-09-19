   1                     ; C Compiler for STM8 (COSMIC Software)
   2                     ; Generator V4.2.8 - 03 Dec 2008
  34                     ; 10 void MCU_Flash_Init(void)
  34                     ; 11 {
  36                     	switch	.text
  37  0000               _MCU_Flash_Init:
  41                     ; 12 	FLASH_SetProgrammingTime(FLASH_PROGRAMTIME_STANDARD);
  43  0000 4f            	clr	a
  44  0001 ad05          	call	_FLASH_SetProgrammingTime
  46                     ; 13 	FLASH_Unlock(FLASH_MEMTYPE_DATA);
  48  0003 a6f7          	ld	a,#247
  49  0005 ad0c          	call	_FLASH_Unlock
  51                     ; 14 }
  54  0007 81            	ret
 112                     ; 22 void FLASH_SetProgrammingTime(FLASH_ProgramTime_TypeDef FLASH_ProgTime)
 112                     ; 23 {
 113                     	switch	.text
 114  0008               _FLASH_SetProgrammingTime:
 118                     ; 24 	FLASH->CR1 &= (u8)(~FLASH_CR1_FIX);
 120  0008 7211505a      	bres	20570,#0
 121                     ; 25 	FLASH->CR1 |= (u8)FLASH_ProgTime;
 123  000c ca505a        	or	a,20570
 124  000f c7505a        	ld	20570,a
 125                     ; 26 }
 128  0012 81            	ret
 183                     ; 34 void FLASH_Unlock(FLASH_MemType_TypeDef FLASH_MemType)
 183                     ; 35 {
 184                     	switch	.text
 185  0013               _FLASH_Unlock:
 189                     ; 36 	if (FLASH_MemType == FLASH_MEMTYPE_PROG)	// Unlock program memory
 191  0013 a1fd          	cp	a,#253
 192  0015 260a          	jrne	L57
 193                     ; 38 		FLASH->PUKR = FLASH_RASS_KEY1;
 195  0017 35565062      	mov	20578,#86
 196                     ; 39 		FLASH->PUKR = FLASH_RASS_KEY2;
 198  001b 35ae5062      	mov	20578,#174
 200  001f 2008          	jra	L77
 201  0021               L57:
 202                     ; 43 		FLASH->DUKR = FLASH_RASS_KEY2;	// Warning: keys are reversed on data memory !!!
 204  0021 35ae5064      	mov	20580,#174
 205                     ; 44 		FLASH->DUKR = FLASH_RASS_KEY1;
 207  0025 35565064      	mov	20580,#86
 208  0029               L77:
 209                     ; 46 }
 212  0029 81            	ret
 246                     ; 54 u8 FLASH_ReadByte(u32 Address)
 246                     ; 55 {
 247                     	switch	.text
 248  002a               _FLASH_ReadByte:
 250       00000000      OFST:	set	0
 253                     ; 56 	return(*(PointerAttr u8 *) (u16)Address); 
 255  002a 7b05          	ld	a,(OFST+5,sp)
 256  002c 97            	ld	xl,a
 257  002d 7b06          	ld	a,(OFST+6,sp)
 258  002f 3f00          	clr	c_x
 259  0031 02            	rlwa	x,a
 260  0032 9093          	ldw	y,x
 261  0034 bf01          	ldw	c_x+1,x
 262  0036 92bc0000      	ldf	a,[c_x.e]
 265  003a 81            	ret
 308                     ; 65 void FLASH_ProgramByte(u32 Address, u8 Data)
 308                     ; 66 {
 309                     	switch	.text
 310  003b               _FLASH_ProgramByte:
 312       00000000      OFST:	set	0
 315                     ; 67 	*(PointerAttr u8*) (u16)Address = Data;
 317  003b 7b05          	ld	a,(OFST+5,sp)
 318  003d 97            	ld	xl,a
 319  003e 7b06          	ld	a,(OFST+6,sp)
 320  0040 3f00          	clr	c_x
 321  0042 02            	rlwa	x,a
 322  0043 9093          	ldw	y,x
 323  0045 7b07          	ld	a,(OFST+7,sp)
 324  0047 bf01          	ldw	c_x+1,x
 325  0049 92bd0000      	ldf	[c_x.e],a
 326                     ; 68 }
 329  004d 81            	ret
 342                     	xdef	_FLASH_ProgramByte
 343                     	xdef	_FLASH_ReadByte
 344                     	xdef	_FLASH_Unlock
 345                     	xdef	_FLASH_SetProgrammingTime
 346                     	xdef	_MCU_Flash_Init
 347                     	xref.b	c_x
 366                     	end
