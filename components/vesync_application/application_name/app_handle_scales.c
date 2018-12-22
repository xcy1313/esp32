/**
 * @file app_handle_uart.C
 * @brief 处理和体脂称硬件逻辑相关功能
 * @author Jiven
 * @date 2018-12-18
 */
#include "app_handle_scales.h"
#include "app_handle_phone.h"
#include "vesync_uart.h"
#include "vesync_bt_hal.h"
#include "vesync_flash.h"
#include "vesync_button.h"
#include "freertos/timers.h"
#include "vesync_production.h"
#include "app_public_events.h"

#include "vesync_log.h"

#include "app_body_fat_calc.h"

static const char* TAG = "app_handle_scales";

RESEND_COMD_BIT resend_cmd_bit;
uni_frame_t uart_frame = {0};
hw_info info_str;
static TimerHandle_t uart_resend_timer;

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
			if(frame->frame_data_len >2){  //主机请求效应设备返回的数据或设备主动上传的数据
				opt = &frame->frame_data[1];    //过滤命令id字节
				switch(frame->frame_cmd){
					case CMD_BODY_WEIGHT:{
						static uint8_t cnt =0;
						resp_cnt =&cnt;
						*(uint16_t *)&bt_command = CMD_REPORT_WEIGHT;
						memcpy((uint8_t *)&res->response_weight_data.weight,opt,frame->frame_data_len-1);
						printf("\r\n weight =%d ,lb = %d,if =0x%x ,unit =0x%x,imped =0x%04x\r\n",res->response_weight_data.weight,\
																					res->response_weight_data.lb,\
																					res->response_weight_data.if_stabil,\
																					res->response_weight_data.measu_unit,\
																					res->response_weight_data.imped_value);                                  
						// 添加根据当前返回阻抗值来判断是否为绑定用户的体重数据来决定是否对当前数据记录并存储的功能;
						res_ctl.data = 0;       //表示设备主动上传
						if(body_fat_person(vesync_bt_connected(),res,&res->response_weight_data)){
							LOG_I(TAG, "------>the same person! \r\n");
						}
						cnt++;
					}  
					break;
					case CMD_HW_VN:{
						static uint8_t cnt =0;
						resp_cnt =&cnt;

						*(uint16_t *)&bt_command = CMD_REPORT_VESION;
						memcpy((uint8_t *)&res->response_version_data.hardware,opt,frame->frame_data_len-1);
						// printf("\r\n hardware =0x%02x ,firmware =0x%02x ,protocol =0x%02x\r\n",res->response_version_data.hardware,
						// 														res->response_version_data.firmware,
						// 														res->response_version_data.protocol);
						res_ctl.data = 0;       //表示设备主动上传
						cnt++;
					}
					break;
					case CMD_ID:{
						static uint8_t cnt =0;
						resp_cnt =&cnt;

						*(uint16_t *)&bt_command = CMD_REPORT_CODING;
						res_ctl.data = 0;       //表示设备主动上传
						memcpy((uint8_t *)&res->response_encodeing_data.type,opt,frame->frame_data_len-1);
						//printf("\r\n type =0x%02x ,item =0x%02x\r\n",res->response_encodeing_data.type,res->response_encodeing_data.item);
						cnt++;
					}
					break;
					case CMD_POWER_BATTERY:{
						static uint8_t cnt =0;
						static uint8_t opwer_status =0;
						static uint8_t npwer_status =0;
						resp_cnt =&cnt;

						res_ctl.data = 0;       //表示设备主动上传
						*(uint16_t *)&bt_command = CMD_REPORT_POWER;
						memcpy((uint8_t *)&res->response_hardstate.power,opt,frame->frame_data_len-1);
						//printf("\r\n power =0x%02x ,battery_per =0x%02x\r\n",res->response_hardstate.power,res->response_hardstate.battery_level);

						opwer_status = npwer_status;
						npwer_status = res->response_hardstate.power;
						cnt++;
						if((npwer_status == 0) && (opwer_status == 1)){         //关机
							LOG_I(TAG, "scales power down!!!");
							//vesync_power_save_enter(WAKE_UP_PIN);
							//vesync_bt_advertise_stop();
						}else if((npwer_status == 1) && (opwer_status == 0)){   //开机
							LOG_I(TAG, "scales power on!!!");
							//vesync_bt_advertise_start(ADVER_TIME_OUT);
							LOG_I(TAG, "unit is %d\r\n" ,info_str.user_config_data.measu_unit);
							resend_cmd_bit |= RESEND_CMD_MEASURE_UNIT_BIT;
							app_uart_encode_send(MASTER_SET,CMD_MEASURE_UNIT,&info_str.user_config_data.measu_unit,sizeof(uint8_t),true);
						}
					}
					break;
					case CMD_HADRWARE_ERROR:{
						static uint8_t cnt =0;
						resp_cnt =&cnt;
						*(uint16_t *)&bt_command = CMD_REPORT_ERRPR;
						res->response_error_notice.error.para = *(uint32_t *)&opt[0];
						printf("\r\n error type =0x%04x\r\n",res->response_error_notice.error.para);
						cnt++;
					}
					break;
					default:
					break;
				}
				vesync_bt_notify(res_ctl,resp_cnt,bt_command,(uint8_t *)opt ,frame->frame_data_len-1);  //透传控制码
			}else{	//设备返回的应答
				static uint8_t *resp_cnt =NULL;
				 printf("\n<---------------------\n");
                        for(uint8_t i=0;i< len;i++){
                            printf("%02x ",data[i]);
                        }
                printf("\n");
				switch(frame->frame_cmd){
					case CMD_MEASURE_UNIT:{
							static uint8_t cnt =0;
							resp_cnt =&cnt;
							vesync_bt_notify(res_ctl,resp_cnt,CMD_SET_WEIGHT_UNIT,&info_str.user_config_data.measu_unit,sizeof(uint8_t));
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
					default:
						break;
				}
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
    ESP_LOGI(TAG, "key [%d]\r\n" ,*(uint8_t *)p_event_data);
    switch(*(uint8_t *)p_event_data){
        case Short_key:{
				//xTaskNotify(app_public_events_task, NET_CONFIG_NOTIFY_BIT, eSetBits);
				uint8_t backup_unix = info_str.user_config_data.measu_unit;
				if(backup_unix == UNIT_LB){
					backup_unix = UNIT_KG;
				}else if(backup_unix == UNIT_KG){
					backup_unix = UNIT_ST;
				}else if(backup_unix == UNIT_ST){
					backup_unix = UNIT_LB;
				}
				info_str.user_config_data.measu_unit = backup_unix;
				//vesync_flash_write_i8(CONFIG_NAMESPACE,config_unit,info_str.user_config_data.measu_unit);
				resend_cmd_bit |= RESEND_CMD_MEASURE_UNIT_BIT;
				app_uart_encode_send(MASTER_SET,CMD_MEASURE_UNIT,(unsigned char *)&info_str.user_config_data.measu_unit,sizeof(uint8_t),true);
            }
			return;
        case Double_key:
			return;
        case Reapet_key:
			return;
        case Very_Long_key:

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
    ESP_LOGI(TAG, "---------------------->uart resend timer stop");

    if(resend_cmd_bit & RESEND_CMD_MEASURE_UNIT_BIT){
        ESP_LOGI(TAG, "RESEND_CMD_MEASURE_UNIT_BIT");
        app_uart_encode_send(MASTER_SET,CMD_MEASURE_UNIT,(unsigned char *)&info_str.user_config_data.measu_unit,sizeof(uint8_t),true);
    }
    if(resend_cmd_bit & RESEND_CMD_BT_STATUS_BIT){
		uint8_t bt_conn;
        ESP_LOGI(TAG, "RESEND_CMD_BT_STATUS_BIT");
		bt_conn = vesync_bt_connected()?CMD_BT_STATUS_CONNTED:CMD_BT_STATUS_DISCONNECT;
        app_uart_encode_send(MASTER_SET,CMD_BT_STATUS,(unsigned char *)&bt_conn,sizeof(bt_conn),true);
    }
    if(resend_cmd_bit & RESEND_CMD_BODY_FAT_BIT){
        ESP_LOGI(TAG, "RESEND_CMD_BODY_FAT_BIT");
        app_uart_encode_send(MASTER_SET,CMD_BODY_FAT,(unsigned char *)&info_str.user_fat_data.fat,sizeof(info_str.user_fat_data)-28,true);
    }
    if(resend_cmd_bit & RESEND_CMD_WIFI_STATUS_BIT){
        // ESP_LOGI(TAG, "RESEND_CMD_WIFI_STATUS_BIT");
        // uint8_t wifi_status = vesync_wifi_get_status();
        // uint8_t wifi_conn =0 ;
        // if(wifi_status == STATION_GOT_IP){
        //     wifi_conn = 2;
        //     app_uart_encode_send(MASTER_SET,CMD_WIFI_STATUS,(char *)&wifi_conn,sizeof(uint8_t),true);
        // }else if(wifi_status == STATION_CONNECTING){
        //     wifi_conn = 1;
        //     app_uart_encode_send(MASTER_SET,CMD_WIFI_STATUS,(char *)&wifi_conn,sizeof(uint8_t),true);
        // }
        // else if(wifi_status == STATION_DIS_CONNECTED){
        //     wifi_conn = 0;
        //     app_uart_encode_send(MASTER_SET,CMD_WIFI_STATUS,(char *)&wifi_conn,sizeof(uint8_t),true);
        // }
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
	printf("\n--------------------->\n");
    for(uint8_t i=0;i<len;i++){
        printf("%02x ",data[i]);
    }
    printf("\n");

	if(resend){
		if(resend){
			app_uart_resend_timer_stop(); 
			app_uart_resend_timer_start(); 
		}
	}
}
/**
 * @brief uart创建重传定时器
 * @return true 
 * @return false 
 */
static bool app_uart_resend_timer_init(void)
{
    uart_resend_timer = xTimerCreate("uart_resend_timer_timer", 500 / portTICK_PERIOD_MS, pdFALSE,
                                         NULL, app_uart_resend_timerout_callback);
    if (uart_resend_timer == NULL){
        return false;
    }else{
        return true;
    }
}

/**
 * @brief 体脂称硬件功能初始化
 */
void app_scales_start(void)
{
	uint8_t unit;
	app_uart_init();
    if(app_uart_resend_timer_init() == false){
        ESP_LOGE(TAG, "vesync_uart_resend_timer_init fail");
    }

	vesync_flash_config(true ,USER_HISTORY_DATA_NAMESPACE);//初始化用户沉淀数据flash区域
	vesync_flash_config(true ,USER_MODEL_NAMESPACE);	//初始化用户模型flash区域
	vesync_button_init(BUTTON_KEY,app_button_event_handler);

	if(ESP_OK == vesync_flash_read_i8(CONFIG_NAMESPACE,config_unit,&unit)){
        switch(unit){
            case UNIT_KG:
            case UNIT_LB:
            case UNIT_ST:
                info_str.user_config_data.measu_unit = unit;
                ESP_LOGI(TAG, "read flash unit %d",info_str.user_config_data.measu_unit);
            break;
            default:
                info_str.user_config_data.measu_unit = UNIT_KG;
                ESP_LOGI(TAG, "read unit error");
                break;
        }
    }
}



