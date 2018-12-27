/**
 * @file vesync_production.c
 * @brief 产测模式
 * @author WatWu
 * @date 2018-05-28
 */

#include <stdlib.h>

#include "vesync_log.h"
#include "vesync_wifi.h"
#include "vesync_bt_hal.h"
#include "vesync_net_service.h"
#include "vesync_production.h"
#include "vesync_flash.h"

#include "vesync_build_cfg.h"

//上电更新信息
#define PUBLISHDATAINFO "{\"state\":{\"reported\":{\"delDevice\":null,\"restore\":null,\"mac\":\"%s\",\"connectionStatus\":\"online\"},\"desired\":null}}"

static const char* TAG = "vesync_production";

static production_status_e production_status = PRODUCTION_EXIT;
static production_status_cb_t production_status_cb = NULL;

static void  vesync_resp_json_to_server(cJSON* json);

/**
 * @brief 获取产测状态
 * @return production_status_e [产测状态值]
 */
production_status_e vesync_get_production_status(void)
{
	return production_status;
}

/**
 * @brief 设置产测状态，并同步进行回调
 * @param status [产测状态值]
 */
void vesync_set_production_status(production_status_e status)
{
	production_status = status;
	if(NULL != production_status_cb)
	{
		production_status_cb(status);
	}
}

/**
 * @brief 进入产测模式
 * @param cb   [产测状态回调函数]
 */
void vesync_enter_production_testmode(production_status_cb_t cb)
{
	production_status_cb = cb;

	vesync_client_connect_wifi(PRODUCTION_WIFI_SSID, PRODUCTION_WIFI_KEY);	// wifi driver初始化，否则无法获取mac地址
	vesync_set_production_status(RPODUCTION_START);

	LOG_I(TAG, "Production param :");
	LOG_I(TAG, "==============================");
	//产测时使用MAC地址代替CID用于产测通过前的mqtt连接
	char cid_mac[6 * 3];
	char username[64];
	vesync_get_wifi_sta_mac_string(cid_mac);
	sprintf(username, "%s%s", DEV_AUTHKEY, DEV_PID);
	LOG_I(TAG, "cid : %s", cid_mac);
	LOG_I(TAG, "username : %s", username);
	LOG_I(TAG, "server : %s:1883", PRODUCTION_SERVER_ADDR);
	LOG_I(TAG, "ssid : %s", PRODUCTION_WIFI_SSID);
	LOG_I(TAG, "pwd : %s", PRODUCTION_WIFI_KEY);
	LOG_I(TAG, "==============================");

	//vesync_config_cloud_mqtt_client(cid_mac, TEST_MQTT_SERVER_ADDR, 61613, "etekcity", "hardware");
	vesync_config_cloud_mqtt_client_with_tls(cid_mac, PRODUCTION_SERVER_ADDR, 1883, username, "0");
	vesync_mqtt_client_connect_to_cloud();
}

/**
 * @brief 订阅相应的主题
 * @param char 
 * @param qos 
 * @return int 
 */
char sub_Topic[128] ={0};				
int vesync_subscribe_topic(char *data, int qos)
{
	int ret;

	sprintf(sub_Topic, SUBSRECEIVE_TOPIC, data);

	ret = vesync_mqtt_subscribe_topic(vesync_get_cloud_mqtt_client(), sub_Topic, qos);
	return ret;
}
/**
 * @brief 发布主题
 * @param json 
 * @param qos 
 * @param retain 
 * @return int 
 */
int vesync_publish_topic(cJSON* json, int qos, int retain)
{
	int ret=0;
	char publicTopic[128] ={0};				

	char mac_string[6 * 3] ={0};
	vesync_get_wifi_sta_mac_string(mac_string);
	
	sprintf(publicTopic, PUBLISHSEND_TOPIC, mac_string);

	char *out = cJSON_PrintUnformatted(json);			
	//char publishdatainfo[256];
	//sprintf(publishdatainfo, out, mac_string);
	
	//ret = vesync_mqtt_publish_rawdata_to_topic(vesync_get_cloud_mqtt_client(), publicTopic, publishdatainfo, strlen(publishdatainfo), qos, retain);
	LOG_I(TAG, "Mqtt send : \n%s\n", out);

	free(out);										
	return ret;
}

/**
 * @brief 订阅产测系统的mqtt主题
 * @return int 	[订阅结果]
 */
int vesync_subscribe_production_topic(void)
{
	char mac_string[6 * 3] ={0};
	vesync_get_wifi_sta_mac_string(mac_string);

	return vesync_subscribe_topic(mac_string,MQTT_QOS1);
}

