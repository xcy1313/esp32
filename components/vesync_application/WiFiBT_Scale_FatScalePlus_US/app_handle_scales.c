/**
 * @file app_handle_uart.C
 * @brief 处理和体脂称硬件逻辑相关功能
 * @author Jiven
 * @date 2018-12-18
 */
#include "app_handle_scales.h"
#include "app_handle_phone.h"
#include "vesync_power_management.h"
#include "vesync_uart.h"
#include "vesync_bt_hal.h"
#include "vesync_wifi.h"
#include "vesync_wifi_hal.h"
#include "vesync_flash.h"
#include "vesync_button.h"
#include "freertos/timers.h"
#include "vesync_production.h"
#include "vesync_interface.h"
#include "vesync_device.h"
#include "app_public_events.h"
#include "vesync_flash.h"
#include "driver/rtc_io.h"
#include "driver/adc.h"

#include "vesync_log.h"

#include "app_body_fat_calc.h"
#include "app_handle_server.h"

static const char* TAG = "app_handle_scales";

RESEND_COMD_BIT resend_cmd_bit =0;
uni_frame_t uart_frame = {0};
hw_info info_str;
static TimerHandle_t uart_resend_timer;
static bool app_uart_resend_timer_stop(void);
static bool bmask_scale = false;

/**
 * @brief  进入深度休眠模式 io中断唤醒 rtc内部上拉 低电平唤醒
 * @param pin 
 */
static void app_power_save_enter(void)
{
    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
    ESP_LOGI(TAG,"sleep source %d\r\n",cause);

    switch (esp_sleep_get_wakeup_cause()){
        case ESP_SLEEP_WAKEUP_EXT1: {
            uint64_t wakeup_pin_mask = esp_sleep_get_ext1_wakeup_status();
            if (wakeup_pin_mask == 0) {
                int pin = __builtin_ffsll(wakeup_pin_mask) - 1;
                printf("Wake up from GPIO %d\n", pin);
            }else{
                printf("Wake up from other GPIO\n");
            }
            break;
        }
        default:
            break;
    }
    //rtc_gpio_isolate(GPIO_NUM_12);

    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
    gpio_pullup_en(WAKE_UP_PIN);
    gpio_pulldown_dis(WAKE_UP_PIN);

	gpio_pullup_en(BUTTON_KEY);
	gpio_pulldown_dis(BUTTON_KEY);

    adc_power_off();    //不添加增加1.4mah功耗
    const int ext_wakeup_pin_1 = WAKE_UP_PIN;
    const uint64_t ext_wakeup_pin_1_mask = 1ULL << ext_wakeup_pin_1;

	const int ext_wakeup_pin_0 = BUTTON_KEY;
    const uint64_t ext_wakeup_pin_0_mask = 1ULL << ext_wakeup_pin_0;

    esp_sleep_enable_ext1_wakeup(ext_wakeup_pin_1_mask, ESP_EXT1_WAKEUP_ALL_LOW);
	esp_sleep_enable_ext0_wakeup(ext_wakeup_pin_0_mask, ESP_EXT1_WAKEUP_ALL_LOW);

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    esp_deep_sleep_start();
}

void app_scales_power_on(void)
{
	if(PRODUCTION_EXIT != vesync_get_production_status())		return;

	vesync_bt_advertise_start(APP_ADVERTISE_TIMEOUT);
	app_uart_encode_send(MASTER_SET,CMD_MEASURE_UNIT,&info_str.user_config_data.measu_unit,sizeof(uint8_t),true);
	resend_cmd_bit |= RESEND_CMD_MEASURE_UNIT_BIT;

	uint8_t bt_conn;
	bt_conn = vesync_bt_connected()?CMD_BT_STATUS_CONNTED:CMD_BT_STATUS_DISCONNECT;
	app_uart_encode_send(MASTER_SET,CMD_BT_STATUS,(unsigned char *)&bt_conn,sizeof(bt_conn),true);
	resend_cmd_bit |= RESEND_CMD_BT_STATUS_BIT;
	uint8_t wifi_conn =0 ;
	switch(vesync_get_device_status()){
		case DEV_CONFNET_NOT_CON:				//未配网
			wifi_conn = 0;
			app_uart_encode_send(MASTER_SET,CMD_WIFI_STATUS,(unsigned char *)&wifi_conn,sizeof(uint8_t),true);
			resend_cmd_bit |= RESEND_CMD_WIFI_STATUS_BIT;
			break;
		case DEV_CONFNET_ONLINE:				//已连接上服务器
			wifi_conn = 2;
			app_uart_encode_send(MASTER_SET,CMD_WIFI_STATUS,(unsigned char *)&wifi_conn,sizeof(uint8_t),true);
			resend_cmd_bit |= RESEND_CMD_WIFI_STATUS_BIT;
			break;
		case DEV_CONFNET_OFFLINE:				//已配网但未连接上服务器
			wifi_conn = 1;
			app_uart_encode_send(MASTER_SET,CMD_WIFI_STATUS,(unsigned char *)&wifi_conn,sizeof(uint8_t),true);
			resend_cmd_bit |= RESEND_CMD_WIFI_STATUS_BIT;
			break;
		default:
			break;				
	}
}

