#define _STM8S105_AWU_C_
#include "include.h"

/********************************************************************
Function Name:	AWU_IRQHandler
Description:	
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
@near @interrupt void AWU_IRQHandler (void)
{
	#if 0	//susan- 121105
	AWU_GetFlagStatus();	// In order to detect unexpected events during development, it is recommended to set a breakpoint on the following instruction.

	MCU_ADC_Init();
	#endif
	
	return;
}

/********************************************************************
Function Name:	AWU_GetFlagStatus
Description:	Returns status of the AWU peripheral flag.
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
FlagStatus AWU_GetFlagStatus(void)
{
	return((FlagStatus)(((u8)(AWU->CSR & AWU_CSR_AWUF) == (u8)0x00) ? RESET : SET));
}

/********************************************************************
Function Name:	AWU_Config
Description:	
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
void AWU_Config(u8 TBR, u8 APR)
{
	CLK->PCKENR2 |= 0x04;	// Enable the clock PCG for AWU

	AWU->APR = APR;
	AWU->TBR = TBR;

	AWU->CSR |= 0x10;		// Enable the AWU feature
}

/********************************************************************
Function Name:	AWU_DeInit
Description:	
Create Date:	
Created by:		
Modify Date:	
Modified by:	
Input:			
Output:			
Return:			
********************************************************************/
void AWU_DeInit(void)
{
	AWU->CSR = AWU_CSR_RESET_VALUE;
	AWU->APR = AWU_APR_RESET_VALUE;
	AWU->TBR = AWU_TBR_RESET_VALUE;
}

