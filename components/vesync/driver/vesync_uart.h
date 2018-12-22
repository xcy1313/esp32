/**
 * @file vesync_uart.h
 * @brief 
 * @author Jiven 
 * @date 2018-12-18
 */
#ifndef _VESYNC_UART_H
#define _VESYNC_UART_H

#include <stdio.h>
#include <string.h>

#include "driver/uart.h"
#include "vesync_unixtime.h"
#include "etekcity_comm.h"

#define VESYNC_SDK_VESION		"0.0.01"

#define WDT_UART_TIMEOUT_S		2
#define EX_UART_NUM UART_NUM_1			//打印输出使用 UART_NUM_0
#define PATTERN_CHR_NUM    (3)         /*!< Set the number of consecutive and identical characters received by receiver which defines a UART pattern*/

#define MAX_ITEM		0x8
#define BUF_SIZE 		(1024)
#define RD_BUF_SIZE 	(BUF_SIZE)

#define ACK_TimeOut		600

#define WriteCoreQueue(a ,b)	uart_write_bytes(EX_UART_NUM, a, b)
#define ReadCoreQueue(a)		uart_read_bytes(EX_UART_NUM, a, 1, portMAX_DELAY)

typedef void (*uart_recv_cb_t)(const unsigned char*, unsigned short);

typedef struct{
	uint32_t rx_pin;
	uint32_t tx_pin;
	uint32_t baud_rate;
	uart_recv_cb_t cb;
}vesync_uart_t;

void vesync_uart_deint(void);
/**
 * @brief 
 * @param rx_pin 设置串口rx
 * @param tx_pin 设置串口tx
 * @param baud_rate 设置串口波特率
 * @param cb 串口接收回调
 */
void vesync_uart_int(uint32_t rx_pin,uint32_t tx_pin,uint32_t baud_rate,uart_recv_cb_t cb);
/**
 * @brief 
 * @param ctl 
 * @param cmd 
 * @param data 
 * @param len 
 */
void vesync_uart_encode_send(uint8_t ctl,uint8_t cmd,const unsigned char *data,uint16_t len);

#endif
