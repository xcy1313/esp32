/**
 * Copyright © Etekcity Technologies Co., Ltd. 2018-2018. All rights reserved.
 * @file etekcity_comm.h
 * @brief 设备间通讯协议头文件
 * @author Blue.Qi
 * @date 2018-05-15
 */
#ifndef ETEKCITY_COMM_H
#define ETEKCITY_COMM_H

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

//帧头帧尾定义
#define UART_HEAD          	0xA5    //帧起始符
#define UART_END           	0x5A    //帧结束符

//帧数据域缓存区最大长度定义，可设置的最大值为200
#define DATA_BUFF_MAX       200

//协议通道最大数量定义
#define CHANNEL_MAX         4

#define UNI_CH1				0
#define UNI_CH2				1
#define UNI_CH3				2
#define UNI_CH4				3

#define RESP_SUCCESS			  0
#define	RESP_INVALID_CTL_ERROR    1
#define	RESP_INVALID_LEN_ERROR	  2
#define	RESP_INVALID_CMD_ERROR    3
#define	RESP_INVALID_PDT_ERROR    4
#define	RESP_INVALID_CRC_ERROR    5

enum
{
	UART_FRAME_HEAD = 0,
	UART_FRAME_CTRL = 1,
	UART_FRAME_LEN  = 2,
	UART_FRAME_CMD  = 3,
	UART_FRAME_DATA = 4,
	UART_FRAME_SUM  = 5,
	UART_FRAME_END  = 6,
	UART_BUTT
}UART_FRAME;

typedef struct
{
	unsigned char frame_data_len;
	unsigned char frame_ctrl;
	unsigned char frame_cmd;
	unsigned char frame_sum;
	unsigned char frame_data_pos;
	unsigned char frame_data[DATA_BUFF_MAX];
}uni_frame_t;

/**
 * @brief 	协议解析，协议数据按照顺序逐个字节传入，使用通道值区分解析不同来源的协议数据
 * @param	byteData 		[当前传入的字节数据]
 * @param	channel_sel  	[当前使用的解析通道]
 * @param	frame			[功能模块的结构体指针]
 * @return	unsigned char	[解析的结果，0 - 非完整指令帧；1 - 解读到完整指令帧]
 */
unsigned char Comm_frame_parse(unsigned char byteData, unsigned char channel_sel, uni_frame_t *frame);

/**
 * @brief 	协议帧封包
 * @param 	ctrl 			[控制码]
 * @param 	cmd 			[命令码]
 * @param 	data 			[命令参数数据指针]
 * @param 	datalen 		[命令参数数据长度]
 * @param	outbuf 			[封装好的协议帧缓存区指针]
 * @return	unsigned char	[协议封包结果，0 - 封包失败；大于0 - 封装好的协议帧缓存区数据长度]
 */
unsigned char Comm_frame_pack(unsigned char ctrl,
							  unsigned char cmd,
							  const unsigned char *data,
							  unsigned char datalen,
							  unsigned char *outbuf);

#endif