/**
 * @brief 
 */
void app_scales_power_off(void)
{
	if(PRODUCTION_EXIT != vesync_get_production_status())		return;

	vesync_bt_advertise_stop();
	resend_cmd_bit &= ~RESEND_CMD_ALL_BIT;
	app_uart_resend_timer_stop();	//称体休眠 下发数据无效
	LOG_I(TAG, "scales power down!!!");

	vesync_hal_bt_client_deinit();
    //vesync_wifi_deinit();
    vesync_uart_deint();
	vesync_flash_config(false,"nvs");
	app_power_save_enter();
}
/**
 * @brief 
 * @param reg  用户配置结构体
 * @param data 串口接收数据缓冲
 * @param len  串口接收数据长度
 */
static void app_uart_recv_cb(const unsigned char *data,unsigned short len)
{
	for(unsigned short i=0;i<len;i++){
		uni_frame_t *frame = &uart_frame;
        if(1 == Comm_frame_parse(data[i],0,frame)){        
			frame_ctrl_t res_ctl ={     //应答包res状态  
				.data =0,
			};
			hw_info		*res = &info_str;
			static uint8_t *resp_cnt =NULL;
			uint8_t *opt = NULL;
			uint16_t bt_command = 0;
			opt = &frame->frame_data[1];    //过滤命令id字节
			switch(frame->frame_cmd){
				case CMD_FACTORY_SYNC_START:
						resend_cmd_bit &= ~RESEND_CMD_FACTORY_START_BIT;
					return;
				case CMD_FACTORY_CHARGING:{
						uint8_t charge_status  = opt[0];
						uint8_t charge_percent = opt[1];
						//LOG_I(TAG, "charge_status %d,charge_percent %d ,time %s",charge_status,charge_percent,vesync_get_time());
						if(vesync_get_production_status() == RPODUCTION_RUNNING){
							if((factory_test_bit & FACTORY_TEST_SYNC_CHARGE_BIT) == FACTORY_TEST_SYNC_CHARGE_BIT){
								app_handle_production_report_charge(vesync_get_time(),charge_status,charge_percent);
								factory_test_bit &= ~FACTORY_TEST_SYNC_CHARGE_BIT;
							}
						}
						resend_cmd_bit &= ~RESEND_CMD_FACTORY_CHARGE_BIT;
					}
					return;
				case CMD_FACTORY_WEIGHT:{
						uint16_t weight  = *(uint16_t *)&opt[0];
						uint16_t imped =  *(uint16_t *)&opt[2];
						LOG_I(TAG, "weight %d,imped %d,time %s",weight,imped,vesync_get_time());
						if(vesync_get_production_status() == RPODUCTION_RUNNING){
							if((factory_test_bit & FACTORY_TEST_SYNC_WEIGHT_BIT) == FACTORY_TEST_SYNC_WEIGHT_BIT){
								if(weight > 500){		//5kg
									app_handle_production_report_weight(vesync_get_time(),weight,imped);
									factory_test_bit &= ~FACTORY_TEST_SYNC_WEIGHT_BIT;
									resend_cmd_bit &= ~RESEND_CMD_FACTORY_WEIGHT_BIT;
								}
							}
						}
					}
					return;
				case CMD_FACTORY_SYNC_STOP:
						resend_cmd_bit &= ~RESEND_CMD_FACTORY_STOP_BIT;
					return;	
			}
			if(frame->frame_data_len >2){  //主机请求效应设备返回的数据或设备主动上传的数据
				switch(frame->frame_cmd){
					case CMD_HW_VN:
						memcpy((uint8_t *)&res->response_version_data.hardware,opt,frame->frame_data_len-1);
						printf("\r\n hardware =0x%02x ,firmware =0x%02x ,protocol =0x%02x\r\n",res->response_version_data.hardware,\
																				res->response_version_data.firmware,\
																				res->response_version_data.protocol);
						break;
					case CMD_BODY_WEIGHT:{
						static uint8_t cnt =0;
						resp_cnt =&cnt;
						res_ctl.data = 0x0;       //表示设备主动上传
						*(uint16_t *)&bt_command = CMD_REPORT_WEIGHT;
						memcpy((uint8_t *)&res->response_weight_data.weight,opt,frame->frame_data_len-1);
						printf("\r\n weight =%d ,lb = %d,if =%x ,unit =%x,imped =%d\r\n",res->response_weight_data.weight,\
																					res->response_weight_data.lb,\
																					res->response_weight_data.if_stabil,\
																					res->response_weight_data.measu_unit,\
																					res->response_weight_data.imped_value);                                  
						// 添加根据当前返回阻抗值来判断是否为绑定用户的体重数据来决定是否对当前数据记录并存储的功能;
						if(body_fat_person(vesync_bt_connected(),res,&res->response_weight_data)){
							LOG_I(TAG, "------>the same person! \r\n");
						}
						cnt++;
						vesync_bt_notify(res_ctl,resp_cnt,bt_command,(uint8_t *)opt ,frame->frame_data_len-1);  //透传控制码
					}
					break;
					case CMD_POWER_BATTERY:{
						static uint8_t cnt =0;
						static uint8_t opwer_status =0;
						static uint8_t npwer_status =0;
						resp_cnt =&cnt;

						res_ctl.data = 0x0;       //表示设备主动上传
						*(uint16_t *)&bt_command = CMD_REPORT_POWER;
						memcpy((uint8_t *)&res->response_hardstate.power,opt,frame->frame_data_len-1);
						opwer_status = npwer_status;
						npwer_status = res->response_hardstate.power;

						cnt++;
						uint8_t send_buff[2];
						send_buff[0] = opt[1];		//mcu上报的数据中，是电量在前开关机状态在后
						send_buff[1] = opt[0];		//蓝牙发送的数据中，是开关机状态在前电量在后
						vesync_bt_notify(res_ctl,resp_cnt,bt_command,(uint8_t *)send_buff ,frame->frame_data_len-1);  //透传控制码
						if((npwer_status == 0) && (opwer_status == 1)){         //关机
							LOG_I(TAG,"[-----------------------");
							LOG_I(TAG, "scales power off!!!");
							LOG_I(TAG,"------------------------]");
							vTaskDelay(200 / portTICK_PERIOD_MS);	//正常使用10ms；
							app_scales_power_off();
						}else if((npwer_status == 1) && (opwer_status == 0)){   //开机
							LOG_I(TAG,"[-----------------------");
							LOG_I(TAG, "scales power on!!!");
							LOG_I(TAG,"------------------------]");
							//app_scales_power_on();
						}
					}
					break;
					case CMD_HADRWARE_ERROR:{
						static uint8_t cnt =0;
						resp_cnt =&cnt;
						res_ctl.data = 0x0;       //表示设备主动上传
						*(uint16_t *)&bt_command = CMD_REPORT_ERRPR;
						memcpy(res->response_error_notice.type,(uint8_t *)opt,sizeof(response_error_notice_t));
						LOG_E(TAG,"------------------------");
						LOG_E(TAG,"weight error [%d] ,",res->response_error_notice.type[0]);
						LOG_E(TAG,"zero error [%d] ,",res->response_error_notice.type[1]);
						LOG_E(TAG,"battery low error [%d] ,",res->response_error_notice.type[2]);
						LOG_E(TAG,"imped error [%d] ,",res->response_error_notice.type[3]);
						LOG_E(TAG,"------------------------");
						vesync_bt_notify(res_ctl,resp_cnt,bt_command,res->response_error_notice.type ,sizeof(response_error_notice_t));  //透传控制码
						cnt++;
					}
					break;
					default:
						LOG_E(TAG, "other error cmd\r\n");
					break;
				}
			}else{	//设备返回的应答FACTORY_TEST_SYNC_BUTTON_BIT
				static uint8_t *resp_cnt =NULL;
				switch(frame->frame_cmd){
					case CMD_MEASURE_UNIT:{
							static uint8_t cnt =0;
							resp_cnt =&cnt;
							if(bmask_scale){
								bmask_scale = false;
								*(uint16_t *)&bt_command = CMD_SET_WEIGHT_UNIT;
								vesync_bt_notify(res_ctl,resp_cnt,bt_command,&info_str.user_config_data.measu_unit,sizeof(uint8_t));
							}
							cnt++;
							resend_cmd_bit &= ~RESEND_CMD_MEASURE_UNIT_BIT;
						}
						break;
					case CMD_BT_STATUS:
							resend_cmd_bit &= ~RESEND_CMD_BT_STATUS_BIT;
						break;
					case CMD_WIFI_STATUS:
							resend_cmd_bit &= ~RESEND_CMD_WIFI_STATUS_BIT;
						break;
					case CMD_BODY_FAT:
							resend_cmd_bit &= ~RESEND_CMD_BODY_FAT_BIT;
						break;
					default:
						break;
				}
				LOG_I(TAG, "ack for cmd bits [0x%04x]\r\n" ,resend_cmd_bit);
			}
		}
	}
}

