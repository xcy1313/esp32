#include "vesync_uart.h"
#include "vesync_wifi.h"
#include "vesync_bt.h"
#include "vesync_public.h"
#include "user_common.h"
#include "body_fat_calc.h"

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_task_wdt.h"
#include "freertos/timers.h"

static const char *TAG = "Vesync_UART";
static QueueHandle_t uart0_queue;

static TimerHandle_t uart_resend_timer;

static bool uart_init = false;
hw_info         info_str;
UARTSTRUCT      vesync_uart;
uint8_t rec_command;

RESEND_COMD_BIT resend_cmd_bit;

command_types command_type[15] ={
    {CMD_HW_VN	            ,true  ,NULL},  //查询硬件版本
    {CMD_ID	                ,true  ,NULL},  //查询产品ID     
    {CMD_BODY_WEIGHT	    ,true  ,NULL}, 	//查询用户体重        
    {CMD_HADRWARE_ERROR	    ,true  ,NULL},  //硬件出错通知
    {CMD_BODY_FAT	        ,false ,NULL},  //计算后的体脂参数      
    {CMD_POWER_BATTERY	    ,true  ,NULL},  //硬件状态
    {CMD_MEASURE_UNIT	    ,false ,NULL},  //切换硬件计算单位
    {CMD_BT_STATUS	        ,false ,NULL},
    {CMD_WIFI_STATUS	    ,false ,NULL},
};

static bool uart_resend_timer_stop(void)
{
    bool status = false;

    if (xTimerStop(uart_resend_timer, portMAX_DELAY) != pdPASS) {
        status = false;
    } else {
        status = true;
    }

    return status;
}

static bool uart_resend_timer_start(void)
{
    bool status = false;

    uart_resend_timer_stop();
    if (xTimerStart(uart_resend_timer, portMAX_DELAY) != pdPASS) {
        status = false;
    } else {
        status =  true;
    }

    return status;
}

/**
 * @brief uart 重传任务处理
 * @param timer 
 */
static void vesynv_uart_resend_timerout_callback(TimerHandle_t timer){
    ESP_LOGI(TAG, "---------------------->uart resend timer stop");

    if(resend_cmd_bit & RESEND_CMD_MEASURE_UNIT_BIT){
        ESP_LOGI(TAG, "RESEND_CMD_MEASURE_UNIT_BIT");
        uart_encode_send(MASTER_SET,CMD_MEASURE_UNIT,(char *)&info_str.user_config_data.measu_unit,sizeof(uint8_t),true);
    }
    if(resend_cmd_bit & RESEND_CMD_BT_STATUS_BIT){
        ESP_LOGI(TAG, "RESEND_CMD_BT_STATUS_BIT");
        uint8_t bt_status = vesync_get_bt_status();
        uint8_t bt_conn =0 ;
        if(bt_status == BT_CONNTED){
            bt_conn = 2;
            uart_encode_send(MASTER_SET,CMD_BT_STATUS,(char *)&bt_conn,sizeof(uint8_t),true);
        }else if(bt_status == BT_DISCONNTED){
            uart_encode_send(MASTER_SET,CMD_BT_STATUS,(char *)&bt_conn,sizeof(uint8_t),true);
        }
    }
    if(resend_cmd_bit & RESEND_CMD_BODY_FAT_BIT){
        ESP_LOGI(TAG, "RESEND_CMD_BODY_FAT_BIT");
        uart_encode_send(MASTER_SET,CMD_BODY_FAT,(char *)&info_str.user_fat_data.fat,sizeof(info_str.user_fat_data)-28,true);
    }
    if(resend_cmd_bit & RESEND_CMD_WIFI_STATUS_BIT){
        ESP_LOGI(TAG, "RESEND_CMD_WIFI_STATUS_BIT");
        uint8_t wifi_status = vesync_wifi_get_status();
        uint8_t wifi_conn =0 ;
        if(wifi_status == STATION_GOT_IP){
            wifi_conn = 2;
            uart_encode_send(MASTER_SET,CMD_WIFI_STATUS,(char *)&wifi_conn,sizeof(uint8_t),true);
        }else if(wifi_status == STATION_CONNECTING){
            wifi_conn = 1;
            uart_encode_send(MASTER_SET,CMD_WIFI_STATUS,(char *)&wifi_conn,sizeof(uint8_t),true);
        }
        else if(wifi_status == STATION_DIS_CONNECTED){
            wifi_conn = 0;
            uart_encode_send(MASTER_SET,CMD_WIFI_STATUS,(char *)&wifi_conn,sizeof(uint8_t),true);
        }
    }
}

/**
 * @brief uart创建重传定时器
 * @return true 
 * @return false 
 */
static bool uart_resend_timer_init(void)
{
    uart_resend_timer = xTimerCreate("uart_resend_timer_timer", 500 / portTICK_PERIOD_MS, pdFALSE,
                                         NULL, vesynv_uart_resend_timerout_callback);
    if (uart_resend_timer == NULL){
        return false;
    }else{
        return true;
    }
}