/**
 * @brief 产测系统连接成功后上报数据
 */
int vesync_production_connected_report_to_server(void)
{
	int ret;

	//上电上报信息中携带设备mac地址
	char mac_string[6 * 3];
	vesync_get_wifi_sta_mac_string(mac_string);

	char production_topic[100];
	sprintf(production_topic, "$vesync/things/%s/shadow/update", mac_string);
	char publishdatainfo[256];
	sprintf(publishdatainfo, PUBLISHDATAINFO, mac_string);

	//发送设备状态
	ret = vesync_mqtt_publish_rawdata_to_topic(vesync_get_cloud_mqtt_client(), production_topic, publishdatainfo, strlen(publishdatainfo), MQTT_QOS1, 1);

	return ret;

}

static void vesync_printf_cjson(cJSON *json)
{
	char *out = cJSON_Print(json);			//json鏍囧噯鏍煎紡锛屽甫缂╄繘
	LOG_I(TAG, "\n%s\n", out);
	free(out);
}

/**
 * @brief 上传产测信息至产测系统
 * @param json 
 */
static void  vesync_resp_json_to_server(cJSON* json)
{
	cJSON* root = NULL;
	cJSON* state = NULL;

	root = cJSON_CreateObject();

	if(root == NULL){
		LOG_I(TAG, "Create cJSON objects error !\r\n");
		return;
	}

	//娣诲姞root鎴愬憳
	cJSON_AddItemToObject(root, "state", state = cJSON_CreateObject());

	if(state != NULL){
		cJSON* reported = NULL;

		//娣诲姞state鐩綍鎴愬憳
		cJSON_AddItemToObject(state, "reported", reported = cJSON_CreateObject());
		cJSON_AddNullToObject(state, "desired");	//desired鎴愬憳璧嬪€间负null

		//娣诲姞reported鐩綍鎴愬憳
		if(reported != NULL){
			cJSON_AddItemReferenceToObject(reported, json->string, json);
		}

		LOG_I(TAG, "Reporte to server : \r\n");
		vesync_printf_cjson(root);
		vesync_publish_topic(root, 0, 0);
		//vesync_publish_cjson(root, 0, 0);
	}

	cJSON_Delete(root);
}

/**
 * @brief 解析cid主题信息
 * @param data 
 */
static bool vesync_prase_topic_cid_packet(product_config_t *info,char *data)
{
	bool ret = false;
	cJSON *root = cJSON_Parse( data );
	if(NULL == root){
		LOG_I(TAG, "Parse cjson error !\r\n" );
		return;
	}

	cJSON* state = cJSON_GetObjectItemCaseSensitive( root, "state" );
	if(state != NULL){
		cJSON* cid = cJSON_GetObjectItemCaseSensitive( state, "cid" );
		if(cJSON_IsString(cid)){
			if( strlen(cid->valuestring) == CID_LENGTH ){
				strcpy((char *)info->cid, cid->valuestring);
				LOG_I(TAG, "cid : %s\r\n", info->cid);

				//info->cid_holder = CID_HOLDER;
				if(vesync_flash_write_product_config(info)==0){
				 	//set_production_status(RPODUCTION_TEST_PASS);

				// 	vesync_resp_json_to_server(cid);
				// }else{
				// 	set_production_status(PRODUCTION_TEST_FAIL);
				// 	LOG_I(TAG, "Save cid to flash fail !\r\n");
				// 	LOG_I(TAG, "Erase sector ret : %d\r\n",erase_ret);
				// 	LOG_I(TAG, "Write sector ret : %d\r\n",write_ret);

				// 	cJSON *cid_error = cJSON_CreateObject();
				// 	if(NULL != cid_error){
				// 		cJSON_AddStringToObject(cid_error,"cid","error");
				// 		cJSON* cid_report = cJSON_GetObjectItem( cid_error, "cid" );
				// 		vesync_resp_json_to_server(cid_report);
				// 	}
				// 	cJSON_Delete(cid_error);            
				 }
			}
		}
	}
	cJSON_Delete( root );
}

/**
 * @brief 解析产测系统数据流
 * @param data 
 */
void vesync_prase_production_json_packet(const char *topic,char *data)
{
	if(vesync_get_production_status() == PRODUCTION_EXIT) return;

	vesync_prase_topic_cid_packet(&net_info,data);
	if(strcmp(topic, sub_Topic) == 0){	//订阅的cid主题
		vesync_prase_topic_cid_packet(&net_info,data);
	}else{

	}
}
