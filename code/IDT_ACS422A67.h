#ifndef _IDT_ACS422A67_H
	#define _IDT_ACS422A67_H


#define DEVICE_ADDRESS			0xd2

#define INPUT1_CHANNEL		0
#define INPUT2_CHANNEL		1


void IDT_InputChannel(u8 ChannelNum);
void IDT_SetVolume(u8 Volume);
void IDT_Init(void);
void IDT_MuteOn(void);

#endif

