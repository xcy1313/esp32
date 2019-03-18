/**
 * @file vesync_uart.c
 * @brief 串口硬件驱动
 * @author Jiven 
 * @date 2018-12-18
 */
#include "vesync_uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

static const char *TAG = "Vesync_UART";
static QueueHandle_t uart_queue;

static bool uart_init = false;
static uart_recv_cb_t m_uart_handler;

static void vesync_uart_task_handler(void *pvParameters)
{
    uart_event_t event;
    while(1){
        if(xQueueReceive(uart_queue, (void * )&event, (portTickType)portMAX_DELAY)) {
            switch(event.type) {
                case UART_DATA:
                	if(event.size){
                        char *temp = (char*) malloc(RD_BUF_SIZE);
                        int32_t ret_len = uart_read_bytes(EX_UART_NUM, (uint8_t *)temp, event.size, portMAX_DELAY);

                        if(ret_len == event.size && ret_len > 0){
                            // esp_log_buffer_hex(TAG,(char *)temp,event.size);
                            m_uart_handler((unsigned char *)temp, event.size);
                        }
                        free(temp);
                	}
                    break;
                case UART_FIFO_OVF:
                case UART_BUFFER_FULL:
                    uart_flush_input(EX_UART_NUM);
                    xQueueReset(uart_queue);
                    break;
                default:
                    ESP_LOGI(TAG, "uart event type: %d", event.type);
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
void vesync_uart_encode_send(uint8_t ctl,uint8_t cmd,const unsigned char *data,uint16_t len)
{
    char sendbuf[200] ={0xA5};
	uint8_t sendlen =0;

    if(!uart_init)  return;
	if(len >sizeof(sendbuf)-len)	return;

    sendlen = Comm_frame_pack(ctl,cmd,data,len+1,(unsigned char *)&sendbuf); //数据内容包含1个字节cmd 所以加一

    esp_log_buffer_hex(TAG, sendbuf, sendlen);

    if(sendlen != 0){
        WriteCoreQueue(sendbuf,sendlen);
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
 * @brief 
 * @param rx_pin 设置串口rx
 * @param tx_pin 设置串口tx
 * @param baud_rate 设置串口波特率
 * @param cb 串口接收回调
 */
void vesync_uart_int(uint32_t rx_pin,uint32_t tx_pin,uint32_t baud_rate,uart_recv_cb_t cb)
{
    esp_log_level_set(TAG, ESP_LOG_INFO);
    if(uart_init)   return ;

    uart_config_t uart_config = {
        .baud_rate = baud_rate,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(EX_UART_NUM, &uart_config);

    esp_log_level_set(TAG, ESP_LOG_INFO);
    uart_set_pin(EX_UART_NUM, tx_pin, rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(EX_UART_NUM, BUF_SIZE, BUF_SIZE, 20, &uart_queue, 0);

    uart_enable_pattern_det_intr(EX_UART_NUM, '+', PATTERN_CHR_NUM, 10000, 10, 10);
    uart_pattern_queue_reset(EX_UART_NUM, 20);

    xTaskCreate(vesync_uart_task_handler, "uart_task_handler_loop", 4096, NULL, 13, NULL);
    ESP_LOGI(TAG, "uart_event_int init success");

    if(cb != NULL){
        m_uart_handler = cb;
    }
    uart_init = true;
}