/**
 * @brief 初始化串口功能
 */
static void app_uart_init(void)
{
	vesync_uart_int(UART_RX_PIN,UART_TX_PIN,UART_BAUD_RATE,app_uart_recv_cb);
}

/**
 * @brief 单位按键功能驱动
 * @param p_event_data 
 */
void app_button_event_handler(void *p_event_data){
	static uint8_t enter_factory_mode_cnt =0;
	static uint16_t ochecktime = 0;
	static uint16_t nchecktime = 0; 
	static uint16_t lchecktime = 0; 

    ESP_LOGI(TAG, "key [%d]\r\n" ,*(uint8_t *)p_event_data);
    switch(*(uint8_t *)p_event_data){
        case Short_key:
				if(vesync_get_production_status() >= PRODUCTION_EXIT){
					uint8_t backup_unix = info_str.user_config_data.measu_unit;
					if(backup_unix == UNIT_LB){
						backup_unix = UNIT_KG;
					}else if(backup_unix == UNIT_KG){
						backup_unix = UNIT_ST;
					}else if(backup_unix == UNIT_ST){
						backup_unix = UNIT_LB;
					}
					info_str.user_config_data.measu_unit = backup_unix;
					vesync_flash_write_i8(UNIT_NAMESPACE,UNIT_KEY,info_str.user_config_data.measu_unit);
					resend_cmd_bit |= RESEND_CMD_MEASURE_UNIT_BIT;
					app_uart_encode_send(MASTER_SET,CMD_MEASURE_UNIT,(unsigned char *)&info_str.user_config_data.measu_unit,sizeof(uint8_t),true);
					bmask_scale = true;
					if(enter_factory_mode_cnt == 1){
						ochecktime = xTaskGetTickCount();	//记录当前记录的时间
					}else{
						ochecktime = nchecktime;
					}
					nchecktime = xTaskGetTickCount();		//记录当前记录的时间
					enter_factory_mode_cnt++;

					if(abs(nchecktime-ochecktime) > 200){	//连续4次
						enter_factory_mode_cnt =0;
					} 
					LOG_I(TAG, "ochecktime[%d] nchecktime[%d] time [%d] cnt [%d]\r\n" ,ochecktime,nchecktime,abs(nchecktime-ochecktime),enter_factory_mode_cnt);
				}else if(vesync_get_production_status() == RPODUCTION_RUNNING){
					static uint8_t factory_button_cnt =0;
					if(factory_button_cnt++ >=2){
						if((factory_test_bit & FACTORY_TEST_SYNC_BUTTON_BIT) == FACTORY_TEST_SYNC_BUTTON_BIT){
							app_handle_production_report_button(vesync_get_time(),factory_button_cnt);
							factory_test_bit &= ~FACTORY_TEST_SYNC_BUTTON_BIT;
						}
						factory_button_cnt = 0;
					}
				}
			return;
		case Double_key:
			return;
		case Reapet_key:
			if(PRODUCTION_EXIT == vesync_get_production_status()){
				if(enter_factory_mode_cnt >= 3){
					lchecktime = xTaskGetTickCount();	//记录当前记录的时间
					LOG_I(TAG, "lchecktime[%d]\r\n" ,lchecktime);
					if(abs(lchecktime-nchecktime) < 400){
						uint8_t mac_addr[6];
						enter_factory_mode_cnt =0;
						ESP_LOGE(TAG, "enter factory mode");
						resend_cmd_bit &= ~RESEND_CMD_ALL_BIT;
						resend_cmd_bit |= RESEND_CMD_FACTORY_START_BIT;

						vesync_regist_recvjson_cb(vesync_recv_json_data);
						vesync_enter_production_testmode(NULL);
						esp_wifi_get_mac(ESP_MAC_WIFI_STA, mac_addr);
						app_uart_encode_send(MASTER_SET,CMD_FACTORY_SYNC_START,mac_addr,sizeof(mac_addr),true);
					}
				}
			}
			return;
		case Very_Long_key:{
				static bool factory_set = false;
				if(factory_set == false){
					uint32_t ret;
					factory_set = true;
					ret = vesync_flash_erase_partiton(USER_MODEL_NAMESPACE);
					if(ret != 0){
						LOG_E(TAG, "erase USER_MODEL_NAMESPACE\r\n");
					}
					ret = vesync_flash_erase_partiton(USER_HISTORY_DATA_NAMESPACE);
					if(ret != 0){
						LOG_E(TAG, "erase USER_HISTORY_DATA_NAMESPACE\r\n");
					}
					//esp_restart();
				}
			}
			return;
		default:
			return;  
	}
}

