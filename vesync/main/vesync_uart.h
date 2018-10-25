
#ifndef _VESYNC_UART_H
#define _VESYNC_UART_H

#include "driver/uart.h"
#include "etekcity_comm.h"

#define WDT_UART_TIMEOUT_S		2
#define EX_UART_NUM UART_NUM_1			//打印输出使用 UART_NUM_0
#define PATTERN_CHR_NUM    (3)         /*!< Set the number of consecutive and identical characters received by receiver which defines a UART pattern*/

#define MAX_ITEM		0x8
#define BUF_SIZE (2048)
#define RD_BUF_SIZE (BUF_SIZE)

#define UART_TX_PIN		(16)
#define UART_RX_PIN		(17)

#define ACK_TimeOut		600


#define WriteCoreQueue(a ,b)	uart_write_bytes(EX_UART_NUM, a, b)
#define ReadCoreQueue(a)		uart_read_bytes(EX_UART_NUM, a, 1, portMAX_DELAY)

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
	uint32_t compar_len;	//单条长度;
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
	uint8_t  weight[2];	//体重;
	uint8_t  measu_unit;//测量单位
	uint8_t  user_mode;	//用户模式 1为普通人 0为运动员;
	uint8_t  unused[8];
}user_config_data_t;
#pragma pack()			//

//预留12个字节备用,凑齐4的整数倍字节,共12+28字节;
#pragma pack(1)
typedef struct{
	uint16_t fat;		//脂肪
	uint16_t muscle;	//肌肉
	uint16_t water;		//水分;
	uint16_t bone;		//骨重
	uint16_t bmr;		//基础代谢;
	uint16_t bmi;		//身体质量指数;
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

typedef struct {
	bool  		 			  devie_status;
	response_weight_data_t    response_weight_data;
	response_version_data_t   response_version_data;
	response_encodeing_data_t response_encodeing_data;
	response_hardstate_t 	  response_hardstate;
	update_reg_t	 		  updateReg;
	uint8_t 	 			  macaddr[2][6];	//蓝牙地址0 wifi地址1;
}hw_info;
extern hw_info info_str;

typedef struct{
    uart_config_t 							uart_config;			//uart句柄;
	uni_frame_t 							rec_frame;
	void  (*uart_recv_cb_t)(const void*, unsigned short);
}UARTSTRUCT;
extern UARTSTRUCT vesync_uart;



void vesync_uart_int(void);
extern void Vesync_Bt_Notify(uint8_t *notify_data ,uint16_t len);


#endif