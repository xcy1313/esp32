/**
 * @file etekcity_bt_prase.c
 * @brief 蓝牙透传数据解析
 * @author Jiven 
 * @date 2018-11-20
 */
#include "etekcity_bt_prase.h"
#include "esp_log.h"

#define BT_TAG "BT_FRAME"

//使用默认的初始化值0，且必须为0
static unsigned char channel[8] = {0};

/**
 * @brief	计算和校验值
 * @param	frame 	[被校验数据帧结构]
 * @return	sum		[计算的校验和结果]
 */
static unsigned char sum_verify(bt_frame_t *frame)
{
	unsigned char sum = 0;
	unsigned char i = 0;

 	sum += BT_HEAD;
	sum += frame->frame_ctrl.data;
	sum += frame->frame_cnt;    
	sum += frame->frame_data_len;
	sum += frame->frame_cmd;

	for (i = 0; i < frame->frame_data_len-1; i++){	//去掉cmd
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
unsigned char bt_data_frame_decode(unsigned char byteData, unsigned char channel_sel, bt_frame_t *frame)
{
    unsigned char calc_sum = 0;
    unsigned char ret = 0;
	unsigned char *step = NULL;
	static unsigned char cnt =0;
	static unsigned char back_buf[2] = {0};

	if (NULL == frame){
		ESP_LOGE(BT_TAG, "BT Prase NULL!");
		return 0;
	}
	
	step = &channel[channel_sel];

	switch(*step){
		case BT_FRAME_HEAD:
			if( byteData == BT_HEAD ){
				*step = BT_FRAME_CTRL;
	        }
			break;

		case BT_FRAME_CTRL:
			frame->frame_ctrl.data = byteData;
			*step = BT_FRAME_SEQ;
			break;

		case BT_FRAME_SEQ:
			frame->frame_cnt = byteData;
			*step = BT_FRAME_LEN;
			break;

		case BT_FRAME_LEN:
			if(cnt == 0){
				back_buf[0] = byteData;		//低字节在前
				cnt++;
			}else{
				back_buf[1] = byteData;		//高字节在后
				*step = BT_FRAME_CMD;
				frame->frame_data_len = *(unsigned short *)&back_buf[0];
				cnt =0;
				//帧数据长度有效性判断
				if (BT_DATA_BUFF_MAX < frame->frame_data_len){
					*step = BT_FRAME_HEAD;
				}else{
					frame->frame_data_pos = 0;
					*step = BT_FRAME_CMD;
				}
			}
			
			break;

		case BT_FRAME_CMD:
			if(cnt == 0){
				back_buf[0] = byteData;		//低字节在前
				cnt++;
			}else{
				back_buf[1] = byteData;		//高字节在后
				*step = BT_FRAME_DATA;
				frame->frame_cmd = *(unsigned short *)&back_buf[0];
				frame->frame_data_pos =0;	
				cnt =0;
			}			
			break;

		case BT_FRAME_DATA:
			frame->frame_data[frame->frame_data_pos++] = byteData;
			if(frame->frame_data_pos >= (frame->frame_data_len-sizeof(frame->frame_cmd))){
				frame->frame_data_pos = 0;
				*step = BT_FRAME_SUM;
			}	
			break;

		case BT_FRAME_SUM:
			frame->frame_sum = byteData;
			calc_sum = sum_verify(frame);	 //计算校验和
			frame->frame_sum = calc_sum;
    		if(frame->frame_sum != calc_sum){
				*step = BT_FRAME_HEAD;
    		}else{
				*step = BT_FRAME_END;				
			}
			break;

		case BT_FRAME_END:
		    *step = BT_FRAME_HEAD;
			if( byteData == BT_END ){
				ret = 1;				
			}
			break;

		default:
			*step = BT_FRAME_HEAD;
			break;
	}

	return ret;
}

/**
 * @brief 	协议帧封包
 * @param 	ctrl 			[控制码]
 * @param 	cmd 			[命令码]
 * @param 	data 			[命令参数数据指针]
 * @param	datalen 		[参数数据长度]
 * @param	outbuf 			[封装好的协议帧缓存区指针]
 * @return	unsigned char	[协议封包结果，0 - 封包失败；大于0 - 封装好的协议帧缓存区数据长度]
 */
unsigned short bt_data_frame_encode(frame_ctrl_t ctrl, 
								unsigned char *cnt,
								unsigned short cmd, 
								const unsigned char *data, 
								unsigned short datalen, 
								unsigned char *outbuf)
{
	unsigned char i;
	unsigned sum = 0;
	unsigned short sendlen =0;    //包头+命令+载荷长度+载荷+checksum+包尾；

	outbuf[0] = BT_HEAD; 											//帧头
	outbuf[1] = ctrl.data;                  						//控制码
	outbuf[2] = cnt[0];                  							//包计数
    *(unsigned short *)&outbuf[3] = datalen+sizeof(cmd);            //用户数据（命令码+参数）长度
    *(unsigned short *)&outbuf[5] = cmd;                   			//命令码
	
	sendlen += 7;													//包含包头|header|sequence|payload len|command|
	if(datalen > 0){
		memcpy(&outbuf[7], (char *)data, datalen);						    //只包含参数
		sendlen += datalen;
	}
	for(i=0;i<sendlen;i++){
		sum += outbuf[i];
	}

	sendlen +=2;
	
	outbuf[sendlen-2] = sum;										//校验和
    outbuf[sendlen-1] = BT_END;       								//帧尾

    return sendlen;													//返回有效发送数据总长
}