static bool app_uart_resend_timer_stop(void)
{
    bool status = false;

    if (xTimerStop(uart_resend_timer, portMAX_DELAY) != pdPASS) {
        status = false;
    } else {
        status = true;
    }

    return status;
}

static bool app_uart_resend_timer_start(void)
{
    bool status = false;

    app_uart_resend_timer_stop();
    if (xTimerStart(uart_resend_timer, portMAX_DELAY) != pdPASS) {
        status = false;
    } else {
        status =  true;
    }

    return status;
}

static void app_uart_resend_timerout_callback(TimerHandle_t timer)
{
	//ESP_LOGI(TAG, "uart resend timer stop [0x%04x] status =%d" ,resend_cmd_bit,vesync_get_production_status());
	// if(/*(info_str.response_hardstate.power == 0 ) && */(vesync_get_production_status() == PRODUCTION_EXIT)){
	// 	resend_cmd_bit &=~RESEND_CMD_ALL_BIT;
	// 	return;
	// }
	if(vesync_get_production_status() == RPODUCTION_RUNNING){
		if((resend_cmd_bit & RESEND_CMD_FACTORY_CHARGE_BIT) == RESEND_CMD_FACTORY_CHARGE_BIT){
			app_uart_encode_send(MASTER_SET,CMD_FACTORY_CHARGING,0,0,true);
		}
		if((resend_cmd_bit & RESEND_CMD_FACTORY_WEIGHT_BIT) == RESEND_CMD_FACTORY_WEIGHT_BIT){
			app_uart_encode_send(MASTER_SET,CMD_FACTORY_WEIGHT,0,0,true);
		}
	}else if(vesync_get_production_status() == RPODUCTION_START){
		if((resend_cmd_bit & RESEND_CMD_FACTORY_START_BIT) == RESEND_CMD_FACTORY_START_BIT){
			uint8_t mac_addr[6];
			esp_wifi_get_mac(ESP_MAC_WIFI_STA, mac_addr);
			app_uart_encode_send(MASTER_SET,CMD_FACTORY_SYNC_START,mac_addr,sizeof(mac_addr),true);
		}
	}else if(vesync_get_production_status() == PRODUCTION_EXIT){
		if((resend_cmd_bit & RESEND_CMD_FACTORY_STOP_BIT) == RESEND_CMD_FACTORY_STOP_BIT){
			app_uart_encode_send(MASTER_SET,CMD_FACTORY_SYNC_STOP,0,0,true);
		}
	}
	if((resend_cmd_bit & RESEND_CMD_MEASURE_UNIT_BIT) == RESEND_CMD_MEASURE_UNIT_BIT){
		app_uart_encode_send(MASTER_SET,CMD_MEASURE_UNIT,(unsigned char *)&info_str.user_config_data.measu_unit,sizeof(uint8_t),true);
	}
	if((resend_cmd_bit & RESEND_CMD_BT_STATUS_BIT) == RESEND_CMD_BT_STATUS_BIT){
		uint8_t bt_conn;
		bt_conn = vesync_bt_connected()?CMD_BT_STATUS_CONNTED:CMD_BT_STATUS_DISCONNECT;
		app_uart_encode_send(MASTER_SET,CMD_BT_STATUS,(unsigned char *)&bt_conn,sizeof(bt_conn),true);
	}
	if((resend_cmd_bit & RESEND_CMD_BODY_FAT_BIT) == RESEND_CMD_BODY_FAT_BIT){
		app_uart_encode_send(MASTER_SET,CMD_BODY_FAT,(unsigned char *)&info_str.user_fat_data.fat,sizeof(info_str.user_fat_data),true);
	}
	if((resend_cmd_bit & RESEND_CMD_WIFI_STATUS_BIT) == RESEND_CMD_WIFI_STATUS_BIT){
		uint8_t wifi_conn =0 ;
		switch(vesync_get_device_status()){
			case DEV_CONFNET_NOT_CON:				//未配网
				wifi_conn = 0;
				app_uart_encode_send(MASTER_SET,CMD_WIFI_STATUS,(unsigned char *)&wifi_conn,sizeof(uint8_t),true);
				resend_cmd_bit |= RESEND_CMD_WIFI_STATUS_BIT;
				break;
			case DEV_CONFNET_ONLINE:				//已连接上服务器
				wifi_conn = 2;
				app_uart_encode_send(MASTER_SET,CMD_WIFI_STATUS,(unsigned char *)&wifi_conn,sizeof(uint8_t),true);
				resend_cmd_bit |= RESEND_CMD_WIFI_STATUS_BIT;
				break;
			case DEV_CONFNET_OFFLINE:				//已配网但未连接上服务器
				wifi_conn = 1;
				app_uart_encode_send(MASTER_SET,CMD_WIFI_STATUS,(unsigned char *)&wifi_conn,sizeof(uint8_t),true);
				resend_cmd_bit |= RESEND_CMD_WIFI_STATUS_BIT;
				break;
			default:
				break;				
		}
	}
}

