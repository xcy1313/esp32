
#ifndef _VESYNC_UART_H
#define _VESYNC_UART_H

#include "driver/uart.h"
#include "etekcity_comm.h"
#include "vesync_unixtime.h"

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

#define MASTER_SET		0x10	//主设备对从设备进行参数或功能设置
#define MASTER_INQUIRY	0x20	//主设备对从设备状态查询操作
#define SLAVE_SET		0x30	//从设备对主设备进行需要的设置
#define SLAVE_INQUIRY	0x40	//从设备对主设备进行必要的查询操作
#define SLAVE_SEND		0x50	//从设备主动上数据给主设备

typedef enum{
	RESEND_CMD_HW_VN_BIT	 	 = 0x0001,
	RESEND_CMD_ID_BIT		 	 = 0x0002,
	RESEND_CMD_BODY_FAT_BIT 	 = 0x0004,
	RESEND_CMD_POWER_BATTERY_BIT = 0x0008,
	RESEND_CMD_BT_STATUS_BIT	 = 0x0010,
	RESEND_CMD_WIFI_STATUS_BIT	 = 0x0020,
	RESEND_CMD_MEASURE_UNIT_BIT	 = 0x0040
}RESEND_COMD_BIT;
extern RESEND_COMD_BIT resend_cmd_bit;

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
//APP相关命令 用户接口操作

//REPORT为主动上报 RESP为设备返回
#define PROJECT_FOR_SCALES_BASE_ADDR	0x0000
#define CMD_REPORT_VESION       (PROJECT_FOR_SCALES_BASE_ADDR+0x0001)	
#define CMD_RESP_VERSION        (PROJECT_FOR_SCALES_BASE_ADDR+0x0002)
#define CMD_REPORT_CODING       (PROJECT_FOR_SCALES_BASE_ADDR+0x0003)
#define CMD_RESP_CODING         (PROJECT_FOR_SCALES_BASE_ADDR+0x0004)
#define CMD_REPORT_WEIGHT       (PROJECT_FOR_SCALES_BASE_ADDR+0x0005)
#define CMD_REPORT_ERRPR        (PROJECT_FOR_SCALES_BASE_ADDR+0x0006)
#define CMD_SET_BODY_FAT        (PROJECT_FOR_SCALES_BASE_ADDR+0x0008)
#define CMD_REPORT_POWER        (PROJECT_FOR_SCALES_BASE_ADDR+0x0009)
#define CMD_RESP_POWER          (PROJECT_FOR_SCALES_BASE_ADDR+0x000A)
#define CMD_SET_BT_STATUS       (PROJECT_FOR_SCALES_BASE_ADDR+0x000B)
#define CMD_SET_WIFI_STATUS     (PROJECT_FOR_SCALES_BASE_ADDR+0x000C)
#define CMD_SET_HW_UNIT    	    (PROJECT_FOR_SCALES_BASE_ADDR+0x000D)
#define CMD_SET_WEIGHT_UNIT		(PROJECT_FOR_SCALES_BASE_ADDR+0x001F)
#define CMD_SYNC_TIME			(PROJECT_FOR_SCALES_BASE_ADDR+0x0020)
#define CMD_CONFIG_ACCOUNT		(PROJECT_FOR_SCALES_BASE_ADDR+0x0021)
#define CMD_DELETE_ACCOUNT		(PROJECT_FOR_SCALES_BASE_ADDR+0x0022)
#define CMD_INQUIRY_HISTORY		(PROJECT_FOR_SCALES_BASE_ADDR+0x0023)
#define CMD_SET_FAT_CONFIG		(PROJECT_FOR_SCALES_BASE_ADDR+0x0024)
#define CMD_UPGRADE				(PROJECT_FOR_SCALES_BASE_ADDR+0x0025)


typedef enum {
    RET_MSG_ID_NONE = 0,
    RET_MSG_ID_OK = 1,
    RET_MSG_ID_IDLE,
    RET_MSG_ID_ERROR,
    RET_MSG_ID_TIMEOUT,
	RET_MSG_ACK_ERROR,
	RET_MSG_ACK_OK,
	RET_MSG_DATA_OVER,
	RET_MSG_DATA_NULL,
	RET_MSG_DATA_ERROR,
    RET_MSG_ID_BUSY
} RET_MSG_ID_E;

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

#define MAX_CONUT		16

//20个字节，凑齐4的整数倍;
#pragma pack(1)
typedef struct{
	uint8_t  action;	//操作类型，2为修改旧账户模型信息，1为删除旧账户模型信息，0为创建新账户模型信息
	uint32_t account;	//账户
	uint8_t  gender;	//性别;
	uint8_t  height_unit;//身高单位 0表示ft ,1表示cm
	uint8_t  height;	//身高
	uint8_t  age;		//年龄;
	uint8_t  measu_unit;//测量单位
	uint16_t weight_kg; //体重kg值；
	uint16_t weight_st; //体重st值；
	uint16_t imped_value; //阻抗值
	uint8_t  ueser_id;	//用户id
	uint8_t  user_mode; //用户模式;
	uint8_t  crc8;		//crc8;
	uint8_t  length;	//长度;
}user_config_data_t;
#pragma pack()                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       //

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
	uint16_t weight;	//体重 体重kg值(2字节)+体重lb值（2字节）;
	uint16_t lb;		//体重 体重kg值(2字节)+体重lb值（2字节）;
	uint8_t	 if_stabil;	//是否稳定
	uint8_t  measu_unit;//测量单位
	uint16_t imped_value;//阻抗值;
	uint8_t  unused[12];
}response_weight_data_t;
#pragma pack()			//
extern response_weight_data_t response_weight_data;

typedef struct{
	uint8_t	 power;			//开关机状态
	uint8_t  battery_level;	//电池电量
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

//用户沉淀数据
typedef struct{
	uint8_t 				  number;			//账户模型信息数组编号
	uint32_t				  account;			//账户号
	uint16_t 				  imped_value;		//阻抗值;
	mytime_struct			  utc_time;			//测量时间戳
	uint8_t 				  measu_unit;		//测量单位
	uint16_t 				  weight_kg; 		//体重kg值；
	uint16_t 				  weight_st; 		//体重st值；	  
}user_account_t;

typedef struct {
	response_weight_data_t    response_weight_data;			
	response_version_data_t   response_version_data;
	response_encodeing_data_t response_encodeing_data;
	response_hardstate_t 	  response_hardstate;
	response_error_notice_t   response_error_notice;
	user_config_data_t        user_config_data;
	user_fat_data_t           user_fat_data;
	user_account_t			  *user_history;
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
    void    (*transfer_callback)(uint8_t,uint8_t,const void*, unsigned short,bool);
}command_types;
extern command_types command_type[15];



void vesync_uart_deint(void);
void vesync_uart_int(uart_recv_cb_t cb);
void uart_encode_send(uint8_t ctl,uint8_t cmd,const char *data,uint16_t len,bool resend);

#endif
