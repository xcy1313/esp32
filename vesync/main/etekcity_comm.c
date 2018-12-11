/**
 * Copyright © Etekcity Technologies Co., Ltd. 2018-2018. All rights reserved.
 * @file etekcity_comm.c
 * @brief 设备间通讯协议处理
 * @author Blue.Qi
 * @date 2018-05-15
 */

#include <string.h>
#include <stdio.h>
#include "etekcity_comm.h"

#define DEBUG

#ifdef DEBUG
#define DBG(format, arg...) printf(format, ##arg)
#else
#define DBG(format, arg...) do{}while(0)
#endif

//使用默认的初始化值0，且必须为0
static unsigned char channel[CHANNEL_MAX] = {UART_FRAME_HEAD};

/**
 * @brief	计算和校验值
 * @param	frame 	[被校验数据帧结构]
 * @return	sum		[计算的校验和结果]
 */
static unsigned char sum_verify(uni_frame_t *frame)
{
	unsigned char sum = 0;
	unsigned char i = 0;

 	sum += UART_HEAD;   	
	sum += frame->frame_ctrl;
	sum += frame->frame_data_len;
	sum += frame->frame_cmd;

	for (i = 0; i < frame->frame_data_len-1; i++)
	{
		sum += frame->frame_data[i];
	}

	return sum;
}

/**
 * @brief 	协议解析，协议数据按照顺序逐个字节传入，使用通道值区分解析不同来源的协议数据
 * @param 	byteData 		[当前传入的字节数据]
 * @param 	channel_sel  	[当前使用的解析通道]
 * @param 	frame			[功能模块的结构体指针]
 * @return	unsigned char	[解析的结果，0 - 非完整指令帧；1 - 解读到完整指令帧]
 */
unsigned char Comm_frame_parse(unsigned char byteData, unsigned char channel_sel, uni_frame_t *frame)
{
    unsigned char calc_sum = 0;
    unsigned char ret = 0;
	unsigned char *step = NULL;

	if (NULL == frame)
	{
		DBG("[Comm_frame_parse]Err: frame pointer is NULL.\r\n");
		return 0;
	}

	if (CHANNEL_MAX <= channel_sel)
	{
		DBG("[Comm_frame_parse]Err: channel_sel is greater than CHANNEL_MAX.\r\n");
		return 0;		
	}
	
	step = &(channel[channel_sel]);

	switch(*step)
	{
		case UART_FRAME_HEAD:
			if( byteData == UART_HEAD )
			{
				*step = UART_FRAME_CTRL;
	        }
			break;

		case UART_FRAME_CTRL:
			frame->frame_ctrl = byteData;
			*step = UART_FRAME_LEN;
			break;

		case UART_FRAME_LEN:
			frame->frame_data_len = byteData;
			//帧数据长度有效性判断
    		if (DATA_BUFF_MAX < frame->frame_data_len)
			{
				DBG("[Comm_frame_parse]Err: frame length is greater than DATA_BUFF_MAX.\r\n");	
				*step = UART_FRAME_HEAD;
			}
			else
			{
				frame->frame_data_pos = 0;
				*step = UART_FRAME_CMD;
			}
			break;

		case UART_FRAME_CMD:
			frame->frame_cmd = byteData;
			if(frame->frame_data_len == 1)			//只有命名码，不带命令参数
			{
				*step = UART_FRAME_SUM;
			}
			else
			{
				*step = UART_FRAME_DATA;
				frame->frame_data[frame->frame_data_pos] = byteData;//包含命令一起打包；
				frame->frame_data_pos++;
			}				
			break;

		case UART_FRAME_DATA:
			frame->frame_data[frame->frame_data_pos++] = byteData;
			if(frame->frame_data_pos >= frame->frame_data_len)
			{
				*step = UART_FRAME_SUM;
			}	
			break;
		case UART_FRAME_SUM:
			frame->frame_sum = byteData;
			calc_sum = sum_verify(frame);	 //计算校验和
			frame->frame_sum = calc_sum;
    		if(frame->frame_sum != calc_sum)
    		{
        		DBG("[Comm_frame_parse]Err: frame calcute sum error.\r\n");
				*step = UART_FRAME_HEAD;
    		}
			else
			{
				*step = UART_FRAME_END;				
			}
			break;

		case UART_FRAME_END:
		    *step = UART_FRAME_HEAD;
			if( byteData == UART_END )
			{
				ret = 1;				
			}
			break;

		default:
			*step = UART_FRAME_HEAD;
			break;
	}

	return ret;
}

/**
 * @brief 	协议帧封包
 * @param 	ctrl 			[控制码]
 * @param 	cmd 			[命令码]
 * @param 	data 			[命令参数数据指针]
 * @param	datalen 		[命令参数数据长度]
 * @param	outbuf 			[封装好的协议帧缓存区指针]
 * @return	unsigned char	[协议封包结果，0 - 封包失败；大于0 - 封装好的协议帧缓存区数据长度]
 */
unsigned char Comm_frame_pack(	unsigned char ctrl, 
								unsigned char cmd, 
								unsigned char *data, 
								unsigned char datalen, 
								unsigned char *outbuf)
{
	unsigned char i;
	unsigned sum = 0;

	if ((NULL == outbuf) || (NULL == data))
	{
		DBG("[Comm_frame_pack]Err: data or outbuf pointer is NULL.\r\n");
		return 0;
	}

    if (DATA_BUFF_MAX < datalen) 
	{
		DBG("[Comm_frame_pack]Err: frame length is greater than DATA_BUFF_MAX.\r\n");		
		return 0;
	}

    outbuf[0] = UART_HEAD; 					//帧头
    outbuf[1] = ctrl;                  			//控制码
    outbuf[2] = datalen;             			//用户数据（命令码+参数）长度
    outbuf[3] = cmd;                   			//命令码
 	memcpy(&(outbuf[4]), data, datalen);		//命令参数

	for(i=0;i<datalen+3;i++)
	{
		sum += outbuf[i];
	}
	outbuf[datalen+3] = sum;					//校验和

    outbuf[datalen+4] = UART_END;       		//帧尾

    return (datalen + 5);						//返回数据总长
}
