/**
 * @file vesync_production.h
 * @brief 产测模式
 * @author WatWu
 * @date 2018-12-05
 */

#ifndef VESYNC_PRODUCATION_H
#define VESYNC_PRODUCATION_H
#include "cJSON.h"

#define PUBLISHSEND_TOPIC "$vesync/things/%s/shadow/update"				//发布主题
#define SUBSRECEIVE_TOPIC "$vesync/things/%s/shadow/update/delta"		//订阅主题

#define CID_LENGTH						32				//cid闀垮害
#define DEVCONF_HOLDER					0xA5A5			//璁惧閰嶇疆鍒濆鍖栦繚瀛樻爣蹇?
#define CID_HOLDER 						0xCCDD			//CID淇濆瓨鏍囧織
#define CONFIGKEY_BUF_LEN				16+4			//configKey缂撳瓨鍖哄ぇ灏?

/**
 * @brief 产测状态
 */
typedef enum
{
	RPODUCTION_START = 0,				//产测模式开始
	RPODUCTION_RUNNING = 1,				//产测模式运行中
	RPODUCTION_TEST_PASS = 2,			//产测模式测试通过
	PRODUCTION_TEST_FAIL = 3,			//产测模式测试失败
	PRODUCTION_EXIT = 4,				//产测模式退出
	PRODUCTION_LOGIN_FAIL = 5,			//产测模式固件登录产测服务器失败
} production_status_e;

/**
 * @brief 产测结果回调函数指针，该函数由应用层定义，在进入产测模式后的产测特定阶段会回调该函数
 * @param [产测结果]
 */
typedef void (*production_status_cb_t)(production_status_e status);

/**
 * @brief 获取产测状态
 * @return production_status_e [产测状态值]
 */
production_status_e vesync_get_production_status(void);

/**
 * @brief 设置产测状态，并同步进行回调
 * @param status [产测状态值]
 */
void vesync_set_production_status(production_status_e status);

/**
 * @brief 进入产测模式
 * @param cb   [产测状态回调函数]
 */
void vesync_enter_production_testmode(production_status_cb_t cb);

/**
 * @brief 订阅产测系统的mqtt主题
 * @return int 	[订阅结果]
 */
int vesync_subscribe_production_topic(void);

/**
 * @brief 产测系统连接成功后上报数据
 */
int vesync_production_connected_report_to_server(void);

/**
 * @brief 解析产测系统数据流
 */
void vesync_prase_production_json_packet(const char *topic,char *data);

#endif
