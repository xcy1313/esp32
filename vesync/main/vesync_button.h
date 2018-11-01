
#ifndef _VESYNC_BUTTON_H
#define _VESYNC_BUTTON_H

#include "c_types.h"

#define LongTimePressDown   	75//150--->75 1.5s

#define key_state_0 	0 
#define key_state_1 	1 
#define key_state_2 	2    
#define key_state_3 	3        

#define press		0
#define release	1

#define None_key    	0            //无键 
#define Short_key    	1            //单键 
#define Double_key    	2            //双键 
#define Reapet_key    	3            //长键 
#define Very_Long_key   4            //长长键 

#define GPIO_KEY  		12

#define KEY_NULL                    0x00  
#define KEY_BUTTON0                 0x01
#define KEY_BUTTON1                 0x02
#define KEY_BUTTON2                 0x04
#define KEY_BUTTON3                 0x08
#define KEY_BUTTON4                 0x10
#define KEY_BUTTON5                 0x20
#define KEY_BUTTON6                 0x40
#define KEY_BUTTON7                 0x80

typedef void (* ButtonCallBack)(uint8_t message);
typedef void (*vesync_button_cb_t)(void*);

void vesync_button_init(vesync_button_cb_t cb);








#endif

