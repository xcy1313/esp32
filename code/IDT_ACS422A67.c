#include "include.h"
#include "cdr01iic.h"


void IDT_EQ(void);
void IDT_CompressorSet(void);


 uc32 Coeff[176] ={
0x003f787c, 0xff810f07, 0x003f787c, 0x007ef015, 0xffc10e24, 0x003f787c, 0xff810f07, 0x003f787c, 
0x007ef015, 0xffc10e24, 0x00408528, 0xff807e8c, 0x003effae, 0x007f8174, 0xffc07b2a, 0x0040e4e0, 
0xff853109, 0x003ac587, 0x007acef7, 0xffc45599, 0x003e6996, 0xffba22a8, 0x0024641a, 0x0045dd58, 
0xffdd324f, 0x004e54ff, 0xfff30d47, 0x0014de14, 0x000cf2b9, 0xffdccced, 0x00000000, 0x00159ae8, 
0x003f787c, 0xff810f07, 0x003f787c, 0x007ef015, 0xffc10e24, 0x003f787c, 0xff810f07, 0x003f787c, 
0x007ef015, 0xffc10e24, 0x00408528, 0xff807e8c, 0x003effae, 0x007f8174, 0xffc07b2a, 0x0040e4e0, 
0xff853109, 0x003ac587, 0x007acef7, 0xffc45599, 0x003e6996, 0xffba22a8, 0x0024641a, 0x0045dd58, 
0xffdd324f, 0x004e54ff, 0xfff30d47, 0x0014de14, 0x000cf2b9, 0xffdccced, 0x00000000, 0x00159ae8, 
0x0042c3a9, 0xff961fee, 0x002fd592, 0x0069e012, 0xffcd66c5, 0x003fc65c, 0xff81c2db, 0x003eaeed, 
0x007e3d25, 0xffc18ab6, 0x0040e379, 0xff8a614f, 0x0039c86f, 0x00759eb1, 0xffc55417, 0x0040197c, 
0xff820577, 0x003e44c5, 0x007dfa89, 0xffc1a1bf, 0x00401f79, 0xff816ff6, 0x003e9298, 0x007e900a, 
0xffc14def, 0x00400000, 0xffb93b7f, 0x00241499, 0x0046c481, 0xffdbeb67, 0x00000000, 0x00400000, 
0x0042c3a9, 0xff961fee, 0x002fd592, 0x0069e012, 0xffcd66c5, 0x003fc65c, 0xff81c2db, 0x003eaeed, 
0x007e3d25, 0xffc18ab6, 0x0040e379, 0xff8a614f, 0x0039c86f, 0x00759eb1, 0xffc55417, 0x0040197c, 
0xff820577, 0x003e44c5, 0x007dfa89, 0xffc1a1bf, 0x00401f79, 0xff816ff6, 0x003e9298, 0x007e900a, 
0xffc14def, 0x00400000, 0xffb93b7f, 0x00241499, 0x0046c481, 0xffdbeb67, 0x00000000, 0x00400000, 
0x0000b2b6, 0x00000000, 0xffff4d49, 0x007e8197, 0xffc1656d, 0x0000b2b6, 0x00000000, 0xffff4d49, 
0x007e8197, 0xffc1656d, 0x00000000, 0x00000000, 0x000010ce, 0x0000219d, 0x000010ce, 0x007a65c3, 
0xffc55703, 0x003dcdc3, 0xff846478, 0x003dcdc3, 0x007b85d5, 0xffc44ec7, 0x00000000, 0x0027dcfd, 
0xffb04605, 0x0027dcfd, 0x00468a27, 0xffe71633, 0x0027dcfd, 0xffb04605, 0x0027dcfd, 0x00468a27, 
0xffe71633, 0x00000000, 0x00000000, 0x000e718b, 0x001ce317, 0x000e718b, 0xffe811b5, 0x001e281d, 
0x00400000, 0xff847a2b, 0x003bb139, 0x007b85d5, 0xffc44ec7, 0x00000000, 0x00000000, 0x00400000
 };

const u8 VolTable[] = {
130,
134,
138,
142,
146,
150,
154,
158,
162,
166,
170,
174,
178,
182,
186,
190,
194,
198,
202,
206,
210,
214,
218,
222,
226,
230,
234,
238,
242,
246,
250,
255
};



