/**
 * @file app_handle_scales.h
 * @brief 
 * @author Jiven 
 * @date 2018-12-18
 */
#ifndef _APP_HANLDE_SACLES_H
#define _APP_HANLDE_SACLES_H

#include "etekcity_bt_prase.h"
#include "etekcity_comm.h"
#include <stdint.h>

#define BUTTON_KEY  	19
#define WAKE_UP_PIN     25


#define UNIT_NAMESPACE "unit_space"	//体脂称测量单位保存记录键值
#define UNIT_KEY      "unit_key"

#define USER_MODEL_NAMESPACE "userconfig" //体脂称用户模型保存记录键值
#define USER_MODEL_KEY 		 "config"

#define USER_HISTORY_DATA_NAMESPACE  "userdata" //体脂称用户测量历史数据记录键值
#define USER_HISTORY_KEY 			 "history"

#define UART_TX_PIN		(16)
#define UART_RX_PIN		(17)
#define UART_BAUD_RATE  9600

#define MASTER_SET		0x10	//主设备对从设备进行参数或功能设置
#define MASTER_INQUIRY	0x20	//主设备对从设备状态查询操作
#define SLAVE_SET		0x30	//从设备对主设备进行需要的设置
#define SLAVE_INQUIRY	0x40	//从设备对主设备进行必要的查询操作
#define SLAVE_SEND		0x50	//从设备主动上数据给主设备

//串口数据重发功能标识
typedef enum{
	RESEND_CMD_HW_VN_BIT	 	 = 0x0001,
	RESEND_CMD_ID_BIT		 	 = 0x0002,
	RESEND_CMD_BODY_FAT_BIT 	 = 0x0004,
	RESEND_CMD_POWER_BATTERY_BIT = 0x0008,
	RESEND_CMD_BT_STATUS_BIT	 = 0x0010,
	RESEND_CMD_WIFI_STATUS_BIT	 = 0x0020,
	RESEND_CMD_MEASURE_UNIT_BIT	 = 0x0040,
	RESEND_CMD_FACTORY_START_BIT = 0x0080,
	RESEND_CMD_FACTORY_STOP_BIT	 = 0x0100,
	RESEND_CMD_FACTORY_CHARGE_BIT= 0x0200,
	RESEND_CMD_FACTORY_WEIGHT_BIT= 0x0400,
	RESEND_CMD_ALL_BIT			 = 0xffff
}RESEND_COMD_BIT;
extern RESEND_COMD_BIT resend_cmd_bit;

//升级状态标识
enum{
	CMD_BT_STATUS_DISCONNECT = 0,
	CMD_BT_STATUS_CONNTEING  = 1,
	CMD_BT_STATUS_CONNTED    = 2,
	CMD_BT_STATUS_UPGADE_ING = 3,
	CMD_BT_STATUS_UPGADE_DONE = 4,
};

//称体计量单位切换
enum{
    UNIT_KG,
    UNIT_LB,
    UNIT_ST,
}UNIT;

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

#define CMD_FACTORY_SYNC_START	130
#define CMD_FACTORY_CHARGING	131
#define CMD_FACTORY_WEIGHT		132
#define CMD_FACTORY_SYNC_STOP	133

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

#define MAX_CONUT		16

//20个字节，凑齐4的整数倍;
#pragma pack(1)
typedef struct{
	uint8_t  action;	//操作类型，3为同步最新的用户模型,2为修改旧账户模型信息，1为删除旧账户模型信息，0为创建新账户模型信息
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
}response_weight_data_t;
#pragma pack()			//
extern response_weight_data_t response_weight_data;

typedef struct{
	uint8_t	 power;			//开关机状态
	uint8_t  battery_level;	//电池电量
}response_hardstate_t;

typedef struct{
	union{
		uint32_t para;
		uint8_t  type[4];
	}error;
}response_error_notice_t;

//用户沉淀数据
#pragma pack(1)
typedef struct{
	uint32_t				account;		//账户号
	uint16_t 				imped_value;	//阻抗值;
	uint32_t  			 	utc_time;		//测量时间戳;
	int8_t					time_zone;		//测量时区;
	uint8_t 				measu_unit;		//测量单位
	uint16_t 				weight_kg; 		//体重kg值；
	uint16_t 				weight_lb; 		//体重st值；	  
}user_history_t;
#pragma pack()			//

typedef struct {
	response_weight_data_t    response_weight_data;			
	response_hardstate_t 	  response_hardstate;
	response_error_notice_t   response_error_notice;
	user_config_data_t        user_config_data;
	user_fat_data_t           user_fat_data;
	user_history_t			  *user_history_data;
}hw_info;
extern hw_info info_str;


/**
 * @brief 启动体脂称硬件接口功能
 */
void app_scales_start(void);
/**
 * @brief app层调用串口发送
 * @param ctl 
 * @param cmd 
 * @param data 
 * @param len 
 * @param resend 是否重传
 */
void app_uart_encode_send(uint8_t ctl,uint8_t cmd,const unsigned char *data,uint16_t len,bool resend);

#endif

