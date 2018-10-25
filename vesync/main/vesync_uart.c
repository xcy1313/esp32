#include "vesync_uart.h"
#include "vesync_bt.h"
#include "user_common.h"

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_task_wdt.h"

static const char *TAG = "Vesync_UART";
static QueueHandle_t uart0_queue;

hw_info         info_str;
UARTSTRUCT      vesync_uart;

typedef struct{
    uint8_t command;
    bool    record;
    void    (*transfer_callback)(const void*, unsigned short);
}command_types;
command_types command_type[] ={
    {CMD_HW_VN	        ,true  ,Vesync_Bt_Notify},  //硬件需要记录称体
    {CMD_ID	            ,true  ,Vesync_Bt_Notify},  //硬件需要记录称体      
    {CMD_BODY_WEIGHT	,true  ,Vesync_Bt_Notify}, 	//硬件需要记录称体        
    {CMD_HADRWARE_ERROR	,false ,Vesync_Bt_Notify},
    {CMD_BODY_FAT	    ,false ,Vesync_Bt_Notify},        
    {CMD_POWER_BATTERY	,true  ,Vesync_Bt_Notify},  //硬件需要记录称体
    {CMD_MEASURE_UNIT	,false ,Vesync_Bt_Notify},  //硬件需要记录称体
    {CMD_BACKLIGHT_TIME	,false ,Vesync_Bt_Notify}
};

static void decodecommand(hw_info *res ,const char *data,uint16_t len ,uint8_t channel,uni_frame_t *frame){
#if 1
	printf("\r\n");		//数据入库;
	printf("\r\n");		//数据入库;
	printf("receive come----->");		//数据入库;
	printf("\r\n");		//数据入库;
	for(int j=0;j<len;j++){
		printf("%02x ",data[j]);
	}
	printf("\r\n");
	printf("len =%d \r\n",len);
	printf("receive end----->\r\n");		//数据入库;
	printf("\r\n");
#endif    
    esp_task_wdt_reset();

    for(int i=0;i<len;i++){
        if(Comm_frame_parse(data[i],channel,frame) == 1){
            for(uint8_t i=0;i<frame->frame_data_len;i++){
                printf("%02x ",frame->frame_data[i]);
            }
            printf("\r\n ctl =%d ,len =%d ,cmd = %d\r\n ",frame->frame_ctrl,frame->frame_data_len,frame->frame_cmd);
            
            for(uint8_t j=0;j<sizeof(command_type)/sizeof(command_type[0]);j++){
                if(frame->frame_cmd == command_type[j].command){    //
                    if(frame->frame_data_len >1){  //主机请求效应设备返回的数据或设备主动上传的数据
                        ESP_LOGI(TAG, "----->\r\n");
                        if(command_type[j].record){
                            uint8_t *opt = NULL;
                            ESP_LOGI(TAG, "record \r\n");
                            opt = &frame->frame_data[1];    //过滤命令id字节
                            switch(command_type[j].command){
                                case CMD_BODY_WEIGHT:
                                        memcpy((uint8_t *)&res->response_weight_data.weight,opt,frame->frame_data_len-1);
                                        printf("\r\n weight =0x%04x ,if =0x%x ,unit =0x%x,imped =0x%02x\r\n",res->response_weight_data.weight,\
                                                                                                 res->response_weight_data.if_stabil,\
                                                                                                 res->response_weight_data.measu_unit,\
                                                                                                 res->response_weight_data.imped_value);
                                    break;
                                case CMD_HW_VN:
                                        memcpy((uint8_t *)&res->response_version_data.hardware,opt,frame->frame_data_len-1);
                                        printf("\r\n hardware =0x%02x ,firmware =0x%02x ,protocol =0x%02x\r\n",res->response_version_data.hardware,\
                                                                                                res->response_version_data.firmware,\
                                                                                                res->response_version_data.protocol);
                                                                                               
                                    break;
                                case CMD_ID:
                                        memcpy((uint8_t *)&res->response_encodeing_data.type,opt,frame->frame_data_len-1);
                                        printf("\r\n type =0x%02x ,item =0x%02x\r\n",res->response_encodeing_data.type,res->response_encodeing_data.item);
                                    break;
                                case CMD_POWER_BATTERY:
                                        memcpy((uint8_t *)&res->response_hardstate.battery_level,opt,frame->frame_data_len-1);
                                        printf("\r\n type =0x%02x ,item =0x%02x\r\n",res->response_hardstate.battery_level,res->response_hardstate.power);
                                    break;
                                default:
                                    break;
                            }
                        }
                        command_type[j].transfer_callback((uint8_t *)&frame->frame_data[0] ,frame->frame_data_len);  //透传控制码
                    }else if(frame->frame_data_len == 1){//设备返回的应答
                        ESP_LOGI(TAG, "----ack \r\n");
                        command_type[j].transfer_callback(&frame->frame_cmd ,1);  //应答
                    }
                    break;
                }   
            }
        }
    }
}

static void uart_task_handler(void *pvParameters){
    uart_event_t event;
    while(1){
        if(xQueueReceive(uart0_queue, (void * )&event, (portTickType)portMAX_DELAY)) {
            switch(event.type) {
                case UART_DATA:
                	if(event.size){
                        char *temp = (char*) malloc(RD_BUF_SIZE);

                        ESP_LOGI(TAG, "[UART DATA]: %d", event.size);
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
 * @brief  初始化串口硬件配置
 * @return 无
 */
void vesync_uart_int(void)
{
    esp_log_level_set(TAG, ESP_LOG_INFO);

    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
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
    uart_driver_install(EX_UART_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 20, &uart0_queue, 0);

    uart_enable_pattern_det_intr(EX_UART_NUM, '+', PATTERN_CHR_NUM, 10000, 10, 10);
    //Reset the pattern queue length to record at most 20 pattern positions.
    uart_pattern_queue_reset(EX_UART_NUM, 20);

       //Create a task to handler UART event from ISR
    xTaskCreate(uart_task_handler, "uart_task_handler_loop", 4096, NULL, 13, NULL);
    ESP_LOGI(TAG, "uart_event_int init success");
}