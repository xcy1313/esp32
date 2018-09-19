#define _STM8S105_FLASH_C_
#include "include.h"

/*******************************************************************************
*��������: MCU_Flash_Init
*�������: ��
*�������: ��
*�� �� ֵ: ��
*******************************************************************************/
void MCU_Flash_Init(void)
{
	FLASH_SetProgrammingTime(FLASH_PROGRAMTIME_STANDARD);
	FLASH_Unlock(FLASH_MEMTYPE_DATA);
}

/*******************************************************************************
*��������: Sets the fixed programming time
*�������: ��
*�������: ��
*�� �� ֵ: ��
*******************************************************************************/
void FLASH_SetProgrammingTime(FLASH_ProgramTime_TypeDef FLASH_ProgTime)
{
	FLASH->CR1 &= (u8)(~FLASH_CR1_FIX);
	FLASH->CR1 |= (u8)FLASH_ProgTime;
}

/*******************************************************************************
*��������: Unlocks the program or data EEPROM memory
*�������: ��
*�������: ��
*�� �� ֵ: ��
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
*��������: Reads any byte from flash memory
*�������: Address : Address to read
*�������: ��
*�� �� ֵ: Value of the byte
*******************************************************************************/
u8 FLASH_ReadByte(u32 Address)
{
	return(*(PointerAttr u8 *) (u16)Address); 
}

/*******************************************************************************
*��������: Programs one byte in program or data EEPROM memory
*�������: 
*�������: ��
*�� �� ֵ: 
*******************************************************************************/
void FLASH_ProgramByte(u32 Address, u8 Data)
{
	*(PointerAttr u8*) (u16)Address = Data;
}

