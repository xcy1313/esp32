   1                     ; C Compiler for STM8 (COSMIC Software)
   2                     ; Generator V4.2.8 - 03 Dec 2008
  32                     ; 15 void MCU_ADC_Init(void)
  32                     ; 16 {
  34                     	switch	.text
  35  0000               _MCU_ADC_Init:
  39                     ; 18 	ADC1->CR2 = ADC1_ALIGN_LEFT;	// Configure the data alignment
  41  0000 725f5402      	clr	21506
  42                     ; 19 }
  45  0004 81            	ret
  98                     ; 32 u16 ADC_GetADValue(u8 u8AdCh)
  98                     ; 33 {
  99                     	switch	.text
 100  0005               _ADC_GetADValue:
 102  0005 5203          	subw	sp,#3
 103       00000003      OFST:	set	3
 106                     ; 37 	ADC1->CSR = u8AdCh;	
 108  0007 c75400        	ld	21504,a
 109                     ; 38 	ADC1->CR1 |= 0x01;
 111  000a 72105401      	bset	21505,#0
 112                     ; 39 	for(j=0;(j<250)&&((ADC1->CSR&0x80)==0); j++)	
 114  000e 0f03          	clr	(OFST+0,sp)
 116  0010 2003          	jra	L35
 117  0012               L74:
 118                     ; 41 		NOP;
 121  0012 9d            nop
 123                     ; 39 	for(j=0;(j<250)&&((ADC1->CSR&0x80)==0); j++)	
 125  0013 0c03          	inc	(OFST+0,sp)
 126  0015               L35:
 129  0015 7b03          	ld	a,(OFST+0,sp)
 130  0017 a1fa          	cp	a,#250
 131  0019 2407          	jruge	L75
 133  001b c65400        	ld	a,21504
 134  001e a580          	bcp	a,#128
 135  0020 27f0          	jreq	L74
 136  0022               L75:
 137                     ; 44 	temp = (ADC1->DRH);
 139  0022 c65404        	ld	a,21508
 140  0025 5f            	clrw	x
 141  0026 97            	ld	xl,a
 142  0027 1f01          	ldw	(OFST-2,sp),x
 143                     ; 45 	temp = (temp<<2);
 145  0029 0802          	sll	(OFST-1,sp)
 146  002b 0901          	rlc	(OFST-2,sp)
 147  002d 0802          	sll	(OFST-1,sp)
 148  002f 0901          	rlc	(OFST-2,sp)
 149                     ; 46 	temp = (temp | (ADC1->DRL&0x03));
 151  0031 c65405        	ld	a,21509
 152  0034 a403          	and	a,#3
 153  0036 5f            	clrw	x
 154  0037 97            	ld	xl,a
 155  0038 01            	rrwa	x,a
 156  0039 1a02          	or	a,(OFST-1,sp)
 157  003b 01            	rrwa	x,a
 158  003c 1a01          	or	a,(OFST-2,sp)
 159  003e 01            	rrwa	x,a
 160  003f 1f01          	ldw	(OFST-2,sp),x
 161                     ; 48 	return temp;
 163  0041 1e01          	ldw	x,(OFST-2,sp)
 166  0043 5b03          	addw	sp,#3
 167  0045 81            	ret
 190                     ; 62 @near @interrupt void ADC1_IRQHandler(void)
 190                     ; 63 {
 191                     	switch	.text
 192  0046               _ADC1_IRQHandler:
 196                     ; 64 	return;
 199  0046 80            	iret
 222                     ; 78 void ADC1_DeInit(void)
 222                     ; 79 {
 223                     	switch	.text
 224  0047               _ADC1_DeInit:
 228                     ; 80 	ADC1->CSR   = ADC1_CSR_RESET_VALUE;
 230  0047 725f5400      	clr	21504
 231                     ; 81 	ADC1->CR1   = ADC1_CR1_RESET_VALUE;
 233  004b 725f5401      	clr	21505
 234                     ; 82 	ADC1->CR2   = ADC1_CR2_RESET_VALUE;
 236  004f 725f5402      	clr	21506
 237                     ; 83 	ADC1->CR3   = ADC1_CR3_RESET_VALUE;
 239  0053 725f5403      	clr	21507
 240                     ; 84 	ADC1->TDRH  = ADC1_TDRH_RESET_VALUE;
 242  0057 725f5406      	clr	21510
 243                     ; 85 	ADC1->TDRL  = ADC1_TDRL_RESET_VALUE;
 245  005b 725f5407      	clr	21511
 246                     ; 86 	ADC1->HTRH  = ADC1_HTRH_RESET_VALUE;
 248  005f 35035409      	mov	21513,#3
 249                     ; 87 	ADC1->HTRL  = ADC1_HTRL_RESET_VALUE;
 251  0063 35ff5408      	mov	21512,#255
 252                     ; 88 	ADC1->LTRH  = ADC1_LTRH_RESET_VALUE;
 254  0067 725f540a      	clr	21514
 255                     ; 89 	ADC1->LTRL  = ADC1_LTRL_RESET_VALUE;
 257  006b 725f540b      	clr	21515
 258                     ; 90 	ADC1->AWCRH = ADC1_AWCRH_RESET_VALUE;
 260  006f 725f540e      	clr	21518
 261                     ; 91 	ADC1->AWCRL = ADC1_AWCRL_RESET_VALUE;
 263  0073 725f540f      	clr	21519
 264                     ; 92 }
 267  0077 81            	ret
 449                     ; 100 void ADC1_ConversionConfig(ADC1_ConvMode_TypeDef ADC1_ConversionMode, ADC1_Channel_TypeDef ADC1_Channel, ADC1_Align_TypeDef ADC1_Align)
 449                     ; 101 {
 450                     	switch	.text
 451  0078               _ADC1_ConversionConfig:
 453  0078 89            	pushw	x
 454       00000000      OFST:	set	0
 457                     ; 102 	ADC1->CR2 &= (u8)(~ADC1_CR2_ALIGN);		// Clear the align bit
 459  0079 72175402      	bres	21506,#3
 460                     ; 103 	ADC1->CR2 |= (u8)(ADC1_Align);			// Configure the data alignment
 462  007d c65402        	ld	a,21506
 463  0080 1a05          	or	a,(OFST+5,sp)
 464  0082 c75402        	ld	21506,a
 465                     ; 105 	if (ADC1_ConversionMode == ADC1_CONVERSIONMODE_CONTINUOUS)
 467  0085 9e            	ld	a,xh
 468  0086 a101          	cp	a,#1
 469  0088 2606          	jrne	L102
 470                     ; 108 		ADC1->CR1 |= ADC1_CR1_CONT;			// Set the continuous coversion mode
 472  008a 72125401      	bset	21505,#1
 474  008e 2004          	jra	L302
 475  0090               L102:
 476                     ; 112 		ADC1->CR1 &= (u8)(~ADC1_CR1_CONT);	// Set the single conversion mode
 478  0090 72135401      	bres	21505,#1
 479  0094               L302:
 480                     ; 115 	ADC1->CSR &= (u8)(~ADC1_CSR_CH);		// Clear the ADC1 channels
 482  0094 c65400        	ld	a,21504
 483  0097 a4f0          	and	a,#240
 484  0099 c75400        	ld	21504,a
 485                     ; 116 	ADC1->CSR |= (u8)(ADC1_Channel);		// Select the ADC1 channel
 487  009c c65400        	ld	a,21504
 488  009f 1a02          	or	a,(OFST+2,sp)
 489  00a1 c75400        	ld	21504,a
 490                     ; 117 }
 493  00a4 85            	popw	x
 494  00a5 81            	ret
 600                     ; 125 void ADC1_PrescalerConfig(ADC1_PresSel_TypeDef ADC1_Prescaler)
 600                     ; 126 {
 601                     	switch	.text
 602  00a6               _ADC1_PrescalerConfig:
 604  00a6 88            	push	a
 605       00000000      OFST:	set	0
 608                     ; 127 	ADC1->CR1 &= (u8)(~ADC1_CR1_SPSEL);	// Clear the SPSEL bits
 610  00a7 c65401        	ld	a,21505
 611  00aa a48f          	and	a,#143
 612  00ac c75401        	ld	21505,a
 613                     ; 128 	ADC1->CR1 |= (u8)(ADC1_Prescaler);	// Select the prescaler division factor according to ADC1_PrescalerSelection values
 615  00af c65401        	ld	a,21505
 616  00b2 1a01          	or	a,(OFST+1,sp)
 617  00b4 c75401        	ld	21505,a
 618                     ; 129 }
 621  00b7 84            	pop	a
 622  00b8 81            	ret
 708                     ; 137 void ADC1_ExternalTriggerConfig(ADC1_ExtTrig_TypeDef ADC1_ExtTrigger, FunctionalState NewState)
 708                     ; 138 {
 709                     	switch	.text
 710  00b9               _ADC1_ExternalTriggerConfig:
 712  00b9 89            	pushw	x
 713       00000000      OFST:	set	0
 716                     ; 139 	ADC1->CR2 &= (u8)(~ADC1_CR2_EXTSEL);		// Clear the external trigger selection bits
 718  00ba c65402        	ld	a,21506
 719  00bd a4cf          	and	a,#207
 720  00bf c75402        	ld	21506,a
 721                     ; 141 	if (NewState != DISABLE)
 723  00c2 9f            	ld	a,xl
 724  00c3 4d            	tnz	a
 725  00c4 2706          	jreq	L113
 726                     ; 143 		ADC1->CR2 |= (u8)(ADC1_CR2_EXTTRIG);	// Enable the selected external Trigger
 728  00c6 721c5402      	bset	21506,#6
 730  00ca 2004          	jra	L313
 731  00cc               L113:
 732                     ; 147 		ADC1->CR2 &= (u8)(~ADC1_CR2_EXTTRIG);	// Disable the selected external trigger
 734  00cc 721d5402      	bres	21506,#6
 735  00d0               L313:
 736                     ; 150 	ADC1->CR2 |= (u8)(ADC1_ExtTrigger);			// Set the selected external trigger
 738  00d0 c65402        	ld	a,21506
 739  00d3 1a01          	or	a,(OFST+1,sp)
 740  00d5 c75402        	ld	21506,a
 741                     ; 151 }
 744  00d8 85            	popw	x
 745  00d9 81            	ret
 894                     ; 159 void ADC1_SchmittTriggerConfig(ADC1_SchmittTrigg_TypeDef ADC1_SchmittTriggerChannel, FunctionalState NewState)
 894                     ; 160 {
 895                     	switch	.text
 896  00da               _ADC1_SchmittTriggerConfig:
 898  00da 89            	pushw	x
 899       00000000      OFST:	set	0
 902                     ; 161 	if (ADC1_SchmittTriggerChannel == ADC1_SCHMITTTRIG_ALL)
 904  00db 9e            	ld	a,xh
 905  00dc a1ff          	cp	a,#255
 906  00de 2620          	jrne	L373
 907                     ; 163 		if (NewState != DISABLE)
 909  00e0 9f            	ld	a,xl
 910  00e1 4d            	tnz	a
 911  00e2 270a          	jreq	L573
 912                     ; 165 			ADC1->TDRL &= (u8)0x0;
 914  00e4 725f5407      	clr	21511
 915                     ; 166 			ADC1->TDRH &= (u8)0x0;
 917  00e8 725f5406      	clr	21510
 919  00ec 2078          	jra	L104
 920  00ee               L573:
 921                     ; 170 			ADC1->TDRL |= (u8)0xFF;
 923  00ee c65407        	ld	a,21511
 924  00f1 aaff          	or	a,#255
 925  00f3 c75407        	ld	21511,a
 926                     ; 171 			ADC1->TDRH |= (u8)0xFF;
 928  00f6 c65406        	ld	a,21510
 929  00f9 aaff          	or	a,#255
 930  00fb c75406        	ld	21510,a
 931  00fe 2066          	jra	L104
 932  0100               L373:
 933                     ; 174 	else if (ADC1_SchmittTriggerChannel < ADC1_SCHMITTTRIG_CHANNEL8)
 935  0100 7b01          	ld	a,(OFST+1,sp)
 936  0102 a108          	cp	a,#8
 937  0104 242f          	jruge	L304
 938                     ; 176 		if (NewState != DISABLE)
 940  0106 0d02          	tnz	(OFST+2,sp)
 941  0108 2716          	jreq	L504
 942                     ; 178 			ADC1->TDRL &= (u8)(~(u8)((u8)0x01 << (u8)ADC1_SchmittTriggerChannel));
 944  010a 7b01          	ld	a,(OFST+1,sp)
 945  010c 5f            	clrw	x
 946  010d 97            	ld	xl,a
 947  010e a601          	ld	a,#1
 948  0110 5d            	tnzw	x
 949  0111 2704          	jreq	L42
 950  0113               L62:
 951  0113 48            	sll	a
 952  0114 5a            	decw	x
 953  0115 26fc          	jrne	L62
 954  0117               L42:
 955  0117 43            	cpl	a
 956  0118 c45407        	and	a,21511
 957  011b c75407        	ld	21511,a
 959  011e 2046          	jra	L104
 960  0120               L504:
 961                     ; 182 			ADC1->TDRL |= (u8)((u8)0x01 << (u8)ADC1_SchmittTriggerChannel);
 963  0120 7b01          	ld	a,(OFST+1,sp)
 964  0122 5f            	clrw	x
 965  0123 97            	ld	xl,a
 966  0124 a601          	ld	a,#1
 967  0126 5d            	tnzw	x
 968  0127 2704          	jreq	L03
 969  0129               L23:
 970  0129 48            	sll	a
 971  012a 5a            	decw	x
 972  012b 26fc          	jrne	L23
 973  012d               L03:
 974  012d ca5407        	or	a,21511
 975  0130 c75407        	ld	21511,a
 976  0133 2031          	jra	L104
 977  0135               L304:
 978                     ; 187 		if (NewState != DISABLE)
 980  0135 0d02          	tnz	(OFST+2,sp)
 981  0137 2718          	jreq	L314
 982                     ; 189 			ADC1->TDRH &= (u8)(~(u8)((u8)0x01 << ((u8)ADC1_SchmittTriggerChannel - (u8)8)));
 984  0139 7b01          	ld	a,(OFST+1,sp)
 985  013b a008          	sub	a,#8
 986  013d 5f            	clrw	x
 987  013e 97            	ld	xl,a
 988  013f a601          	ld	a,#1
 989  0141 5d            	tnzw	x
 990  0142 2704          	jreq	L43
 991  0144               L63:
 992  0144 48            	sll	a
 993  0145 5a            	decw	x
 994  0146 26fc          	jrne	L63
 995  0148               L43:
 996  0148 43            	cpl	a
 997  0149 c45406        	and	a,21510
 998  014c c75406        	ld	21510,a
1000  014f 2015          	jra	L104
1001  0151               L314:
1002                     ; 193 			ADC1->TDRH |= (u8)((u8)0x01 << ((u8)ADC1_SchmittTriggerChannel - (u8)8));
1004  0151 7b01          	ld	a,(OFST+1,sp)
1005  0153 a008          	sub	a,#8
1006  0155 5f            	clrw	x
1007  0156 97            	ld	xl,a
1008  0157 a601          	ld	a,#1
1009  0159 5d            	tnzw	x
1010  015a 2704          	jreq	L04
1011  015c               L24:
1012  015c 48            	sll	a
1013  015d 5a            	decw	x
1014  015e 26fc          	jrne	L24
1015  0160               L04:
1016  0160 ca5406        	or	a,21510
1017  0163 c75406        	ld	21510,a
1018  0166               L104:
1019                     ; 196 }
1022  0166 85            	popw	x
1023  0167 81            	ret
1058                     ; 204 void ADC1_DataBufferCmd(FunctionalState NewState)
1058                     ; 205 {
1059                     	switch	.text
1060  0168               _ADC1_DataBufferCmd:
1064                     ; 206 	if (NewState != DISABLE)
1066  0168 4d            	tnz	a
1067  0169 2706          	jreq	L534
1068                     ; 208 		ADC1->CR3 |= ADC1_CR3_DBUF;
1070  016b 721e5403      	bset	21507,#7
1072  016f 2004          	jra	L734
1073  0171               L534:
1074                     ; 212 		ADC1->CR3 &= (u8)(~ADC1_CR3_DBUF);
1076  0171 721f5403      	bres	21507,#7
1077  0175               L734:
1078                     ; 214 }
1081  0175 81            	ret
1116                     ; 222 void ADC1_Cmd(FunctionalState NewState)
1116                     ; 223 {
1117                     	switch	.text
1118  0176               _ADC1_Cmd:
1122                     ; 224 	if (NewState != DISABLE)
1124  0176 4d            	tnz	a
1125  0177 2706          	jreq	L754
1126                     ; 226 		ADC1->CR1 |= ADC1_CR1_ADON;
1128  0179 72105401      	bset	21505,#0
1130  017d 2004          	jra	L164
1131  017f               L754:
1132                     ; 230 		ADC1->CR1 &= (u8)(~ADC1_CR1_ADON);
1134  017f 72115401      	bres	21505,#0
1135  0183               L164:
1136                     ; 232 }
1139  0183 81            	ret
1174                     ; 240 void ADC1_ScanModeCmd(FunctionalState NewState)
1174                     ; 241 {
1175                     	switch	.text
1176  0184               _ADC1_ScanModeCmd:
1180                     ; 242 	if (NewState != DISABLE)
1182  0184 4d            	tnz	a
1183  0185 2706          	jreq	L105
1184                     ; 244 		ADC1->CR2 |= ADC1_CR2_SCAN;
1186  0187 72125402      	bset	21506,#1
1188  018b 2004          	jra	L305
1189  018d               L105:
1190                     ; 248 		ADC1->CR2 &= (u8)(~ADC1_CR2_SCAN);
1192  018d 72135402      	bres	21506,#1
1193  0191               L305:
1194                     ; 250 }
1197  0191 81            	ret
1250                     ; 258 u16 ADC1_GetBufferValue(u8 Buffer)
1250                     ; 259 {
1251                     	switch	.text
1252  0192               _ADC1_GetBufferValue:
1254  0192 88            	push	a
1255  0193 5205          	subw	sp,#5
1256       00000005      OFST:	set	5
1259                     ; 260 	u16 temph = 0;
1261  0195 1e04          	ldw	x,(OFST-1,sp)
1262                     ; 261 	u8 templ = 0;
1264  0197 7b03          	ld	a,(OFST-2,sp)
1265  0199 97            	ld	xl,a
1266                     ; 263 	if((ADC1->CR2 & ADC1_CR2_ALIGN) != 0)	// Right alignment
1268  019a c65402        	ld	a,21506
1269  019d a508          	bcp	a,#8
1270  019f 2723          	jreq	L335
1271                     ; 265 		templ = *(u8*)(u16)((u16)ADC1_BaseAddress + (u8)(Buffer << 1) + 1);	// Read LSB first
1273  01a1 7b06          	ld	a,(OFST+1,sp)
1274  01a3 48            	sll	a
1275  01a4 5f            	clrw	x
1276  01a5 97            	ld	xl,a
1277  01a6 d653e1        	ld	a,(21473,x)
1278  01a9 6b03          	ld	(OFST-2,sp),a
1279                     ; 266 		temph = *(u8*)(u16)((u16)ADC1_BaseAddress + (u8)(Buffer << 1));		// Then read MSB
1281  01ab 7b06          	ld	a,(OFST+1,sp)
1282  01ad 48            	sll	a
1283  01ae 5f            	clrw	x
1284  01af 97            	ld	xl,a
1285  01b0 d653e0        	ld	a,(21472,x)
1286  01b3 5f            	clrw	x
1287  01b4 97            	ld	xl,a
1288  01b5 1f04          	ldw	(OFST-1,sp),x
1289                     ; 267 		temph = (u16)(templ | (u16)(temph << (u8)8));
1291  01b7 1e04          	ldw	x,(OFST-1,sp)
1292  01b9 4f            	clr	a
1293  01ba 02            	rlwa	x,a
1294  01bb 01            	rrwa	x,a
1295  01bc 1a03          	or	a,(OFST-2,sp)
1296  01be 02            	rlwa	x,a
1297  01bf 1f04          	ldw	(OFST-1,sp),x
1298  01c1 01            	rrwa	x,a
1300  01c2 202b          	jra	L535
1301  01c4               L335:
1302                     ; 271 		temph = *(u8*)(u16)((u16)ADC1_BaseAddress + (u8)(Buffer << 1));		// Read MSB firts
1304  01c4 7b06          	ld	a,(OFST+1,sp)
1305  01c6 48            	sll	a
1306  01c7 5f            	clrw	x
1307  01c8 97            	ld	xl,a
1308  01c9 d653e0        	ld	a,(21472,x)
1309  01cc 5f            	clrw	x
1310  01cd 97            	ld	xl,a
1311  01ce 1f04          	ldw	(OFST-1,sp),x
1312                     ; 272 		templ = *(u8*)(u16)((u16)ADC1_BaseAddress + (u8)(Buffer << 1) + 1);	// Then read LSB
1314  01d0 7b06          	ld	a,(OFST+1,sp)
1315  01d2 48            	sll	a
1316  01d3 5f            	clrw	x
1317  01d4 97            	ld	xl,a
1318  01d5 d653e1        	ld	a,(21473,x)
1319  01d8 6b03          	ld	(OFST-2,sp),a
1320                     ; 273 		temph = (u16)((u16)((u16)templ << 6) | (u16)(temph << 8));
1322  01da 1e04          	ldw	x,(OFST-1,sp)
1323  01dc 4f            	clr	a
1324  01dd 02            	rlwa	x,a
1325  01de 1f01          	ldw	(OFST-4,sp),x
1326  01e0 7b03          	ld	a,(OFST-2,sp)
1327  01e2 97            	ld	xl,a
1328  01e3 a640          	ld	a,#64
1329  01e5 42            	mul	x,a
1330  01e6 01            	rrwa	x,a
1331  01e7 1a02          	or	a,(OFST-3,sp)
1332  01e9 01            	rrwa	x,a
1333  01ea 1a01          	or	a,(OFST-4,sp)
1334  01ec 01            	rrwa	x,a
1335  01ed 1f04          	ldw	(OFST-1,sp),x
1336  01ef               L535:
1337                     ; 276 	return ((u16)temph);
1339  01ef 1e04          	ldw	x,(OFST-1,sp)
1342  01f1 5b06          	addw	sp,#6
1343  01f3 81            	ret
1499                     ; 285 void ADC1_ITConfig(ADC1_IT_TypeDef ADC1_IT, FunctionalState NewState)
1499                     ; 286 {
1500                     	switch	.text
1501  01f4               _ADC1_ITConfig:
1503  01f4 89            	pushw	x
1504       00000000      OFST:	set	0
1507                     ; 287 	if (NewState != DISABLE)
1509  01f5 0d05          	tnz	(OFST+5,sp)
1510  01f7 2709          	jreq	L326
1511                     ; 289 		ADC1->CSR |= (u8)ADC1_IT;				// Enable the ADC1 interrupts
1513  01f9 9f            	ld	a,xl
1514  01fa ca5400        	or	a,21504
1515  01fd c75400        	ld	21504,a
1517  0200 2009          	jra	L526
1518  0202               L326:
1519                     ; 293 		ADC1->CSR &= (u8)((u16)~(u16)ADC1_IT);	// Disable the ADC1 interrupts
1521  0202 7b02          	ld	a,(OFST+2,sp)
1522  0204 43            	cpl	a
1523  0205 c45400        	and	a,21504
1524  0208 c75400        	ld	21504,a
1525  020b               L526:
1526                     ; 295 }
1529  020b 85            	popw	x
1530  020c 81            	ret
1575                     ; 303 void ADC1_ClearITPendingBit(ADC1_IT_TypeDef ITPendingBit)
1575                     ; 304 {
1576                     	switch	.text
1577  020d               _ADC1_ClearITPendingBit:
1579  020d 89            	pushw	x
1580  020e 88            	push	a
1581       00000001      OFST:	set	1
1584                     ; 305 	u8 temp = 0;
1586  020f 0f01          	clr	(OFST+0,sp)
1587                     ; 307 	if (((u16)ITPendingBit & 0xF0) == 0x10)
1589  0211 01            	rrwa	x,a
1590  0212 a4f0          	and	a,#240
1591  0214 5f            	clrw	x
1592  0215 02            	rlwa	x,a
1593  0216 a30010        	cpw	x,#16
1594  0219 263a          	jrne	L156
1595                     ; 309 		temp = (u8)((u16)ITPendingBit & 0x0F);	// Clear analog watchdog channel status
1597  021b 7b03          	ld	a,(OFST+2,sp)
1598  021d a40f          	and	a,#15
1599  021f 6b01          	ld	(OFST+0,sp),a
1600                     ; 310 		if (temp < 8)
1602  0221 7b01          	ld	a,(OFST+0,sp)
1603  0223 a108          	cp	a,#8
1604  0225 2416          	jruge	L356
1605                     ; 312 			ADC1->AWSRL &= (u8)~(u8)((u8)1 << temp);
1607  0227 7b01          	ld	a,(OFST+0,sp)
1608  0229 5f            	clrw	x
1609  022a 97            	ld	xl,a
1610  022b a601          	ld	a,#1
1611  022d 5d            	tnzw	x
1612  022e 2704          	jreq	L06
1613  0230               L26:
1614  0230 48            	sll	a
1615  0231 5a            	decw	x
1616  0232 26fc          	jrne	L26
1617  0234               L06:
1618  0234 43            	cpl	a
1619  0235 c4540d        	and	a,21517
1620  0238 c7540d        	ld	21517,a
1622  023b 2021          	jra	L756
1623  023d               L356:
1624                     ; 316 			ADC1->AWSRH &= (u8)~(u8)((u8)1 << (temp - 8));
1626  023d 7b01          	ld	a,(OFST+0,sp)
1627  023f a008          	sub	a,#8
1628  0241 5f            	clrw	x
1629  0242 97            	ld	xl,a
1630  0243 a601          	ld	a,#1
1631  0245 5d            	tnzw	x
1632  0246 2704          	jreq	L46
1633  0248               L66:
1634  0248 48            	sll	a
1635  0249 5a            	decw	x
1636  024a 26fc          	jrne	L66
1637  024c               L46:
1638  024c 43            	cpl	a
1639  024d c4540c        	and	a,21516
1640  0250 c7540c        	ld	21516,a
1641  0253 2009          	jra	L756
1642  0255               L156:
1643                     ; 321 		ADC1->CSR &= (u8)((u16)~(u16)ITPendingBit);  // Clear EOC | AWD flag status
1645  0255 7b03          	ld	a,(OFST+2,sp)
1646  0257 43            	cpl	a
1647  0258 c45400        	and	a,21504
1648  025b c75400        	ld	21504,a
1649  025e               L756:
1650                     ; 323 }
1653  025e 5b03          	addw	sp,#3
1654  0260 81            	ret
1667                     	xdef	_ADC1_ClearITPendingBit
1668                     	xdef	_ADC1_ITConfig
1669                     	xdef	_ADC1_GetBufferValue
1670                     	xdef	_ADC1_ScanModeCmd
1671                     	xdef	_ADC1_Cmd
1672                     	xdef	_ADC1_DataBufferCmd
1673                     	xdef	_ADC1_SchmittTriggerConfig
1674                     	xdef	_ADC1_ExternalTriggerConfig
1675                     	xdef	_ADC1_PrescalerConfig
1676                     	xdef	_ADC1_ConversionConfig
1677                     	xdef	_ADC1_DeInit
1678                     	xdef	_ADC1_IRQHandler
1679                     	xdef	_ADC_GetADValue
1680                     	xdef	_MCU_ADC_Init
1699                     	end
