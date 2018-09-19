   1                     ; C Compiler for STM8 (COSMIC Software)
   2                     ; Generator V4.2.8 - 03 Dec 2008
  34                     ; 32 @near @interrupt void NonHandledInterrupt(void)
  34                     ; 33 {
  36                     	switch	.text
  37  0000               _NonHandledInterrupt:
  41                     ; 37   return;
  44  0000 80            	iret
  47                     .const:	section	.text
  48  0000               __vectab:
  49  0000 82            	dc.b	130
  51  0001 00            	dc.b	page(__stext)
  52  0002 0000          	dc.w	__stext
  53  0004 82            	dc.b	130
  55  0005 00            	dc.b	page(_NonHandledInterrupt)
  56  0006 0000          	dc.w	_NonHandledInterrupt
  57  0008 82            	dc.b	130
  59  0009 00            	dc.b	page(_NonHandledInterrupt)
  60  000a 0000          	dc.w	_NonHandledInterrupt
  61  000c 82            	dc.b	130
  63  000d 00            	dc.b	page(_AWU_IRQHandler)
  64  000e 0000          	dc.w	_AWU_IRQHandler
  65  0010 82            	dc.b	130
  67  0011 00            	dc.b	page(_NonHandledInterrupt)
  68  0012 0000          	dc.w	_NonHandledInterrupt
  69  0014 82            	dc.b	130
  71  0015 00            	dc.b	page(_EXTI_PORTA_IRQHandler)
  72  0016 0000          	dc.w	_EXTI_PORTA_IRQHandler
  73  0018 82            	dc.b	130
  75  0019 00            	dc.b	page(_EXTI_PORTB_IRQHandler)
  76  001a 0000          	dc.w	_EXTI_PORTB_IRQHandler
  77  001c 82            	dc.b	130
  79  001d 00            	dc.b	page(_EXTI_PORTC_IRQHandler)
  80  001e 0000          	dc.w	_EXTI_PORTC_IRQHandler
  81  0020 82            	dc.b	130
  83  0021 00            	dc.b	page(_EXTI_PORTD_IRQHandler)
  84  0022 0000          	dc.w	_EXTI_PORTD_IRQHandler
  85  0024 82            	dc.b	130
  87  0025 00            	dc.b	page(_EXTI_PORTE_IRQHandler)
  88  0026 0000          	dc.w	_EXTI_PORTE_IRQHandler
  89  0028 82            	dc.b	130
  91  0029 00            	dc.b	page(_NonHandledInterrupt)
  92  002a 0000          	dc.w	_NonHandledInterrupt
  93  002c 82            	dc.b	130
  95  002d 00            	dc.b	page(_NonHandledInterrupt)
  96  002e 0000          	dc.w	_NonHandledInterrupt
  97  0030 82            	dc.b	130
  99  0031 00            	dc.b	page(_NonHandledInterrupt)
 100  0032 0000          	dc.w	_NonHandledInterrupt
 101  0034 82            	dc.b	130
 103  0035 00            	dc.b	page(_NonHandledInterrupt)
 104  0036 0000          	dc.w	_NonHandledInterrupt
 105  0038 82            	dc.b	130
 107  0039 00            	dc.b	page(_NonHandledInterrupt)
 108  003a 0000          	dc.w	_NonHandledInterrupt
 109  003c 82            	dc.b	130
 111  003d 00            	dc.b	page(_TIM2_UPD_OVF_BRK_IRQHandler)
 112  003e 0000          	dc.w	_TIM2_UPD_OVF_BRK_IRQHandler
 113  0040 82            	dc.b	130
 115  0041 00            	dc.b	page(_TIM2_CAP_COM_IRQHandler)
 116  0042 0000          	dc.w	_TIM2_CAP_COM_IRQHandler
 117  0044 82            	dc.b	130
 119  0045 00            	dc.b	page(_NonHandledInterrupt)
 120  0046 0000          	dc.w	_NonHandledInterrupt
 121  0048 82            	dc.b	130
 123  0049 00            	dc.b	page(_NonHandledInterrupt)
 124  004a 0000          	dc.w	_NonHandledInterrupt
 125  004c 82            	dc.b	130
 127  004d 00            	dc.b	page(_NonHandledInterrupt)
 128  004e 0000          	dc.w	_NonHandledInterrupt
 129  0050 82            	dc.b	130
 131  0051 00            	dc.b	page(_NonHandledInterrupt)
 132  0052 0000          	dc.w	_NonHandledInterrupt
 133  0054 82            	dc.b	130
 135  0055 00            	dc.b	page(_NonHandledInterrupt)
 136  0056 0000          	dc.w	_NonHandledInterrupt
 137  0058 82            	dc.b	130
 139  0059 00            	dc.b	page(_NonHandledInterrupt)
 140  005a 0000          	dc.w	_NonHandledInterrupt
 141  005c 82            	dc.b	130
 143  005d 00            	dc.b	page(_NonHandledInterrupt)
 144  005e 0000          	dc.w	_NonHandledInterrupt
 145  0060 82            	dc.b	130
 147  0061 00            	dc.b	page(_NonHandledInterrupt)
 148  0062 0000          	dc.w	_NonHandledInterrupt
 149  0064 82            	dc.b	130
 151  0065 00            	dc.b	page(_TIM4_UPD_OVF_IRQHandler)
 152  0066 0000          	dc.w	_TIM4_UPD_OVF_IRQHandler
 153  0068 82            	dc.b	130
 155  0069 00            	dc.b	page(_NonHandledInterrupt)
 156  006a 0000          	dc.w	_NonHandledInterrupt
 157  006c 82            	dc.b	130
 159  006d 00            	dc.b	page(_NonHandledInterrupt)
 160  006e 0000          	dc.w	_NonHandledInterrupt
 161  0070 82            	dc.b	130
 163  0071 00            	dc.b	page(_NonHandledInterrupt)
 164  0072 0000          	dc.w	_NonHandledInterrupt
 165  0074 82            	dc.b	130
 167  0075 00            	dc.b	page(_NonHandledInterrupt)
 168  0076 0000          	dc.w	_NonHandledInterrupt
 169  0078 82            	dc.b	130
 171  0079 00            	dc.b	page(_NonHandledInterrupt)
 172  007a 0000          	dc.w	_NonHandledInterrupt
 173  007c 82            	dc.b	130
 175  007d 00            	dc.b	page(_NonHandledInterrupt)
 176  007e 0000          	dc.w	_NonHandledInterrupt
 227                     	xdef	__vectab
 228                     	xref	__stext
 229                     	xdef	_NonHandledInterrupt
 230                     	xref	_TIM4_UPD_OVF_IRQHandler
 231                     	xref	_TIM2_CAP_COM_IRQHandler
 232                     	xref	_TIM2_UPD_OVF_BRK_IRQHandler
 233                     	xref	_EXTI_PORTE_IRQHandler
 234                     	xref	_EXTI_PORTD_IRQHandler
 235                     	xref	_EXTI_PORTC_IRQHandler
 236                     	xref	_EXTI_PORTB_IRQHandler
 237                     	xref	_EXTI_PORTA_IRQHandler
 238                     	xref	_AWU_IRQHandler
 257                     	end