/********************************************************************
Function Name:	
Description:	
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
void IDT_Init(void)
{

	p_dr11_I2cSendB(0x04, 0x00);
	p_dr11_I2cSendB(0x05, 0x00);
	p_dr11_I2cSendB(0x13, 0x2a);
	p_dr11_I2cSendB(0x14, 0x03);
	p_dr11_I2cSendB(0x00, 0x00);
	p_dr11_I2cSendB(0x01, 0x00);
	p_dr11_I2cSendB(0x02, 0x7f);
	p_dr11_I2cSendB(0x03, 0x7f);
	p_dr11_I2cSendB(0x1a, 0xfc);
	p_dr11_I2cSendB(0x1b, 0x7f);
	p_dr11_I2cSendB(0x1c, 0xc2);
	p_dr11_I2cSendB(0x1d, 0x00);
	p_dr11_I2cSendB(0x1e, 0x83);
	p_dr11_I2cSendB(0x16, 0x00);
	p_dr11_I2cSendB(0x18, 0x00);
	p_dr11_I2cSendB(0x69, 0x20);
	p_dr11_I2cSendB(0x20, 0x0e);

	p_dr11_I2cSendB(0x25, 0x00);
	p_dr11_I2cSendB(0x26, 0x12);
	p_dr11_I2cSendB(0x27, 0xaf);
	p_dr11_I2cSendB(0x28, 0x0a);
	p_dr11_I2cSendB(0x29, 0x67);
	p_dr11_I2cSendB(0x2a, 0xb1);
	p_dr11_I2cSendB(0x2b, 0x36);
	p_dr11_I2cSendB(0x2c, 0xff);
	p_dr11_I2cSendB(0x2d, 0xf1);
	p_dr11_I2cSendB(0x2e, 0xf1);
	p_dr11_I2cSendB(0x2f, 0xb3);
	p_dr11_I2cSendB(0x30, 0xd8);
	p_dr11_I2cSendB(0x31, 0x36);
	p_dr11_I2cSendB(0x32, 0xff);
	p_dr11_I2cSendB(0x33, 0x94);
	p_dr11_I2cSendB(0x34, 0x01);
	p_dr11_I2cSendB(0x35, 0xb3);
	p_dr11_I2cSendB(0x36, 0xd8);
	p_dr11_I2cSendB(0x37, 0x9b);
	p_dr11_I2cSendB(0x38, 0xff);
	p_dr11_I2cSendB(0x39, 0x00);

	IDT_EQ();
	p_dr11_I2cSendB(0x04,VolTable[u8Volume-1]);
	p_dr11_I2cSendB(0x05, VolTable[u8Volume-1]);
	Audio_SetAudioSource(SourceSelect);

}



/********************************************************************
Function Name:	
Description:	
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
void IDT_SetVolume(u8 Volume)
{
	
	if (Volume == MIN_VOLUME)
	{
		IDT_MuteOn();
	}
	else
	{
		p_dr11_I2cSendB(0x04, VolTable[u8Volume-1]);	
		p_dr11_I2cSendB(0x05, VolTable[u8Volume-1]);
	}

}

/********************************************************************
Function Name:	
Description:	
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
void IDT_MuteOn(void)
{
	p_dr11_I2cSendB(0x04, 0);
//	DelayNms(100);
	p_dr11_I2cSendB(0x05, 0);
//	DelayNms(100);
}

/********************************************************************
Function Name:	
Description:	
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
void IDT_InputChannel(u8 ChannelNum)
{
//0x0c:ADC signal path control left
//0x0d:ADC signal path control right

	switch(ChannelNum)
	{
		case INPUT1_CHANNEL:
			p_dr11_I2cSendB(0x0C, 0x00);
	//		DelayNms(100);
			p_dr11_I2cSendB(0x0D, 0x00);
	//		DelayNms(100);
			break;
		case INPUT2_CHANNEL:
			p_dr11_I2cSendB(0x0C, 0x40);
	//		DelayNms(100);
			p_dr11_I2cSendB(0x0D, 0x40);
	//		DelayNms(100);
			break;
	}

}	

/********************************************************************
Function Name:	
Description:	
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
void IDT_EQ(void)
{
	u8 i,DACCRStat;
	u8 LowByte,MidByte,HighByte;
	p_dr11_I2cSendB(0x04, 0x00);
	p_dr11_I2cSendB(0x05,0x00);
	for( i=0; i < 176; i++)
	{
		do
              {
			 p_dr11_I2cRecB(0x8a,&DACCRStat);
		}
		while(DACCRStat != 0);

		LowByte = (Coeff[i] & 0xff);
		MidByte = ((Coeff[i] >> 8) & 0xff);
		HighByte =  ((Coeff[i] >> 16) & 0xff);

		p_dr11_I2cSendB(0x40, i);
	//	DelayNms(100);
		p_dr11_I2cSendB(0x3a, LowByte);
	//	DelayNms(100);
		p_dr11_I2cSendB(0x3b, MidByte);
	//	DelayNms(100);
		p_dr11_I2cSendB(0x3c, HighByte);
	//	DelayNms(100);

	}


}


