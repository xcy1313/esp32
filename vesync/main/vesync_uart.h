
#ifndef _VESYNC_UART_H
#define _VESYNC_UART_H

#include "driver/uart.h"
#include "etekcity_comm.h"

#define VESYNC_SDK_VESION		"0.0.01"

#define WDT_UART_TIMEOUT_S		2
#define EX_UART_NUM UART_NUM_1			//打印输出使用 UART_NUM_0
#define PATTERN_CHR_NUM    (3)         /*!< Set the number of consecutive and identical characters received by receiver which defines a UART pattern*/

#define MAX_ITEM		0x8
#define BUF_SIZE (1024)
#define RD_BUF_SIZE (BUF_SIZE)

#define UART_TX_PIN		(16)
#define UART_RX_PIN		(17)

#define ACK_TimeOut		600


#define WriteCoreQueue(a ,b)	uart_write_bytes(EX_UART_NUM, a, b)
#define ReadCoreQueue(a)		uart_read_bytes(EX_UART_NUM, a, 1, portMAX_DELAY)

#define MASTER_SET		0x10
#define MASTER_INQUIRY	0x20
#define SLAVE_SET		0x30
#define SLAVE_INQUIRY	0x40
#define SLAVE_SEND		0x50


#define CMD_HW_VN	            1
#define CMD_ID	                7
#define CMD_UNUSED1	            9
#define CMD_BODY_WEIGHT	        30
#define CMD_UNUSED2	            38
#define CMD_HADRWARE_ERROR	    60
#define CMD_BODY_FAT	        64
#define CMD_UNUSED3	            76
#define CMD_POWER_BATTERY	    123
#define CMD_BT_STATUS	        125
#define CMD_WIFI_STATUS	        126
#define CMD_MEASURE_UNIT	    127
#define CMD_BACKLIGHT_TIME	    128

#define CMD_HISTORY_TOTALLEN	200		
#define CMD_HISTORY_WEIGHT		201		
#define CMD_HISTORY_FAT			202		
#define CMD_USER_AMOUT			203		
#define CMD_CREATE_USER			204		
#define CMD_DELETE_USER         205		
#define CMD_DELETE_HIS_ITEM     206		
#define CMD_SYNC_UTC			207		
#define CMD_MODIFY_USER         208	

#define FRAME_HEAD          0xA5    //帧起始符
#define FRAME_END           0x5A    //帧结束符

typedef enum{
	SYNC_HEAD,
	SYNC_DATA,
	SYNC_TAIL,
	SYNC_WAIT
}packet_frame;

typedef struct{
	uint32_t total_len;
	uint32_t per_len;	//单条长度;
	uint32_t start_time;//单条时间;
	uint32_t flash_addr;
	uint32_t compar_len;//单条长度;
	uint32_t rest_len;	//单条长度;
	uint32_t fs_len;	//sd文件长度;
	uint8_t  total_item;
}update_reg_t;

//预留9个字节备用,凑齐4的整数倍字节,共11+9字节;
#pragma pack(1)
typedef struct{
	uint32_t account;	//账户
	uint8_t  ueser_id;	//用户id
	uint8_t  gender;	//性别;
	uint8_t  height;	//身高
	uint8_t  age;		//年龄;
	uint16_t weight;	//体重;
	uint8_t  measu_unit;//测量单位
	uint8_t  user_mode;	//用户模式 1为普通人 0为运动员;
	uint8_t  unused[6];
	uint8_t  length;	//长度
	uint8_t  crc8;		//crc
}user_config_data_t;
#pragma pack()			//

//预留12个字节备用,凑齐4的整数倍字节,共12+28字节;
#pragma pack(1)
typedef struct{
	int16_t fat;		//脂肪
	int16_t muscle;		//肌肉
	int16_t water;		//水分;
	int16_t bone;		//骨重
	int16_t bmr;		//基础代谢;
	int16_t bmi;		//身体质量指数;
	uint8_t  unused[28];//预留12
}user_fat_data_t;		//用户体脂数据
#pragma pack()			//

//预留12个字节备用,凑齐4的整数倍字节,共8+12字节;
#pragma pack(1)
typedef struct{
	uint32_t weight;	//体重 体重kg值(2字节)+体重lb值（2字节）;
	uint8_t	 if_stabil;	//是否稳定
	uint8_t  measu_unit;//测量单位
	uint16_t imped_value;//阻抗值;
	uint8_t  unused[12];
}response_weight_data_t;
#pragma pack()			//
extern response_weight_data_t response_weight_data;

typedef struct{
	uint8_t  battery_level;	//电池电量
	uint8_t	 power;			//是否稳定
}response_hardstate_t;

//预留12个字节备用,凑齐4的整数倍字节,共8+12字节;
#pragma pack(1)
typedef struct{
	uint16_t hardware;	//硬件
	uint16_t firmware;	//固件
	uint16_t protocol;	//协议
}response_version_data_t;
#pragma pack()			//

#pragma pack(1)
typedef struct{
	uint8_t  type;	//类型
	uint8_t  item;	//编码
}response_encodeing_data_t;
#pragma pack()			//

typedef struct{
	union{
		uint32_t para;
		uint8_t  type[4];
	}error;
}response_error_notice_t;

typedef struct{
	user_config_data_t        config_data;
	user_fat_data_t           fat_data;
	//response_weight_data_t    response_weight_data;
}user_item_t;

typedef struct{
	//uint8_t 				  amount；
	user_item_t 			  user_item[8];		//最多创建8个账户
}user_info_t;

typedef struct {
	response_weight_data_t    response_weight_data;
	response_version_data_t   response_version_data;
	response_encodeing_data_t response_encodeing_data;
	response_hardstate_t 	  response_hardstate;
	response_error_notice_t   response_error_notice;
	user_config_data_t        user_config_data;
	user_fat_data_t           user_fat_data;
	user_info_t				  user_info;
	update_reg_t	 		  updateReg;
	uint8_t 	 			  macaddr[2][6];	//蓝牙地址0 wifi地址1;
}hw_info;
extern hw_info info_str;

typedef void (*uart_recv_cb_t)(const void*, unsigned short);

typedef struct{
    uart_config_t 							uart_config;			//uart句柄;
	uni_frame_t 							rec_frame;
	uart_recv_cb_t							m_uart_handler;
}UARTSTRUCT;
extern UARTSTRUCT vesync_uart;

typedef struct{
    uint8_t command;
    bool    record;
    void    (*transfer_callback)(const void*, unsigned short);
}command_types;
extern command_types command_type[15];
void vesync_uart_int(uart_recv_cb_t cb);
void uart_encode_send(uint8_t ctl,uint8_t cmd,const char *data,uint16_t len);

#endif
