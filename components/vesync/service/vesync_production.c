/**
 * @file vesync_production.c
 * @brief 产测模式
 * @author WatWu
 * @date 2018-05-28
 */

#include <stdlib.h>

#include "vesync_log.h"
#include "vesync_wifi.h"
#include "vesync_net_service.h"
#include "vesync_production.h"

#include "vesync_build_cfg.h"

//上电更新信息
#define PUBLISHDATAINFO "{\"state\":{\"reported\":{\"delDevice\":null,\"restore\":null,\"mac\":\"%s\",\"connectionStatus\":\"online\"},\"desired\":null}}"

static const char* TAG = "vesync_production";

static production_status_e production_status = PRODUCTION_EXIT;
static production_status_cb_t production_status_cb = NULL;

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
	LOG_I(TAG, "==============================");

	//vesync_client_connect_wifi(PRODUCTION_WIFI_SSID, PRODUCTION_WIFI_KEY);
	//vesync_config_cloud_mqtt_client("64464d9993b347888adb3bef18c10404","192.168.16.25",61613,"etekcity","hardware");
	vesync_config_cloud_mqtt_client_with_tls(cid_mac, PRODUCTION_SERVER_ADDR, 1883, username, "0");
	vesync_mqtt_client_connect_to_cloud();
}

/**
 * @brief 订阅产测系统的mqtt主题
 * @return int 	[订阅结果]
 */
int vesync_subscribe_production_topic(void)
{
	int ret;

	char mac_string[6 * 3];
	vesync_get_wifi_sta_mac_string(mac_string);
	char production_topic[100];
	sprintf(production_topic, "$vesync/things/%s/shadow/update/delta", mac_string);

	ret = vesync_mqtt_subscribe_topic(vesync_get_cloud_mqtt_client(), production_topic, MQTT_QOS1);
	return ret;
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
