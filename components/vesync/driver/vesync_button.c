/**
 * @file vesync_button.c
 * @brief 按键功能驱动
 * @author Jiven
 * @date 2018-12-12
 */
#include "vesync_button.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "driver/gpio.h"

static TimerHandle_t button_timer;

uint8_t pin_key;
volatile uint8_t 	FKEY;    
uint8_t 			key_press; 
static vesync_button_cb_t  m_button_handler;

/**
 * @brief Get the Key Value Control Task object
 * @return uint8_t 
 */
static uint8_t GetKeyValueControlTask(void)
{	
	if(gpio_get_level(pin_key) == 0)
		FKEY = KEY_BUTTON_VALUE ; 
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
        if (key_press == KEY_BUTTON_VALUE){
			key_state = key_state_1;      		
			key_time = 0;    
        } 
		break; 
      case key_state_1:                      				// 按键消抖
		if (key_press == KEY_BUTTON_VALUE){ 
			key_time = 0;                     
			key_state = key_state_2;  
		} 
		else 
			key_state = key_state_0;   					 
      break; 
      case key_state_2: 
        if (key_press != KEY_BUTTON_VALUE){ 
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
			if (key_press != KEY_BUTTON_VALUE){ 
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
static void vesynv_button_callback(TimerHandle_t timer)
{
	ButtonsPatternsTranslate();
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
 * @brief 按键注册
 * @param cb 
 */
void vesync_button_init(uint32_t pin,vesync_button_cb_t cb)
{
    gpio_config_t io_config = init_io(pin);
    gpio_config(&io_config);

	pin_key = pin;
	m_button_handler = cb;
	button_timer = xTimerCreate("button_timer", 20 / portTICK_PERIOD_MS, true,
										NULL, vesynv_button_callback);
	xTimerStart(button_timer, portMAX_DELAY);
}

	