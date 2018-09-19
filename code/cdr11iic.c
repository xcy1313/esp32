/* == IDENTIFICATION ==================================================
 *
 * Copyright (C) 2008, Plastoform Industries Ltd.
 *
 * System           : STM105K series
 * Module           : driver -- I2c
 * File             : cdr11iic.c
 *
 *
 * */

/* == HISTORY =========================================================
 *
 * Name      Date           V.  Action
 * --------------------------------------------------------------------
 * WSong	 Jan-2010			change to STM105K plastoform based on former Winbond 79E80x version
 * WSong	 Mar-2008
 * WSong     Jan-2008     		Created
 *                                             
 */
 

/*MPM==========================================================================
 * 
 * ABSTRACT:
 *
 * This module provide iic interface, with about 26kHz                 
 *                                                                      
 * Global functions to be called by other layers:
 * ----------------------------------------------
 *
 * Layer-local functions to be called from inside the DRIVER layer:
 * ---------------------------------------------------------------
 * 
 */



#define _CDR11IIC_C_


#include "include.h"
#include "cdr01iic.h"


void p_dr11_nop(void)
{
	u8 i;
	for(i=0;i<2;i++)
	{
		;//  Nop_Sub();
	}

}


/*============================================================================*
*Function	: void I2cStart()
*Description: send iic start status
*============================================================================*/
void p_dr11_I2cStart(void)
{
	u8 i=2;
	Write_P_IIC_SDA_1;
	p_dr11_nop();
	Write_P_IIC_SCL_1;

	while(i--)			//wait i2c stable time
	{
		p_dr11_nop();
	}
	
	Write_P_IIC_SDA_0;
	p_dr11_nop();
	p_dr11_nop();
	Write_P_IIC_SCL_0;
	p_dr11_nop();
}

/*============================================================================*
*Function	: void I2cStop()
*Description: send iic stop status
*============================================================================*/
void p_dr11_I2cStop(void)
{
	Write_P_IIC_SDA_0;
	p_dr11_nop();
	Write_P_IIC_SCL_1;
	p_dr11_nop();
	Write_P_IIC_SDA_1;;
	p_dr11_nop();
	p_dr11_nop();
}

u8 p_dr11_CheckAck(void)
{
	u8 Flag=0;
	///Write_P_IIC_SDA_1;
	p_dr11_nop();	
	Write_P_IIC_SCL_1;
	p_dr11_nop();
//	Flag = (u8)(PB_IDR & 0x20);
	Flag = (u8)(PIDR_SDA);
	p_dr11_nop();
	p_dr11_nop();	
	Write_P_IIC_SCL_0;
	p_dr11_nop();
//	PB_DDR |= 0x20;//make SDA Output
	PDDR_IIC_SDA = 1;
	if(Flag == 1)
		return 0;
	else
		return 1;	
}


void p_dr11_Send_0(void)
{
	Write_P_IIC_SDA_0;
	p_dr11_nop();
	Write_P_IIC_SCL_1;
	p_dr11_nop();
	p_dr11_nop();		
	p_dr11_nop();	
	Write_P_IIC_SCL_0;
//	p_dr11_nop();
}
void p_dr11_Send_1(void)
{
	Write_P_IIC_SDA_1;
	p_dr11_nop();
	Write_P_IIC_SCL_1;
	p_dr11_nop();
	p_dr11_nop();		
	p_dr11_nop();	
	Write_P_IIC_SCL_0;
//	p_dr11_nop();
}


/*============================================================================*
*Function	: void SendB(u8 c)
*Description: send 8 bit data through iic
*============================================================================*/
void p_dr11_SendB(u8 c)
{
	u8 bitCnt;
	for(bitCnt=0; bitCnt<8; bitCnt++)
	{
		if((c<<bitCnt)&0x80)		//check the bit to sned
			p_dr11_Send_1();	
		else
			p_dr11_Send_0();
	}
//	PB_DDR &= ~0x20;				//make SDA input, to receive ACK	
	PDDR_IIC_SDA = 0;
	p_dr11_nop();	
}

/*============================================================================*
*Function	: u8 RcvB()
*Description: receive 8 bit data through iic
*============================================================================*/
u8 p_dr11_RcvB(void)
{
	u8 retc;
	u8 bitCnt;
	retc = 0;
	Write_P_IIC_SDA_1;;
	for(bitCnt=0; bitCnt<8; bitCnt++)
	{
	//	_Nop();
		Write_P_IIC_SCL_0;
		p_dr11_nop();
		p_dr11_nop();
		Write_P_IIC_SCL_1;
		p_dr11_nop();
		p_dr11_nop();
		retc = retc<<1;
//		PB_DDR &= ~0x20;//make SDA input
		PDDR_IIC_SDA = 0;
//		if(PB_IDR & 0x20)
		if(PIDR_SDA)
			retc += 1;	
//		PB_DDR |= 0x20;//make SDA Output
		PDDR_IIC_SDA = 1;
	}
	Write_P_IIC_SCL_0;
	p_dr11_nop();
	p_dr11_nop();
	return(retc);	
}