/**
 * @brief app层调用串口发送
 * @param ctl 
 * @param cmd 
 * @param data 
 * @param len 
 * @param resend 是否重传
 */
void app_uart_encode_send(uint8_t ctl,uint8_t cmd,const unsigned char *data,uint16_t len,bool resend)
{
	vesync_uart_encode_send(ctl,cmd,data,len);
	if(resend){
		app_uart_resend_timer_start();
	}
}
/**
 * @brief uart创建重传定时器
 * @return true 
 * @return false 
 */
static bool app_uart_resend_timer_init(void)
{
    uart_resend_timer = xTimerCreate("uart_resend_timer_timer", 500 / portTICK_PERIOD_MS, false,
                                         NULL, app_uart_resend_timerout_callback);
    if (uart_resend_timer == NULL){
        return false;
    }else{
        return true;
    }
}

/**
 * @brief 
 */
void app_uart_start(void)
{
	app_uart_init();
    if(app_uart_resend_timer_init() == false){
        ESP_LOGE(TAG, "vesync_uart_resend_timer_init fail");
    }
}
/**
 * @brief 
 */
void app_button_start(void)
{
	vesync_button_init(BUTTON_KEY,app_button_event_handler);
}

/**
 * @brief 体脂称硬件功能初始化
 */
void app_scales_start(void)
{
	uint8_t unit;
	if(vesync_get_device_status() == DEV_CONFNET_NOT_CON){
		ESP_LOGE(TAG, "device not config net!");
	}
	app_button_start();
	//vesync_flash_config(true ,USER_HISTORY_DATA_NAMESPACE);//初始化用户沉淀数据flash区域
	vesync_flash_config(true ,USER_MODEL_NAMESPACE);	//初始化用户模型flash区域
	if(ESP_OK == vesync_flash_read_i8(UNIT_NAMESPACE,UNIT_KEY,&unit)){
        switch(unit){
            case UNIT_KG:
            case UNIT_LB:
            case UNIT_ST:
                info_str.user_config_data.measu_unit = unit;
            break;
            default:
                info_str.user_config_data.measu_unit = UNIT_KG;
                ESP_LOGI(TAG, "read unit error");
                break;
        }
    }
	app_scales_power_on();
}



