#ifndef _STM8S105_FLASH_H_
	#define _STM8S105_FLASH_H_

#ifdef _STM8S105_FLASH_C_
	#define OS_EXT
#else
	#define OS_EXT extern
#endif

typedef enum
{
	FLASH_PROGRAMTIME_STANDARD = (uint8_t)0x00,	// Standard programming time fixed at 1/2 tprog
	FLASH_PROGRAMTIME_TPROG    = (uint8_t)0x01	// Programming time fixed at tprog
}FLASH_ProgramTime_TypeDef;

typedef enum
{
	FLASH_MEMTYPE_PROG = (uint8_t)0xFD,	// Program memory
	FLASH_MEMTYPE_DATA = (uint8_t)0xF7	// Data EEPROM memory
}FLASH_MemType_TypeDef;

#define FLASH_RASS_KEY1 ((uint8_t)0x56) // First RASS key
#define FLASH_RASS_KEY2 ((uint8_t)0xAE) // Second RASS key


OS_EXT void MCU_Flash_Init(void);

OS_EXT void FLASH_SetProgrammingTime(FLASH_ProgramTime_TypeDef FLASH_ProgTime);

OS_EXT void FLASH_Unlock(FLASH_MemType_TypeDef FLASH_MemType);
OS_EXT u8	FLASH_ReadByte(u32 Address);
OS_EXT void FLASH_ProgramByte(u32 Address, u8 Data);

	#undef OS_EXT
#endif