/*============================================================================*
*Function	: void I2cAck(bit a)
*Description: send iic ACK bit
*============================================================================*/
void p_dr11_I2cAck(u8 a)
{
	if(a == 0)
		p_dr11_Send_0();
	else
		p_dr11_Send_1();
}

 
/*============================================================================*
*Function	: bit I2cSendB(u8 sla, u8 c)
*Description: send iic data including start bit-8bit adress-8bit data-stop bit
*============================================================================*/
u8 p_dr11_I2cSendB(u8 sla, u8 c)
{
	G_dr01_I2cError = 0;	
	while(1)
	{
		p_dr11_I2cStart();
		p_dr11_SendB(DEVICE_ADDRESS);
		if(p_dr11_CheckAck() == 1)
		{
			break;
		}
		else if(G_dr01_I2cError++ > DR01_IIC_ERROR_CNT)
		{
			p_dr11_I2cStop();			
			return(0);
		}
	}
	p_dr11_SendB(sla);
	if(p_dr11_CheckAck() == 0)
	{
		p_dr11_I2cStop();			
		return(0);
	}
	p_dr11_SendB(c);
//--if receive ACK when sending ADDR, not care ACK here
	if(p_dr11_CheckAck() == 0)
	{
		p_dr11_I2cStop();			
		return(0);
	}
	p_dr11_I2cStop();
	return(1);
}


/*============================================================================*
*Function	: bit I2cSendStr(u8 sla, u8 saddr, u8 *s, u8 len)
*Description: send string
*============================================================================*/
u8 p_dr11_I2cSendStr(u8 sla, u8 *s, u8 len)
{
	u8 i;

	G_dr01_I2cError = 0;	
	while(1)
	{
		p_dr11_I2cStart();
		p_dr11_SendB(sla);
		if(p_dr11_CheckAck() == 1)
			break;
		else if(G_dr01_I2cError++ > DR01_IIC_ERROR_CNT)
		{
			p_dr11_I2cStop();			
			return(0);
		}
	}

	for(i=0; i<len; i++)
	{
		p_dr11_SendB(*s);
//--if receive ACK when sending ADDR, not care ACK here
		if(p_dr11_CheckAck() == 0)
		{
			p_dr11_I2cStop();			
			return(0);
		}
		
		s++;
	}
	p_dr11_I2cStop();
	return(1);
}
u8 p_dr11_I2cRecB(u8 sla,u8 *c)
{
	u8 i;
	
	G_dr01_I2cError = 0;	
	while(1)
	{
		p_dr11_I2cStart();
		p_dr11_SendB(DEVICE_ADDRESS);
		if(p_dr11_CheckAck() == 1)
			break;
		else if(G_dr01_I2cError++ > DR01_IIC_ERROR_CNT)
		{
			p_dr11_I2cStop();			
			return(0);
		}
	}
	p_dr11_SendB(sla);
	if(p_dr11_CheckAck() == 0)
	{
			p_dr11_I2cStop();			
			return(0);
	}
	while(1)
	{
		p_dr11_I2cStart();
		p_dr11_SendB(DEVICE_ADDRESS+1);
		if(p_dr11_CheckAck() == 1)
			break;
		else if(G_dr01_I2cError++ > DR01_IIC_ERROR_CNT)
		{
			p_dr11_I2cStop();			
			return(0);
		}
	}
	*c = p_dr11_RcvB();
	p_dr11_I2cAck(1);				//no ack
	p_dr11_I2cStop();
	return(1);
}

/*============================================================================*
*Function	: bit I2cRecB(u8 sla, u8 *c)
*Description: receive data including start bit-8bit adress-8bit data-ack-stop bit
*============================================================================*/
u8 p_dr11_I2cRecStr(u8 sla, u8 *c, u8 len)
{
	u8 i;
	
	G_dr01_I2cError = 0;	
	while(1)
	{
		p_dr11_I2cStart();
		p_dr11_SendB(sla+1);
		
		if(p_dr11_CheckAck() == 1)
			break;
		else if(G_dr01_I2cError++ > DR01_IIC_ERROR_CNT)
		{
			p_dr11_I2cStop();			
			return(0);
		}
	}
	
	for(i=0; i<(len-1); i++)
	{
		*c = p_dr11_RcvB();
		c++;
		p_dr11_I2cAck(0);				//ack
	}

	*c = p_dr11_RcvB();
	p_dr11_I2cAck(1);				//no ack
	p_dr11_I2cStop();
	return(1);
}

u8 p_dr11_I2cRecStrWithoutStop(u8 sla, u8 *c, u8 len)
{
	u8 i;

	G_dr01_I2cError = 0;	
	while(1)
	{
		p_dr11_I2cStart();
		p_dr11_SendB(sla+1);
		
		if(p_dr11_CheckAck() == 1)
			break;
		else if(G_dr01_I2cError++ > DR01_IIC_ERROR_CNT)
		{
			p_dr11_I2cStop();			
			return(0);
		}
	}

	for(i=0; i<len; i++)
	{
		*c = p_dr11_RcvB();
		c++;
		p_dr11_I2cAck(0);				//ack
	}
//	p_dr11_I2cStop();					//not send stop
	return(1);
}

void p_dr11_InitI2cBus()
{
//	G_dr01_I2cRWErrorCnt = 0;
	G_dr01_I2cError = 0;
}



