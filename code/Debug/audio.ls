   1                     ; C Compiler for STM8 (COSMIC Software)
   2                     ; Generator V4.2.8 - 03 Dec 2008
 122                     ; 20 void Audio_SetAudioSource(u8 source)
 122                     ; 21 {
 124                     	switch	.text
 125  0000               _Audio_SetAudioSource:
 129                     ; 23 	if(source == LINE_IN_AUDIO)
 131  0000 a101          	cp	a,#1
 132  0002 2607          	jrne	L76
 133                     ; 25 		IDT_InputChannel(INPUT2_CHANNEL);	// Source Line-in Audio
 135  0004 a601          	ld	a,#1
 136  0006 cd0000        	call	_IDT_InputChannel
 139  0009 2004          	jra	L17
 140  000b               L76:
 141                     ; 29 		IDT_InputChannel(INPUT1_CHANNEL);		// Source Bluetooth Audio
 143  000b 4f            	clr	a
 144  000c cd0000        	call	_IDT_InputChannel
 146  000f               L17:
 147                     ; 31 }
 150  000f 81            	ret
 177                     ; 46 void Audio_AudioSleepCheck(void)
 177                     ; 47 {
 178                     	switch	.text
 179  0010               _Audio_AudioSleepCheck:
 183                     ; 48 	if (!IsPowerOn)
 185  0010 3d00          	tnz	_IsPowerOn
 186  0012 2601          	jrne	L301
 187                     ; 49 		return;
 190  0014 81            	ret
 191  0015               L301:
 192                     ; 50 	if(SLEEP_PIDR_READ == 0)
 194  0015 c6501a        	ld	a,20506
 195  0018 a510          	bcp	a,#16
 196  001a 2603          	jrne	L501
 197                     ; 52 		Clock[SYSTEM_SLEEP_CLK] = 0;
 199  001c 5f            	clrw	x
 200  001d bf04          	ldw	_Clock+4,x
 201  001f               L501:
 202                     ; 54 	if (Clock[SYSTEM_SLEEP_CLK] == 20)
 204  001f be04          	ldw	x,_Clock+4
 205  0021 a30014        	cpw	x,#20
 206  0024 260f          	jrne	L701
 207                     ; 56 		Clock[SYSTEM_SLEEP_CLK] = 0;
 209  0026 5f            	clrw	x
 210  0027 bf04          	ldw	_Clock+4,x
 211                     ; 57 		MainTask |= SYSTEM_POWEROFF_TASK;
 213  0029 b600          	ld	a,_MainTask
 214  002b 97            	ld	xl,a
 215  002c b601          	ld	a,_MainTask+1
 216  002e aa02          	or	a,#2
 217  0030 b701          	ld	_MainTask+1,a
 218  0032 9f            	ld	a,xl
 219  0033 b700          	ld	_MainTask,a
 220  0035               L701:
 221                     ; 59 }
 224  0035 81            	ret
 268                     ; 71 void AudioHandler(void)
 268                     ; 72 {
 269                     	switch	.text
 270  0036               _AudioHandler:
 272  0036 88            	push	a
 273       00000001      OFST:	set	1
 276                     ; 75 	if (!IsPowerOn)
 278  0037 3d00          	tnz	_IsPowerOn
 279  0039 2602          	jrne	L331
 280                     ; 76 		return;
 283  003b 84            	pop	a
 284  003c 81            	ret
 285  003d               L331:
 286                     ; 77 	u8keycode = Key_GetCode();
 288  003d cd0000        	call	_Key_GetCode
 290  0040 6b01          	ld	(OFST+0,sp),a
 291                     ; 78 	u8PerVolume = u8Volume;
 293  0042 450100        	mov	_u8PerVolume,_u8Volume
 294                     ; 79 	switch(u8keycode)
 296  0045 7b01          	ld	a,(OFST+0,sp)
 298                     ; 124 			break;
 299  0047 a005          	sub	a,#5
 300  0049 270c          	jreq	L111
 301  004b 4a            	dec	a
 302  004c 2758          	jreq	L311
 303  004e a03f          	sub	a,#63
 304  0050 2705          	jreq	L111
 305  0052 4a            	dec	a
 306  0053 2751          	jreq	L311
 307  0055 2076          	jra	L731
 308  0057               L111:
 309                     ; 81 		case KEY_VOL_UP:
 309                     ; 82 		case KEY_VOL_UP_LONG:
 309                     ; 83 			if (((u8keycode == KEY_VOL_UP_LONG)&&Key_IsRepeatStep())
 309                     ; 84 				||(u8keycode == KEY_VOL_UP))
 311  0057 7b01          	ld	a,(OFST+0,sp)
 312  0059 a145          	cp	a,#69
 313  005b 2606          	jrne	L541
 315  005d cd0000        	call	_Key_IsRepeatStep
 317  0060 4d            	tnz	a
 318  0061 2606          	jrne	L341
 319  0063               L541:
 321  0063 7b01          	ld	a,(OFST+0,sp)
 322  0065 a105          	cp	a,#5
 323  0067 260e          	jrne	L141
 324  0069               L341:
 325                     ; 86 				if (u8Volume < MAX_VOLUME)
 327  0069 b601          	ld	a,_u8Volume
 328  006b a120          	cp	a,#32
 329  006d 2404          	jruge	L741
 330                     ; 87 					u8Volume++;
 332  006f 3c01          	inc	_u8Volume
 334  0071 2004          	jra	L141
 335  0073               L741:
 336                     ; 89 					u8Volume = MAX_VOLUME;
 338  0073 35200001      	mov	_u8Volume,#32
 339  0077               L141:
 340                     ; 92 			if ((u8Volume == MAX_VOLUME)&&(bMaxVolume))
 342  0077 b601          	ld	a,_u8Volume
 343  0079 a120          	cp	a,#32
 344  007b 261c          	jrne	L351
 346  007d 3d04          	tnz	_bMaxVolume
 347  007f 2718          	jreq	L351
 348                     ; 95 				bMaxVolume = FALSE;
 350  0081 3f04          	clr	_bMaxVolume
 351                     ; 98 				if (SourceSelect == LINE_IN_AUDIO)
 353  0083 b603          	ld	a,_SourceSelect
 354  0085 a101          	cp	a,#1
 355  0087 2607          	jrne	L551
 356                     ; 100 					Beeper_SetPlay(TRUE);
 358  0089 a601          	ld	a,#1
 359  008b cd0000        	call	_Beeper_SetPlay
 362  008e 2009          	jra	L351
 363  0090               L551:
 364                     ; 104 					BT_VOLMAX_H;
 366  0090 721e500f      	bset	20495,#7
 367                     ; 105 					Clock[BT_VOLMAX_CLK] = 1100;
 369  0094 ae044c        	ldw	x,#1100
 370  0097 bf18          	ldw	_Clock+24,x
 371  0099               L351:
 372                     ; 109 			if (u8PerVolume != u8Volume)
 374  0099 b600          	ld	a,_u8PerVolume
 375  009b b101          	cp	a,_u8Volume
 376  009d 272e          	jreq	L731
 377                     ; 110 				IDT_SetVolume(u8Volume);
 379  009f b601          	ld	a,_u8Volume
 380  00a1 cd0000        	call	_IDT_SetVolume
 382  00a4 2027          	jra	L731
 383  00a6               L311:
 384                     ; 112 		case KEY_VOL_DOWN:
 384                     ; 113 		case KEY_VOL_DOWN_LOGN:
 384                     ; 114 			if (((u8keycode == KEY_VOL_DOWN_LOGN)&&Key_IsRepeatStep())
 384                     ; 115 				||(u8keycode == KEY_VOL_DOWN))
 386  00a6 7b01          	ld	a,(OFST+0,sp)
 387  00a8 a146          	cp	a,#70
 388  00aa 2606          	jrne	L761
 390  00ac cd0000        	call	_Key_IsRepeatStep
 392  00af 4d            	tnz	a
 393  00b0 2606          	jrne	L561
 394  00b2               L761:
 396  00b2 7b01          	ld	a,(OFST+0,sp)
 397  00b4 a106          	cp	a,#6
 398  00b6 260a          	jrne	L361
 399  00b8               L561:
 400                     ; 117 				if (u8Volume > MIN_VOLUME)
 402  00b8 3d01          	tnz	_u8Volume
 403  00ba 2704          	jreq	L171
 404                     ; 118 					u8Volume--;
 406  00bc 3a01          	dec	_u8Volume
 408  00be 2002          	jra	L361
 409  00c0               L171:
 410                     ; 120 					u8Volume = MIN_VOLUME;
 412  00c0 3f01          	clr	_u8Volume
 413  00c2               L361:
 414                     ; 122 			if (u8PerVolume != u8Volume)
 416  00c2 b600          	ld	a,_u8PerVolume
 417  00c4 b101          	cp	a,_u8Volume
 418  00c6 2705          	jreq	L731
 419                     ; 123 				IDT_SetVolume(u8Volume);
 421  00c8 b601          	ld	a,_u8Volume
 422  00ca cd0000        	call	_IDT_SetVolume
 424  00cd               L731:
 425                     ; 127 }
 428  00cd 84            	pop	a
 429  00ce 81            	ret
 483                     	switch	.ubsct
 484  0000               _u8PerVolume:
 485  0000 00            	ds.b	1
 486                     	xdef	_u8PerVolume
 487                     	xref	_IDT_SetVolume
 488                     	xref	_IDT_InputChannel
 489                     	xdef	_AudioHandler
 490                     	xdef	_Audio_AudioSleepCheck
 491                     	xdef	_Audio_SetAudioSource
 492  0001               _u8Volume:
 493  0001 00            	ds.b	1
 494                     	xdef	_u8Volume
 495  0002               _bSleepStau:
 496  0002 00            	ds.b	1
 497                     	xdef	_bSleepStau
 498  0003               _SourceSelect:
 499  0003 00            	ds.b	1
 500                     	xdef	_SourceSelect
 501  0004               _bMaxVolume:
 502  0004 00            	ds.b	1
 503                     	xdef	_bMaxVolume
 504                     	xref	_Key_GetCode
 505                     	xref	_Key_IsRepeatStep
 506                     	xref.b	_IsPowerOn
 507                     	xref.b	_MainTask
 508                     	xref	_Beeper_SetPlay
 509                     	xref.b	_Clock
 529                     	end
