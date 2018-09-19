   1                     ; C Compiler for STM8 (COSMIC Software)
   2                     ; Generator V4.2.8 - 03 Dec 2008
  71                     ; 46 void p_dr11_nop(void)
  71                     ; 47 {
  73                     	switch	.text
  74  0000               _p_dr11_nop:
  76  0000 88            	push	a
  77       00000001      OFST:	set	1
  80                     ; 49 	for(i=0;i<2;i++)
  82  0001 0f01          	clr	(OFST+0,sp)
  83  0003               L34:
  87  0003 0c01          	inc	(OFST+0,sp)
  90  0005 7b01          	ld	a,(OFST+0,sp)
  91  0007 a102          	cp	a,#2
  92  0009 25f8          	jrult	L34
  93                     ; 54 }
  96  000b 84            	pop	a
  97  000c 81            	ret
 132                     ; 61 void p_dr11_I2cStart(void)
 132                     ; 62 {
 133                     	switch	.text
 134  000d               _p_dr11_I2cStart:
 136  000d 88            	push	a
 137       00000001      OFST:	set	1
 140                     ; 63 	u8 i=2;
 142  000e a602          	ld	a,#2
 143  0010 6b01          	ld	(OFST+0,sp),a
 144                     ; 64 	Write_P_IIC_SDA_1;
 146  0012 721a5005      	bset	20485,#5
 147                     ; 65 	p_dr11_nop();
 149  0016 ade8          	call	_p_dr11_nop
 151                     ; 66 	Write_P_IIC_SCL_1;
 153  0018 72185005      	bset	20485,#4
 155  001c 2002          	jra	L37
 156  001e               L76:
 157                     ; 70 		p_dr11_nop();
 159  001e ade0          	call	_p_dr11_nop
 161  0020               L37:
 162                     ; 68 	while(i--)			//wait i2c stable time
 164  0020 7b01          	ld	a,(OFST+0,sp)
 165  0022 0a01          	dec	(OFST+0,sp)
 166  0024 4d            	tnz	a
 167  0025 26f7          	jrne	L76
 168                     ; 73 	Write_P_IIC_SDA_0;
 170  0027 721b5005      	bres	20485,#5
 171                     ; 74 	p_dr11_nop();
 173  002b add3          	call	_p_dr11_nop
 175                     ; 75 	p_dr11_nop();
 177  002d add1          	call	_p_dr11_nop
 179                     ; 76 	Write_P_IIC_SCL_0;
 181  002f 72195005      	bres	20485,#4
 182                     ; 77 	p_dr11_nop();
 184  0033 adcb          	call	_p_dr11_nop
 186                     ; 78 }
 189  0035 84            	pop	a
 190  0036 81            	ret
 214                     ; 84 void p_dr11_I2cStop(void)
 214                     ; 85 {
 215                     	switch	.text
 216  0037               _p_dr11_I2cStop:
 220                     ; 86 	Write_P_IIC_SDA_0;
 222  0037 721b5005      	bres	20485,#5
 223                     ; 87 	p_dr11_nop();
 225  003b adc3          	call	_p_dr11_nop
 227                     ; 88 	Write_P_IIC_SCL_1;
 229  003d 72185005      	bset	20485,#4
 230                     ; 89 	p_dr11_nop();
 232  0041 adbd          	call	_p_dr11_nop
 234                     ; 90 	Write_P_IIC_SDA_1;;
 236  0043 721a5005      	bset	20485,#5
 237                     ; 91 	p_dr11_nop();
 240  0047 adb7          	call	_p_dr11_nop
 242                     ; 92 	p_dr11_nop();
 244  0049 adb5          	call	_p_dr11_nop
 246                     ; 93 }
 249  004b 81            	ret
 284                     ; 95 u8 p_dr11_CheckAck(void)
 284                     ; 96 {
 285                     	switch	.text
 286  004c               _p_dr11_CheckAck:
 288  004c 88            	push	a
 289       00000001      OFST:	set	1
 292                     ; 97 	u8 Flag=0;
 294  004d 0f01          	clr	(OFST+0,sp)
 295                     ; 99 	p_dr11_nop();	
 297  004f adaf          	call	_p_dr11_nop
 299                     ; 100 	Write_P_IIC_SCL_1;
 301  0051 72185005      	bset	20485,#4
 302                     ; 101 	p_dr11_nop();
 304  0055 ada9          	call	_p_dr11_nop
 306                     ; 103 	Flag = (u8)(PIDR_SDA);
 308  0057 4f            	clr	a
 309                     	btst	20486,#5
 310  005d 49            	rlc	a
 311  005e 6b01          	ld	(OFST+0,sp),a
 312                     ; 104 	p_dr11_nop();
 314  0060 ad9e          	call	_p_dr11_nop
 316                     ; 105 	p_dr11_nop();	
 318  0062 ad9c          	call	_p_dr11_nop
 320                     ; 106 	Write_P_IIC_SCL_0;
 322  0064 72195005      	bres	20485,#4
 323                     ; 107 	p_dr11_nop();
 325  0068 ad96          	call	_p_dr11_nop
 327                     ; 109 	PDDR_IIC_SDA = 1;
 329  006a 721a5007      	bset	20487,#5
 330                     ; 110 	if(Flag == 1)
 332  006e 7b01          	ld	a,(OFST+0,sp)
 333  0070 a101          	cp	a,#1
 334  0072 2604          	jrne	L521
 335                     ; 111 		return 0;
 337  0074 4f            	clr	a
 340  0075 5b01          	addw	sp,#1
 341  0077 81            	ret
 342  0078               L521:
 343                     ; 113 		return 1;	
 345  0078 a601          	ld	a,#1
 348  007a 5b01          	addw	sp,#1
 349  007c 81            	ret
 373                     ; 117 void p_dr11_Send_0(void)
 373                     ; 118 {
 374                     	switch	.text
 375  007d               _p_dr11_Send_0:
 379                     ; 119 	Write_P_IIC_SDA_0;
 381  007d 721b5005      	bres	20485,#5
 382                     ; 120 	p_dr11_nop();
 384  0081 cd0000        	call	_p_dr11_nop
 386                     ; 121 	Write_P_IIC_SCL_1;
 388  0084 72185005      	bset	20485,#4
 389                     ; 122 	p_dr11_nop();
 391  0088 cd0000        	call	_p_dr11_nop
 393                     ; 123 	p_dr11_nop();		
 395  008b cd0000        	call	_p_dr11_nop
 397                     ; 124 	p_dr11_nop();	
 399  008e cd0000        	call	_p_dr11_nop
 401                     ; 125 	Write_P_IIC_SCL_0;
 403  0091 72195005      	bres	20485,#4
 404                     ; 127 }
 407  0095 81            	ret
 431                     ; 128 void p_dr11_Send_1(void)
 431                     ; 129 {
 432                     	switch	.text
 433  0096               _p_dr11_Send_1:
 437                     ; 130 	Write_P_IIC_SDA_1;
 439  0096 721a5005      	bset	20485,#5
 440                     ; 131 	p_dr11_nop();
 442  009a cd0000        	call	_p_dr11_nop
 444                     ; 132 	Write_P_IIC_SCL_1;
 446  009d 72185005      	bset	20485,#4
 447                     ; 133 	p_dr11_nop();
 449  00a1 cd0000        	call	_p_dr11_nop
 451                     ; 134 	p_dr11_nop();		
 453  00a4 cd0000        	call	_p_dr11_nop
 455                     ; 135 	p_dr11_nop();	
 457  00a7 cd0000        	call	_p_dr11_nop
 459                     ; 136 	Write_P_IIC_SCL_0;
 461  00aa 72195005      	bres	20485,#4
 462                     ; 138 }
 465  00ae 81            	ret
 511                     ; 145 void p_dr11_SendB(u8 c)
 511                     ; 146 {
 512                     	switch	.text
 513  00af               _p_dr11_SendB:
 515  00af 88            	push	a
 516  00b0 88            	push	a
 517       00000001      OFST:	set	1
 520                     ; 148 	for(bitCnt=0; bitCnt<8; bitCnt++)
 522  00b1 0f01          	clr	(OFST+0,sp)
 523  00b3               L371:
 524                     ; 150 		if((c<<bitCnt)&0x80)		//check the bit to sned
 526  00b3 7b02          	ld	a,(OFST+1,sp)
 527  00b5 5f            	clrw	x
 528  00b6 97            	ld	xl,a
 529  00b7 7b01          	ld	a,(OFST+0,sp)
 530  00b9 4d            	tnz	a
 531  00ba 2704          	jreq	L22
 532  00bc               L42:
 533  00bc 58            	sllw	x
 534  00bd 4a            	dec	a
 535  00be 26fc          	jrne	L42
 536  00c0               L22:
 537  00c0 01            	rrwa	x,a
 538  00c1 a580          	bcp	a,#128
 539  00c3 2704          	jreq	L102
 540                     ; 151 			p_dr11_Send_1();	
 542  00c5 adcf          	call	_p_dr11_Send_1
 545  00c7 2002          	jra	L302
 546  00c9               L102:
 547                     ; 153 			p_dr11_Send_0();
 549  00c9 adb2          	call	_p_dr11_Send_0
 551  00cb               L302:
 552                     ; 148 	for(bitCnt=0; bitCnt<8; bitCnt++)
 554  00cb 0c01          	inc	(OFST+0,sp)
 557  00cd 7b01          	ld	a,(OFST+0,sp)
 558  00cf a108          	cp	a,#8
 559  00d1 25e0          	jrult	L371
 560                     ; 156 	PDDR_IIC_SDA = 0;
 562  00d3 721b5007      	bres	20487,#5
 563                     ; 157 	p_dr11_nop();	
 565  00d7 cd0000        	call	_p_dr11_nop
 567                     ; 158 }
 570  00da 85            	popw	x
 571  00db 81            	ret
 615                     ; 164 u8 p_dr11_RcvB(void)
 615                     ; 165 {
 616                     	switch	.text
 617  00dc               _p_dr11_RcvB:
 619  00dc 89            	pushw	x
 620       00000002      OFST:	set	2
 623                     ; 168 	retc = 0;
 625  00dd 0f02          	clr	(OFST+0,sp)
 626                     ; 169 	Write_P_IIC_SDA_1;;
 628  00df 721a5005      	bset	20485,#5
 629                     ; 170 	for(bitCnt=0; bitCnt<8; bitCnt++)
 632  00e3 0f01          	clr	(OFST-1,sp)
 633  00e5               L722:
 634                     ; 173 		Write_P_IIC_SCL_0;
 636  00e5 72195005      	bres	20485,#4
 637                     ; 174 		p_dr11_nop();
 639  00e9 cd0000        	call	_p_dr11_nop
 641                     ; 175 		p_dr11_nop();
 643  00ec cd0000        	call	_p_dr11_nop
 645                     ; 176 		Write_P_IIC_SCL_1;
 647  00ef 72185005      	bset	20485,#4
 648                     ; 177 		p_dr11_nop();
 650  00f3 cd0000        	call	_p_dr11_nop
 652                     ; 178 		p_dr11_nop();
 654  00f6 cd0000        	call	_p_dr11_nop
 656                     ; 179 		retc = retc<<1;
 658  00f9 0802          	sll	(OFST+0,sp)
 659                     ; 181 		PDDR_IIC_SDA = 0;
 661  00fb 721b5007      	bres	20487,#5
 662                     ; 183 		if(PIDR_SDA)
 664  00ff c65006        	ld	a,20486
 665  0102 a520          	bcp	a,#32
 666  0104 2702          	jreq	L532
 667                     ; 184 			retc += 1;	
 669  0106 0c02          	inc	(OFST+0,sp)
 670  0108               L532:
 671                     ; 186 		PDDR_IIC_SDA = 1;
 673  0108 721a5007      	bset	20487,#5
 674                     ; 170 	for(bitCnt=0; bitCnt<8; bitCnt++)
 676  010c 0c01          	inc	(OFST-1,sp)
 679  010e 7b01          	ld	a,(OFST-1,sp)
 680  0110 a108          	cp	a,#8
 681  0112 25d1          	jrult	L722
 682                     ; 188 	Write_P_IIC_SCL_0;
 684  0114 72195005      	bres	20485,#4
 685                     ; 189 	p_dr11_nop();
 687  0118 cd0000        	call	_p_dr11_nop
 689                     ; 190 	p_dr11_nop();
 691  011b cd0000        	call	_p_dr11_nop
 693                     ; 191 	return(retc);	
 695  011e 7b02          	ld	a,(OFST+0,sp)
 698  0120 85            	popw	x
 699  0121 81            	ret
 735                     ; 199 void p_dr11_I2cAck(u8 a)
 735                     ; 200 {
 736                     	switch	.text
 737  0122               _p_dr11_I2cAck:
 741                     ; 201 	if(a == 0)
 743  0122 4d            	tnz	a
 744  0123 2605          	jrne	L552
 745                     ; 202 		p_dr11_Send_0();
 747  0125 cd007d        	call	_p_dr11_Send_0
 750  0128 2003          	jra	L752
 751  012a               L552:
 752                     ; 204 		p_dr11_Send_1();
 754  012a cd0096        	call	_p_dr11_Send_1
 756  012d               L752:
 757                     ; 205 }
 760  012d 81            	ret
 808                     ; 212 u8 p_dr11_I2cSendB(u8 sla, u8 c)
 808                     ; 213 {
 809                     	switch	.text
 810  012e               _p_dr11_I2cSendB:
 812  012e 89            	pushw	x
 813       00000000      OFST:	set	0
 816                     ; 214 	G_dr01_I2cError = 0;	
 818  012f 3f00          	clr	_G_dr01_I2cError
 819  0131               L303:
 820                     ; 217 		p_dr11_I2cStart();
 822  0131 cd000d        	call	_p_dr11_I2cStart
 824                     ; 218 		p_dr11_SendB(DEVICE_ADDRESS);
 826  0134 a6d2          	ld	a,#210
 827  0136 cd00af        	call	_p_dr11_SendB
 829                     ; 219 		if(p_dr11_CheckAck() == 1)
 831  0139 cd004c        	call	_p_dr11_CheckAck
 833  013c a101          	cp	a,#1
 834  013e 2611          	jrne	L703
 835                     ; 221 			break;
 836                     ; 229 	p_dr11_SendB(sla);
 838  0140 7b01          	ld	a,(OFST+1,sp)
 839  0142 cd00af        	call	_p_dr11_SendB
 841                     ; 230 	if(p_dr11_CheckAck() == 0)
 843  0145 cd004c        	call	_p_dr11_CheckAck
 845  0148 4d            	tnz	a
 846  0149 2614          	jrne	L513
 847                     ; 232 		p_dr11_I2cStop();			
 849  014b cd0037        	call	_p_dr11_I2cStop
 851                     ; 233 		return(0);
 853  014e 4f            	clr	a
 855  014f 200c          	jra	L43
 856  0151               L703:
 857                     ; 223 		else if(G_dr01_I2cError++ > DR01_IIC_ERROR_CNT)
 859  0151 b600          	ld	a,_G_dr01_I2cError
 860  0153 3c00          	inc	_G_dr01_I2cError
 861  0155 a104          	cp	a,#4
 862  0157 25d8          	jrult	L303
 863                     ; 225 			p_dr11_I2cStop();			
 865  0159 cd0037        	call	_p_dr11_I2cStop
 867                     ; 226 			return(0);
 869  015c 4f            	clr	a
 871  015d               L43:
 873  015d 85            	popw	x
 874  015e 81            	ret
 875  015f               L513:
 876                     ; 235 	p_dr11_SendB(c);
 878  015f 7b02          	ld	a,(OFST+2,sp)
 879  0161 cd00af        	call	_p_dr11_SendB
 881                     ; 237 	if(p_dr11_CheckAck() == 0)
 883  0164 cd004c        	call	_p_dr11_CheckAck
 885  0167 4d            	tnz	a
 886  0168 2606          	jrne	L713
 887                     ; 239 		p_dr11_I2cStop();			
 889  016a cd0037        	call	_p_dr11_I2cStop
 891                     ; 240 		return(0);
 893  016d 4f            	clr	a
 895  016e 20ed          	jra	L43
 896  0170               L713:
 897                     ; 242 	p_dr11_I2cStop();
 899  0170 cd0037        	call	_p_dr11_I2cStop
 901                     ; 243 	return(1);
 903  0173 a601          	ld	a,#1
 905  0175 20e6          	jra	L43
 972                     ; 251 u8 p_dr11_I2cSendStr(u8 sla, u8 *s, u8 len)
 972                     ; 252 {
 973                     	switch	.text
 974  0177               _p_dr11_I2cSendStr:
 976  0177 88            	push	a
 977  0178 88            	push	a
 978       00000001      OFST:	set	1
 981                     ; 255 	G_dr01_I2cError = 0;	
 983  0179 3f00          	clr	_G_dr01_I2cError
 984  017b               L353:
 985                     ; 258 		p_dr11_I2cStart();
 987  017b cd000d        	call	_p_dr11_I2cStart
 989                     ; 259 		p_dr11_SendB(sla);
 991  017e 7b02          	ld	a,(OFST+1,sp)
 992  0180 cd00af        	call	_p_dr11_SendB
 994                     ; 260 		if(p_dr11_CheckAck() == 1)
 996  0183 cd004c        	call	_p_dr11_CheckAck
 998  0186 a101          	cp	a,#1
 999  0188 2604          	jrne	L753
1000                     ; 261 			break;
1001                     ; 269 	for(i=0; i<len; i++)
1003  018a 0f01          	clr	(OFST+0,sp)
1005  018c 2029          	jra	L173
1006  018e               L753:
1007                     ; 262 		else if(G_dr01_I2cError++ > DR01_IIC_ERROR_CNT)
1009  018e b600          	ld	a,_G_dr01_I2cError
1010  0190 3c00          	inc	_G_dr01_I2cError
1011  0192 a104          	cp	a,#4
1012  0194 25e5          	jrult	L353
1013                     ; 264 			p_dr11_I2cStop();			
1015  0196 cd0037        	call	_p_dr11_I2cStop
1017                     ; 265 			return(0);
1019  0199 4f            	clr	a
1021  019a 2010          	jra	L04
1022  019c               L563:
1023                     ; 271 		p_dr11_SendB(*s);
1025  019c 1e05          	ldw	x,(OFST+4,sp)
1026  019e f6            	ld	a,(x)
1027  019f cd00af        	call	_p_dr11_SendB
1029                     ; 273 		if(p_dr11_CheckAck() == 0)
1031  01a2 cd004c        	call	_p_dr11_CheckAck
1033  01a5 4d            	tnz	a
1034  01a6 2606          	jrne	L573
1035                     ; 275 			p_dr11_I2cStop();			
1037  01a8 cd0037        	call	_p_dr11_I2cStop
1039                     ; 276 			return(0);
1041  01ab 4f            	clr	a
1043  01ac               L04:
1045  01ac 85            	popw	x
1046  01ad 81            	ret
1047  01ae               L573:
1048                     ; 279 		s++;
1050  01ae 1e05          	ldw	x,(OFST+4,sp)
1051  01b0 1c0001        	addw	x,#1
1052  01b3 1f05          	ldw	(OFST+4,sp),x
1053                     ; 269 	for(i=0; i<len; i++)
1055  01b5 0c01          	inc	(OFST+0,sp)
1056  01b7               L173:
1059  01b7 7b01          	ld	a,(OFST+0,sp)
1060  01b9 1107          	cp	a,(OFST+6,sp)
1061  01bb 25df          	jrult	L563
1062                     ; 281 	p_dr11_I2cStop();
1064  01bd cd0037        	call	_p_dr11_I2cStop
1066                     ; 282 	return(1);
1068  01c0 a601          	ld	a,#1
1070  01c2 20e8          	jra	L04
1121                     ; 284 u8 p_dr11_I2cRecB(u8 sla,u8 *c)
1121                     ; 285 {
1122                     	switch	.text
1123  01c4               _p_dr11_I2cRecB:
1125  01c4 88            	push	a
1126       00000000      OFST:	set	0
1129                     ; 288 	G_dr01_I2cError = 0;	
1131  01c5 3f00          	clr	_G_dr01_I2cError
1132  01c7               L124:
1133                     ; 291 		p_dr11_I2cStart();
1135  01c7 cd000d        	call	_p_dr11_I2cStart
1137                     ; 292 		p_dr11_SendB(DEVICE_ADDRESS);
1139  01ca a6d2          	ld	a,#210
1140  01cc cd00af        	call	_p_dr11_SendB
1142                     ; 293 		if(p_dr11_CheckAck() == 1)
1144  01cf cd004c        	call	_p_dr11_CheckAck
1146  01d2 a101          	cp	a,#1
1147  01d4 2612          	jrne	L524
1148                     ; 294 			break;
1149                     ; 301 	p_dr11_SendB(sla);
1151  01d6 7b01          	ld	a,(OFST+1,sp)
1152  01d8 cd00af        	call	_p_dr11_SendB
1154                     ; 302 	if(p_dr11_CheckAck() == 0)
1156  01db cd004c        	call	_p_dr11_CheckAck
1158  01de 4d            	tnz	a
1159  01df 2616          	jrne	L534
1160                     ; 304 			p_dr11_I2cStop();			
1162  01e1 cd0037        	call	_p_dr11_I2cStop
1164                     ; 305 			return(0);
1166  01e4 4f            	clr	a
1169  01e5 5b01          	addw	sp,#1
1170  01e7 81            	ret
1171  01e8               L524:
1172                     ; 295 		else if(G_dr01_I2cError++ > DR01_IIC_ERROR_CNT)
1174  01e8 b600          	ld	a,_G_dr01_I2cError
1175  01ea 3c00          	inc	_G_dr01_I2cError
1176  01ec a104          	cp	a,#4
1177  01ee 25d7          	jrult	L124
1178                     ; 297 			p_dr11_I2cStop();			
1180  01f0 cd0037        	call	_p_dr11_I2cStop
1182                     ; 298 			return(0);
1184  01f3 4f            	clr	a
1187  01f4 5b01          	addw	sp,#1
1188  01f6 81            	ret
1189  01f7               L534:
1190                     ; 309 		p_dr11_I2cStart();
1192  01f7 cd000d        	call	_p_dr11_I2cStart
1194                     ; 310 		p_dr11_SendB(DEVICE_ADDRESS+1);
1196  01fa a6d3          	ld	a,#211
1197  01fc cd00af        	call	_p_dr11_SendB
1199                     ; 311 		if(p_dr11_CheckAck() == 1)
1201  01ff cd004c        	call	_p_dr11_CheckAck
1203  0202 a101          	cp	a,#1
1204  0204 2613          	jrne	L144
1205                     ; 312 			break;
1206                     ; 319 	*c = p_dr11_RcvB();
1208  0206 cd00dc        	call	_p_dr11_RcvB
1210  0209 1e04          	ldw	x,(OFST+4,sp)
1211  020b f7            	ld	(x),a
1212                     ; 320 	p_dr11_I2cAck(1);				//no ack
1214  020c a601          	ld	a,#1
1215  020e cd0122        	call	_p_dr11_I2cAck
1217                     ; 321 	p_dr11_I2cStop();
1219  0211 cd0037        	call	_p_dr11_I2cStop
1221                     ; 322 	return(1);
1223  0214 a601          	ld	a,#1
1226  0216 5b01          	addw	sp,#1
1227  0218 81            	ret
1228  0219               L144:
1229                     ; 313 		else if(G_dr01_I2cError++ > DR01_IIC_ERROR_CNT)
1231  0219 b600          	ld	a,_G_dr01_I2cError
1232  021b 3c00          	inc	_G_dr01_I2cError
1233  021d a104          	cp	a,#4
1234  021f 25d6          	jrult	L534
1235                     ; 315 			p_dr11_I2cStop();			
1237  0221 cd0037        	call	_p_dr11_I2cStop
1239                     ; 316 			return(0);
1241  0224 4f            	clr	a
1244  0225 5b01          	addw	sp,#1
1245  0227 81            	ret
1314                     ; 329 u8 p_dr11_I2cRecStr(u8 sla, u8 *c, u8 len)
1314                     ; 330 {
1315                     	switch	.text
1316  0228               _p_dr11_I2cRecStr:
1318  0228 88            	push	a
1319  0229 88            	push	a
1320       00000001      OFST:	set	1
1323                     ; 333 	G_dr01_I2cError = 0;	
1325  022a 3f00          	clr	_G_dr01_I2cError
1326  022c               L105:
1327                     ; 336 		p_dr11_I2cStart();
1329  022c cd000d        	call	_p_dr11_I2cStart
1331                     ; 337 		p_dr11_SendB(sla+1);
1333  022f 7b02          	ld	a,(OFST+1,sp)
1334  0231 4c            	inc	a
1335  0232 cd00af        	call	_p_dr11_SendB
1337                     ; 339 		if(p_dr11_CheckAck() == 1)
1339  0235 cd004c        	call	_p_dr11_CheckAck
1341  0238 a101          	cp	a,#1
1342  023a 2604          	jrne	L505
1343                     ; 340 			break;
1344                     ; 348 	for(i=0; i<(len-1); i++)
1346  023c 0f01          	clr	(OFST+0,sp)
1348  023e 2021          	jra	L715
1349  0240               L505:
1350                     ; 341 		else if(G_dr01_I2cError++ > DR01_IIC_ERROR_CNT)
1352  0240 b600          	ld	a,_G_dr01_I2cError
1353  0242 3c00          	inc	_G_dr01_I2cError
1354  0244 a104          	cp	a,#4
1355  0246 25e4          	jrult	L105
1356                     ; 343 			p_dr11_I2cStop();			
1358  0248 cd0037        	call	_p_dr11_I2cStop
1360                     ; 344 			return(0);
1362  024b 4f            	clr	a
1364  024c 2036          	jra	L64
1365  024e               L315:
1366                     ; 350 		*c = p_dr11_RcvB();
1368  024e cd00dc        	call	_p_dr11_RcvB
1370  0251 1e05          	ldw	x,(OFST+4,sp)
1371  0253 f7            	ld	(x),a
1372                     ; 351 		c++;
1374  0254 1e05          	ldw	x,(OFST+4,sp)
1375  0256 1c0001        	addw	x,#1
1376  0259 1f05          	ldw	(OFST+4,sp),x
1377                     ; 352 		p_dr11_I2cAck(0);				//ack
1379  025b 4f            	clr	a
1380  025c cd0122        	call	_p_dr11_I2cAck
1382                     ; 348 	for(i=0; i<(len-1); i++)
1384  025f 0c01          	inc	(OFST+0,sp)
1385  0261               L715:
1388  0261 9c            	rvf
1389  0262 7b07          	ld	a,(OFST+6,sp)
1390  0264 5f            	clrw	x
1391  0265 97            	ld	xl,a
1392  0266 5a            	decw	x
1393  0267 7b01          	ld	a,(OFST+0,sp)
1394  0269 905f          	clrw	y
1395  026b 9097          	ld	yl,a
1396  026d 90bf01        	ldw	c_y+1,y
1397  0270 b301          	cpw	x,c_y+1
1398  0272 2cda          	jrsgt	L315
1399                     ; 355 	*c = p_dr11_RcvB();
1401  0274 cd00dc        	call	_p_dr11_RcvB
1403  0277 1e05          	ldw	x,(OFST+4,sp)
1404  0279 f7            	ld	(x),a
1405                     ; 356 	p_dr11_I2cAck(1);				//no ack
1407  027a a601          	ld	a,#1
1408  027c cd0122        	call	_p_dr11_I2cAck
1410                     ; 357 	p_dr11_I2cStop();
1412  027f cd0037        	call	_p_dr11_I2cStop
1414                     ; 358 	return(1);
1416  0282 a601          	ld	a,#1
1418  0284               L64:
1420  0284 85            	popw	x
1421  0285 81            	ret
1491                     ; 361 u8 p_dr11_I2cRecStrWithoutStop(u8 sla, u8 *c, u8 len)
1491                     ; 362 {
1492                     	switch	.text
1493  0286               _p_dr11_I2cRecStrWithoutStop:
1495  0286 88            	push	a
1496  0287 88            	push	a
1497       00000001      OFST:	set	1
1500                     ; 365 	G_dr01_I2cError = 0;	
1502  0288 3f00          	clr	_G_dr01_I2cError
1503  028a               L555:
1504                     ; 368 		p_dr11_I2cStart();
1506  028a cd000d        	call	_p_dr11_I2cStart
1508                     ; 369 		p_dr11_SendB(sla+1);
1510  028d 7b02          	ld	a,(OFST+1,sp)
1511  028f 4c            	inc	a
1512  0290 cd00af        	call	_p_dr11_SendB
1514                     ; 371 		if(p_dr11_CheckAck() == 1)
1516  0293 cd004c        	call	_p_dr11_CheckAck
1518  0296 a101          	cp	a,#1
1519  0298 2604          	jrne	L165
1520                     ; 372 			break;
1521                     ; 380 	for(i=0; i<len; i++)
1523  029a 0f01          	clr	(OFST+0,sp)
1525  029c 2021          	jra	L375
1526  029e               L165:
1527                     ; 373 		else if(G_dr01_I2cError++ > DR01_IIC_ERROR_CNT)
1529  029e b600          	ld	a,_G_dr01_I2cError
1530  02a0 3c00          	inc	_G_dr01_I2cError
1531  02a2 a104          	cp	a,#4
1532  02a4 25e4          	jrult	L555
1533                     ; 375 			p_dr11_I2cStop();			
1535  02a6 cd0037        	call	_p_dr11_I2cStop
1537                     ; 376 			return(0);
1539  02a9 4f            	clr	a
1541  02aa 201b          	jra	L25
1542  02ac               L765:
1543                     ; 382 		*c = p_dr11_RcvB();
1545  02ac cd00dc        	call	_p_dr11_RcvB
1547  02af 1e05          	ldw	x,(OFST+4,sp)
1548  02b1 f7            	ld	(x),a
1549                     ; 383 		c++;
1551  02b2 1e05          	ldw	x,(OFST+4,sp)
1552  02b4 1c0001        	addw	x,#1
1553  02b7 1f05          	ldw	(OFST+4,sp),x
1554                     ; 384 		p_dr11_I2cAck(0);				//ack
1556  02b9 4f            	clr	a
1557  02ba cd0122        	call	_p_dr11_I2cAck
1559                     ; 380 	for(i=0; i<len; i++)
1561  02bd 0c01          	inc	(OFST+0,sp)
1562  02bf               L375:
1565  02bf 7b01          	ld	a,(OFST+0,sp)
1566  02c1 1107          	cp	a,(OFST+6,sp)
1567  02c3 25e7          	jrult	L765
1568                     ; 387 	return(1);
1570  02c5 a601          	ld	a,#1
1572  02c7               L25:
1574  02c7 85            	popw	x
1575  02c8 81            	ret
1599                     ; 390 void p_dr11_InitI2cBus()
1599                     ; 391 {
1600                     	switch	.text
1601  02c9               _p_dr11_InitI2cBus:
1605                     ; 393 	G_dr01_I2cError = 0;
1607  02c9 3f00          	clr	_G_dr01_I2cError
1608                     ; 394 }
1611  02cb 81            	ret
1624                     	xdef	_p_dr11_Send_1
1625                     	xdef	_p_dr11_Send_0
1626                     	xdef	_p_dr11_CheckAck
1627                     	xdef	_p_dr11_nop
1628                     	xdef	_p_dr11_InitI2cBus
1629                     	xdef	_p_dr11_I2cRecStrWithoutStop
1630                     	xdef	_p_dr11_I2cRecStr
1631                     	xdef	_p_dr11_I2cRecB
1632                     	xdef	_p_dr11_I2cSendStr
1633                     	xdef	_p_dr11_I2cSendB
1634                     	xdef	_p_dr11_I2cAck
1635                     	xdef	_p_dr11_RcvB
1636                     	xdef	_p_dr11_SendB
1637                     	xdef	_p_dr11_I2cStop
1638                     	xdef	_p_dr11_I2cStart
1639                     	switch	.ubsct
1640  0000               _G_dr01_I2cError:
1641  0000 00            	ds.b	1
1642                     	xdef	_G_dr01_I2cError
1643                     	xref.b	c_y
1663                     	end
