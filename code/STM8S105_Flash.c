#define _STM8S105_FLASH_C_
#include "include.h"

/*******************************************************************************
*函数介绍: MCU_Flash_Init
*输入参数: 无
*输出参数: 无
*返 回 值: 无
*******************************************************************************/
void MCU_Flash_Init(void)
{
	FLASH_SetProgrammingTime(FLASH_PROGRAMTIME_STANDARD);
	FLASH_Unlock(FLASH_MEMTYPE_DATA);
}

/*******************************************************************************
*函数介绍: Sets the fixed programming time
*输入参数: 无
*输出参数: 无
*返 回 值: 无
*******************************************************************************/
void FLASH_SetProgrammingTime(FLASH_ProgramTime_TypeDef FLASH_ProgTime)
{
	FLASH->CR1 &= (u8)(~FLASH_CR1_FIX);
	FLASH->CR1 |= (u8)FLASH_ProgTime;
}

/*******************************************************************************
*函数介绍: Unlocks the program or data EEPROM memory
*输入参数: 无
*输出参数: 无
*返 回 值: 无
*******************************************************************************/
void FLASH_Unlock(FLASH_MemType_TypeDef FLASH_MemType)
{
	if (FLASH_MemType == FLASH_MEMTYPE_PROG)	// Unlock program memory
	{
		FLASH->PUKR = FLASH_RASS_KEY1;
		FLASH->PUKR = FLASH_RASS_KEY2;
	}
	else	// Unlock data memory
	{
		FLASH->DUKR = FLASH_RASS_KEY2;	// Warning: keys are reversed on data memory !!!
		FLASH->DUKR = FLASH_RASS_KEY1;
	}
}

/*******************************************************************************
*函数介绍: Reads any byte from flash memory
*输入参数: Address : Address to read
*输出参数: 无
*返 回 值: Value of the byte
*******************************************************************************/
u8 FLASH_ReadByte(u32 Address)
{
	return(*(PointerAttr u8 *) (u16)Address); 
}

/*******************************************************************************
*函数介绍: Programs one byte in program or data EEPROM memory
*输入参数: 
*输出参数: 无
*返 回 值: 
*******************************************************************************/
void FLASH_ProgramByte(u32 Address, u8 Data)
{
	*(PointerAttr u8*) (u16)Address = Data;
}