static void decodecommand(hw_info *res ,const char *data,uint16_t len ,uint8_t channel,uni_frame_t *frame){
    for(int i=0;i<len;i++){
        if(Comm_frame_parse(data[i],channel,frame) == 1){            
             for(uint8_t j=0;j<sizeof(command_type)/sizeof(command_type[0]);j++){
                if(frame->frame_cmd == command_type[j].command){    //
                    frame_ctrl_t res_ctl ={     //应答包res状态  
                        .data =0,
                    };
                    static uint8_t *resp_cnt =NULL;
                    uint8_t *opt = NULL;
                    uint16_t bt_command = 0;
                    if(frame->frame_data_len >2){  //主机请求效应设备返回的数据或设备主动上传的数据
                        if(command_type[j].record){
                            opt = &frame->frame_data[1];    //过滤命令id字节
                            switch(command_type[j].command){
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
                                        command_type[j].transfer_callback = vesync_bt_notify;
                                        if(body_fat_person(res,&res->response_weight_data)){
                                            ESP_LOGI(TAG, "------>the same person! \r\n");
                                        }
                                        cnt++;
                                    }  
                                    break;
                                case CMD_HW_VN:{
                                        static uint8_t cnt =0;
                                        resp_cnt =&cnt;

                                        *(uint16_t *)&bt_command = CMD_REPORT_VESION;
                                        memcpy((uint8_t *)&res->response_version_data.hardware,opt,frame->frame_data_len-1);
#if 0                                        
                                        printf("\r\n hardware =0x%02x ,firmware =0x%02x ,protocol =0x%02x\r\n",res->response_version_data.hardware,\
                                                                                                res->response_version_data.firmware,\
                                                                                                res->response_version_data.protocol);
#endif                                                                                                
                                        res_ctl.data = 0;       //表示设备主动上传
                                        command_type[j].transfer_callback = vesync_bt_notify;
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
                                        command_type[j].transfer_callback = vesync_bt_notify;
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
                                        command_type[j].transfer_callback = vesync_bt_notify;

                                        opwer_status = npwer_status;
                                        npwer_status = res->response_hardstate.power;
                                        cnt++;
                                        ESP_LOGI(TAG, ",npwer_status [%d]  opwer_statusd[%d]\r\n",npwer_status,opwer_status);
                                        if((npwer_status == 0) && (opwer_status == 1)){         //关机
                                            //vesync_power_save_enter(WAKE_UP_PIN);
                                            vesync_bt_advertise_stop();
                                        }else if((npwer_status == 1) && (opwer_status == 0)){   //开机
                                            vesync_bt_advertise_start(ADVER_TIME_OUT);
                                            ESP_LOGI(TAG, "unit is %d\r\n" ,info_str.user_config_data.measu_unit);
                                            resend_cmd_bit |= RESEND_CMD_MEASURE_UNIT_BIT;
                                            uart_encode_send(MASTER_SET,CMD_MEASURE_UNIT,(char *)&info_str.user_config_data.measu_unit,sizeof(uint8_t),true);
                                        }
                                    }
                                    break;
                                case CMD_HADRWARE_ERROR:{
                                        static uint8_t cnt =0;
                                        resp_cnt =&cnt;

                                        *(uint16_t *)&bt_command = CMD_REPORT_ERRPR;
                                        res->response_error_notice.error.para = *(uint32_t *)&opt[0];
                                        //printf("\r\n error type =0x%04x\r\n",res->response_error_notice.error.para);
                                        command_type[j].transfer_callback = vesync_bt_notify;
                                        cnt++;
                                    }
                                    break;
                                default:
                                        ESP_LOGI(TAG, "other command %d\r\n" ,command_type[j].record);
                                    break;
                            }
                        }
                        if(command_type[j].transfer_callback != NULL){
                            vesync_bt_notify(res_ctl,resp_cnt,bt_command,(uint8_t *)opt ,frame->frame_data_len-1);  //透传控制码
                        }
                    }else{//设备返回的应答
                        ESP_LOGI(TAG, ",----------------ack %d\r\n",command_type[j].command);
                        static uint8_t *resp_cnt =NULL;
                        switch(command_type[j].command){
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
    }
}

static void uart_task_handler(void *pvParameters)
{
    uart_event_t event;
    while(1){
        if(xQueueReceive(uart0_queue, (void * )&event, (portTickType)portMAX_DELAY)) {
            switch(event.type) {
                case UART_DATA:
                	if(event.size){
                        char *temp = (char*) malloc(RD_BUF_SIZE);
                        int32_t ret_len = uart_read_bytes(EX_UART_NUM, (uint8_t *)temp, event.size, portMAX_DELAY);

                        if(ret_len == event.size && ret_len > 0){
                           decodecommand(&info_str,temp,event.size,0,&vesync_uart.rec_frame);
                        }
#if 0                        
                        uint8_t * ntf_value_p = NULL;
                        uint8_t current_num = 0;
                        uint8_t total_num = 0;

                        if(event.size <= (BLE_MAX_MTU - 3)){
                            Vesync_Bt_Notify((uint8_t)temp, event.size);
                        }else if(event.size > (BLE_MAX_MTU - 3)){  
                            if((event.size%(BLE_MAX_MTU - 7)) == 0){
                                total_num = event.size/(BLE_MAX_MTU - 7);
                            }else{
                                total_num = event.size/(BLE_MAX_MTU - 7) + 1;
                            }
                            current_num = 1;
                            ntf_value_p = (uint8_t *)malloc((BLE_MAX_MTU-3)*sizeof(uint8_t));
                            if(ntf_value_p == NULL){
                                //ESP_LOGE(GATTS_TABLE_TAG, "%s malloc.2 failed\n", __func__);
                                free(temp);
                                break;
                            }
                            while(current_num <= total_num){
                                if(current_num < total_num){
                                    ntf_value_p[0] = '#';
                                    ntf_value_p[1] = '#';
                                    ntf_value_p[2] = total_num;
                                    ntf_value_p[3] = current_num;
                                    memcpy(ntf_value_p + 4,temp + (current_num - 1)*(BLE_MAX_MTU-7),(BLE_MAX_MTU-7));
                                    Vesync_Bt_Notify(ntf_value_p, (BLE_MAX_MTU-3));
                                }else if(current_num == total_num){
                                    ntf_value_p[0] = '#';
                                    ntf_value_p[1] = '#';
                                    ntf_value_p[2] = total_num;
                                    ntf_value_p[3] = current_num;
                                    memcpy(ntf_value_p + 4,temp + (current_num - 1)*(BLE_MAX_MTU-7),(event.size - (current_num - 1)*(BLE_MAX_MTU - 7)));
                                    Vesync_Bt_Notify(ntf_value_p, (event.size - (current_num - 1)*(BLE_MAX_MTU - 7) + 4));
                                }
                                vTaskDelay(20 / portTICK_PERIOD_MS);
                                current_num++;
                            }
                            free(ntf_value_p);
                        }
#endif                  
                        free(temp);
                	}
                    break;
                case UART_FIFO_OVF:
                case UART_BUFFER_FULL:
                    uart_flush_input(EX_UART_NUM);
                    xQueueReset(uart0_queue);
                    break;
                default:
//                    ESP_LOGI(TAG, "uart event type: %d", event.type);
                    break;
            }
        }
    }
    vTaskDelete(NULL);
}

/**
 * @brief 
 * @param ctl 
 * @param cmd 
 * @param data 
 * @param len 
 * @param resend 为真开启定时器重传功能
 */
void uart_encode_send(uint8_t ctl,uint8_t cmd,const char *data,uint16_t len,bool resend)
{
    char sendbuf[200] ={0xA5};
	uint8_t sendlen =0;

    if(!uart_init)  return;
	if(len >sizeof(sendbuf)-len)	return;
    if(len ==0)	return;

    sendlen = Comm_frame_pack(ctl,cmd,data,len+1,&sendbuf); //数据内容包含1个字节cmd 所以加一

    if(sendlen != 0){
        WriteCoreQueue(sendbuf,sendlen);
    }
    //esp_log_buffer_hex(TAG, sendbuf, sendlen);

    if(resend){
       uart_resend_timer_stop(); 
       uart_resend_timer_start(); 
    }
}

/**
 * @brief 关闭串口硬件
 */
void vesync_uart_deint(void)
{
    if(!uart_init)  return;

    uart_init = false;
    uart_driver_delete(EX_UART_NUM);
}
/**
 * @brief  初始化串口硬件配置
 * @param  用户接口回调
 * @return 无
 */
void vesync_uart_int(uart_recv_cb_t cb)
{
    esp_log_level_set(TAG, ESP_LOG_INFO);
    if(uart_init)   return ;

    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(EX_UART_NUM, &uart_config);

    //Set UART log level
    esp_log_level_set(TAG, ESP_LOG_INFO);
    //Set UART pins (using UART0 default pins ie no changes.)
    uart_set_pin(EX_UART_NUM, UART_TX_PIN, UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    //Install UART driver, and get the queue.
    uart_driver_install(EX_UART_NUM, BUF_SIZE, BUF_SIZE, 20, &uart0_queue, 0);

    uart_enable_pattern_det_intr(EX_UART_NUM, '+', PATTERN_CHR_NUM, 10000, 10, 10);
    //Reset the pattern queue length to record at most 20 pattern positions.
    uart_pattern_queue_reset(EX_UART_NUM, 20);

       //Create a task to handler UART event from ISR
    xTaskCreate(uart_task_handler, "uart_task_handler_loop", 4096, NULL, 13, NULL);
    ESP_LOGI(TAG, "uart_event_int init success");

    if(uart_resend_timer_init() == false){
        ESP_LOGE(TAG, "uart_resend_timer_init fail");
    }
    uart_init = true;
    vesync_uart.m_uart_handler = cb;
}
