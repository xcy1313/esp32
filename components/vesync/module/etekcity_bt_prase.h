/**
 * @file etekcity_bt_prase.h
 * @brief 
 * @author Jiven 
 * @date 2018-11-20
 */
#ifndef _ETEKCITY_BT_PRASE_H
#define _ETEKCITY_BT_PRASE_H

#include <string.h>
#include <stdio.h>

enum{
	NO_ACK =0,		//不需要应答
	NEED_ACK		//需要应答	
};

enum{
	PACKET_COMMAND =0,	//数据包是命令
	PACKET_RESP			//数据包是应答
};

//帧头帧尾定义
#define BT_HEAD          0xA5    //帧起始符
#define BT_END           0x5A    //帧结束符

//帧数据域缓存区最大长度定义，可设置的最大值为256
#define BT_DATA_BUFF_MAX       300

//ACK错误码
#define RESP_SUCCESS			  0
#define	RESP_INVALID_CTL_ERROR    1
#define	RESP_INVALID_LEN_ERROR	  2
#define	RESP_INVALID_CMD_ERROR    3
#define	RESP_INVALID_PDT_ERROR    4
#define	RESP_INVALID_CRC_ERROR    5
#define	RESP_INVALID_NULL_ERROR   6

//蓝牙数据解析状态
enum{
	BT_FRAME_HEAD = 0,
	BT_FRAME_CTRL,
	BT_FRAME_SEQ, 
	BT_FRAME_LEN,  
	BT_FRAME_CMD,  
	BT_FRAME_DATA, 
	BT_FRAME_SUM,  
	BT_FRAME_END, 
	BT_BUTT
};

typedef union{
	unsigned char  data;
	struct{
		unsigned char  version:     4;  	//协议版本号，当前版本为0；
		unsigned char  ack_flag:    1;  	//用于指示当前数据包是命令还是应答    0：命令    1: 应答；
		unsigned char  request_flag:1;  	//用于指示接收该数据包的设备是否需要回应答，0：不需要回应答  1：需要回应答；
		unsigned char  error_flag:  1;  	//应答包错误指示位
		unsigned char  resever:     1;  	//保留
	}bitN;
}frame_ctrl_t;

typedef struct{
	frame_ctrl_t frame_ctrl;
	unsigned char frame_cnt;
	unsigned short frame_data_len;
	unsigned short frame_cmd;
	unsigned char frame_sum;
	unsigned short frame_data_pos;
	unsigned char frame_data[BT_DATA_BUFF_MAX];
}bt_frame_t;

/**
 * @brief 	协议解析，协议数据按照顺序逐个字节传入，使用通道值区分解析不同来源的协议数据
 * @param	byteData 		[当前传入的字节数据]
 * @param	channel_sel  	[当前使用的解析通道]
 * @param	frame			[功能模块的结构体指针]
 * @return	unsigned char	[解析的结果，0 - 非完整指令帧；1 - 解读到完整指令帧]
 */
unsigned char bt_data_frame_decode(unsigned char byteData, unsigned char channel_sel, bt_frame_t *frame);

/**
 * @brief 	协议帧封包
 * @param 	ctrl 			[控制码]
 * @param 	cmd 			[命令码]
 * @param 	data 			[命令参数数据指针]
 * @param 	datalen 		[命令参数数据长度]
 * @param	outbuf 			[封装好的协议帧缓存区指针]
 * @return	unsigned char	[协议封包结果，0 - 封包失败；大于0 - 封装好的协议帧缓存区数据长度]
 */
unsigned short bt_data_frame_encode(frame_ctrl_t ctrl,unsigned char *cnt,unsigned short cmd,const unsigned char *data,unsigned short datalen,unsigned char *outbuf);

#endif
