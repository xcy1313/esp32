/* == IDENTIFICATION ==================================================
 *
 * Copyright (C) 2008, Plastoform Industries Ltd.
 *
 * System           : STM105K series
 * Module           : driver -- I2c
 * File             : cdr01iic.h
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
 * This module contains public definitions and prototypes of this driver                      
 *                                                                      
 * Global functions to be called by other layers:
 * ----------------------------------------------
 *
 * Layer-local functions to be called from inside the DRIVER layer:
 * ---------------------------------------------------------------
 * I2cStart
 * I2cStop
 * SendB
 * RcvB
 * I2cAck
 * I2cSendB
 * I2cSendStr
 * I2cRecB
 */



/******************************************************************************/
#ifndef __CDR01IIC_H_
#define __CDR01IIC_H_
/******************************************************************************/

/* Includes ------------------------------------------------------------------*/


#ifdef _CDR11IIC_C_
#define GLOBAL
#else
#define GLOBAL extern
#endif
/*
example:
GLOBAL char variable
GLOBAL bit flag
*/

#define	DR01_IIC_RW_ERROR_CNT	3
#define	DR01_IIC_ERROR_CNT		3



#define Nop_Sub() _asm("nop");_asm("nop");//_asm("nop");_asm("nop");

//#define DELAY_TIME	5

//GLOBAL u8 G_dr01_I2cRWErrorCnt;
GLOBAL u8 G_dr01_I2cError;

/******************************************************************************/
/*         F u n c t i o n      D e f i n e                                   */
/******************************************************************************/

void p_dr11_I2cStart(void);
void p_dr11_I2cStop(void);
void p_dr11_SendB(u8 c);
u8 p_dr11_RcvB(void);
void p_dr11_I2cAck(u8 a);


u8 p_dr11_I2cSendB(u8 sla, u8 c);
u8 p_dr11_I2cSendStr(u8 sla, u8 *s, u8 len);
u8 p_dr11_I2cRecB(u8 sla,u8 *c);		//susan+ 121029
u8 p_dr11_I2cRecStr(u8 sla, u8 *c, u8 len);
u8 p_dr11_I2cRecStrWithoutStop(u8 sla, u8 *c, u8 len);

void p_dr11_InitI2cBus(void);

#undef GLOBAL


/******************************************************************************/
#endif
/******************************************************************************/
