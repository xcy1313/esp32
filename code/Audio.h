#ifndef _AUDIO_H_
	#define _AUDIO_H_

#ifdef _AUDIO_C_
	#define OS_EXT
#else
	#define OS_EXT extern
#endif

#define NO_AUDIO			0x00
#define LINE_IN_AUDIO		0x01
#define BLUETOOTH_AUDIO		0x02

#define MAX_VOLUME		32
#define MIN_VOLUME		0
#define DEFLAUT_VOLUME		20
OS_EXT bool bMaxVolume;
OS_EXT u8   SourceSelect;			// …˘‘¥—°‘Ò,¿∂—¿ªÚLINE-IN
OS_EXT bool bSleepStau;
OS_EXT u8 u8Volume;
OS_EXT void Audio_SetAudioSource(u8 source);
OS_EXT void Audio_AudioSleepCheck(void);

void AudioHandler(void);
	#undef OS_EXT
#endif

