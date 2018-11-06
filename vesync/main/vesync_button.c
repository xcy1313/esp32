#include "vesync_button.h"

#include <stdarg.h>
#include "user_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_sleep.h"

static const char *TAG = "Vesync_BUTTON";

volatile uint8_t 	FKEY;    
volatile uint8_t 	OKEY; 
volatile uint8_t 	NKEY;  
uint8_t 			key_press; 
vesync_button_cb_t  m_button_handler;

/**
 * @brief Get the Key Value Control Task object
 * @return uint8_t 
 */
static uint8_t GetKeyValueControlTask(void)
{	
	if(gpio_get_level(GPIO_KEY) == 0)
		FKEY = KEY_BUTTON2 ; 
	else 		 		       		           
		FKEY = KEY_NULL	;  
	return FKEY;    
}   

/**
 * @brief 
 * @return uint8_t 
 */
static uint8_t key_driver_mfc_button(void)
{ 
    static uint16_t key_state = key_state_0, key_time = 0; 
    uint8_t  key_return = None_key;      
 
    key_press = GetKeyValueControlTask();                      		      

    switch (key_state) {                
      case key_state_0:                                		
        if (key_press == KEY_BUTTON2){
			key_state = key_state_1;      		
			key_time = 0;    
        } 
		break; 
      case key_state_1:                      				// 按键消抖
		if (key_press == KEY_BUTTON2){ 
			key_time = 0;                     
			key_state = key_state_2;  
		} 
		else 
			key_state = key_state_0;   					 
      break; 
      case key_state_2: 
        if (key_press != KEY_BUTTON2){ 
			key_return = Short_key;        				 
			key_state = key_state_0; 
			key_time =0;	
        } 
        else{ 
			if(key_time++ >= LongTimePressDown){
				key_time =0;		
				key_return = Reapet_key;        			 
				key_state = key_state_3;
			}    
        } 
		break; 
      case key_state_3:
			if (key_press != KEY_BUTTON2){ 
				key_state = key_state_0; 		
			}
			break;                          
    } 
    return key_return;  
}  

/**
 * @brief 
 * @return uint8_t 
 */
static uint8_t key_read_mfc_button(void){ 
    static uint8_t key_m = key_state_0, key_time_1 = 0; 
    uint8_t key_return = None_key,key_temp;         
     
    key_temp = key_driver_mfc_button();  
     
    switch(key_m){ 
        case key_state_0: 
            if (key_temp == Short_key){ 
				key_time_1 = 0;               				
				key_m = key_state_1; 
            } 
            else 
                key_return = key_temp;        
        break;  

        case key_state_1: 
            if (key_temp == Short_key){ 
				key_return = Double_key;        
				key_m = key_state_0;  
            } 
            else{                                 				 
				if(++key_time_1 >= 15){ 
					key_return = Short_key;      			
					key_m = key_state_0;     				// 返回初始状态 
            	}   
             }        
        break;   
    }
    return key_return;  
}

/**
 * @brief 
 * @param callback 
 */
void ButtonHandle(ButtonCallBack callback)
{
    volatile uint8_t CurrentKey;  
    
    CurrentKey = key_read_mfc_button();
	
    switch(CurrentKey){
		case Short_key:
		case Double_key: 
		case Reapet_key: 
		case Very_Long_key:
				callback(CurrentKey);          
			break;
		default: 
				callback(CurrentKey);
			break;
    }
}

/**
 * @brief 
 * @param button_type 
 */
void buttonevents(uint8_t button_type){
	if(button_type != None_key){
		if(m_button_handler != NULL){
			m_button_handler(&button_type);
		}
	}
}

/**
 * @brief 
 */
void ButtonsPatternsTranslate(void)
{	
	ButtonHandle(buttonevents);
}   

/**
 * @brief 
 * @param pvParameters 
 */
static void vesync_button_task_handler(void *pvParameters){
	while(1){
		ButtonsPatternsTranslate();
		vTaskDelay(20 / portTICK_PERIOD_MS);	//正常使用10ms；
	}
	vTaskDelete(NULL);
}

/**
 * @brief 
 * @param num 
 * @return gpio_config_t 
 */
static gpio_config_t init_io(gpio_num_t num)
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << num);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;
    return io_conf;
}

/**
 * @brief 
 * @param cb 
 */
void vesync_button_init(vesync_button_cb_t cb)
{
    gpio_config_t io_config = init_io(GPIO_KEY);
    io_config.mode = GPIO_MODE_INPUT;
    gpio_config(&io_config);

	gpio_wakeup_enable(17, GPIO_INTR_LOW_LEVEL);	//串口RX为唤醒脚 低电平有效
	//esp_deep_sleep_start();

	m_button_handler = cb;
	xTaskCreate(&vesync_button_task_handler, "vesync_button_task_handler", 4096, NULL, 2, NULL);
}

	