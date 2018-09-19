#ifndef _EEPROM_H_
	#define _EEPROM_H_

#ifdef _EEPROM_C_
	#define OS_EXT
#else
	#define OS_EXT extern
#endif

// 0x4000 жа 0x407F

#define SOURCE_SELECT_ADDR		(0x4000)





	#undef OS_EXT
#endif

