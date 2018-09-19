#ifndef _STM8S105_AWU_H_
	#define _STM8S105_AWU_H_

#ifdef _STM8S105_AWU_C_
	#define OS_EXT
#else
	#define OS_EXT extern
#endif



OS_EXT void @near AWU_IRQHandler(void);		// AWU ÖÐ”à³ÌÐò
OS_EXT FlagStatus AWU_GetFlagStatus(void);
OS_EXT void AWU_Config(u8 TBR, u8 APR);
OS_EXT void AWU_DeInit(void);

	#undef OS_EXT
#endif


