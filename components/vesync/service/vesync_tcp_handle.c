/**
 * @file vesync_tcp_handle.c
 * @brief 本地TCP服务器的通信及数据交互处理，用于配网时与APP进行交互
 * @author WatWu
 * @date 2018-11-17
 */

#include <stdlib.h>
#include <stdint.h>

#include "vesync_tcp_server.h"
#include "vesync_tcp_handle.h"
// #include "vesync_device.h"
// #include "vesync_mqtt.h"
// #include "vesync_interface.h"
#include "vesync_main.h"
#include "vesync_build_cfg.h"

#include "encryptor.h"
#include "cJSON.h"

static const char* TAG = "vesync_tcp_handle";

// static void config_network_success(void)
// {
// 	if(vesync_get_network_configmode() == AP_CONFIG)
// 	{
// 		confignet_upload_realtime_log(ERR_CONFIG_NET_DATA_OK,"CONFIG_NET_DATA_OK");
// 	}
// }

/**
 * [handle_config_net_json  解析处理APP通过TCP发来的配网json数据]
 * @param  json [配网json数据]
 * @return    	[无]
 */
static void handle_config_net_json(cJSON *json)
{
// 	cJSON *root = json;

// 	cJSON *pid = cJSON_GetObjectItemCaseSensitive(root, "pid");
// 	if(true == cJSON_IsString(pid))
// 	{
// 		strcpy(g_device_config.mqtt_conf.pid, pid->valuestring);
// 		LOG_I(TAG, "pid : %s", g_device_config.mqtt_conf.pid);
// 	}

// 	cJSON *configKey = cJSON_GetObjectItemCaseSensitive(root, "configKey");
// 	if(true == cJSON_IsString(configKey))
// 	{
// 		strcpy(g_device_config.mqtt_conf.configkey, configKey->valuestring);
// 		LOG_I(TAG, "configKey : %s", g_device_config.mqtt_conf.configkey);
// 	}

// 	cJSON *serverDN = cJSON_GetObjectItemCaseSensitive(root, "serverDN");
// 	if(true == cJSON_IsString(serverDN))
// 	{
// 		char *str = strstr(serverDN->valuestring, ":");
// 		if(NULL != str)
// 		{
// 			strncpy(g_device_config.mqtt_conf.server_dn, serverDN->valuestring, str - serverDN->valuestring);
// 			g_device_config.mqtt_conf.server_dn[str - serverDN->valuestring] = '\0';

// 			int port = atoi(str + 1);
// 			g_device_config.mqtt_conf.server_port = port;
// 			LOG_I(TAG, "mqtt port : %d", g_device_config.mqtt_conf.server_port);
// 		}
// 		else
// 		{
// 			strcpy(g_device_config.mqtt_conf.server_dn, serverDN->valuestring);
// 		}
// 		LOG_I(TAG, "serverDN : %s", g_device_config.mqtt_conf.server_dn);
// 	}

// 	cJSON *serverIP = cJSON_GetObjectItemCaseSensitive(root, "serverIP");
// 	if(true == cJSON_IsString(serverIP))
// 	{
// 		char *str = strstr(serverIP->valuestring, ":");
// 		if(NULL != str)
// 		{
// 			strncpy(g_device_config.mqtt_conf.server_ip, serverIP->valuestring, str - serverIP->valuestring);
// 		}
// 		else
// 		{
// 			strcpy(g_device_config.mqtt_conf.server_ip, serverIP->valuestring);
// 		}
// 		LOG_I(TAG, "serverIP : %s", g_device_config.mqtt_conf.server_ip);
// 	}

// 	cJSON *wifiSSID = cJSON_GetObjectItemCaseSensitive(root, "wifiSSID");
// 	if(true == cJSON_IsString(wifiSSID))
// 	{
// 		strcpy(g_device_config.wifi_conf.ssid, wifiSSID->valuestring);
// 		LOG_I(TAG, "wifiSSID : %s", g_device_config.wifi_conf.ssid);
// 	}

// 	cJSON *wifiPassword = cJSON_GetObjectItemCaseSensitive(root, "wifiPassword");
// 	if(true == cJSON_IsString(wifiPassword))
// 	{
// 		strcpy(g_device_config.wifi_conf.password, wifiPassword->valuestring);
// 		LOG_I(TAG, "wifiPassword : %s", g_device_config.wifi_conf.password);
// 	}

// 	cJSON *wifiStaticIP = cJSON_GetObjectItemCaseSensitive(root, "wifiStaticIP");
// 	if(true == cJSON_IsString(wifiStaticIP))
// 	{
// 		strcpy(g_device_config.wifi_conf.staticip, wifiStaticIP->valuestring);
// 		LOG_I(TAG, "wifiStaticIP : %s", g_device_config.wifi_conf.staticip);
// 	}

// 	cJSON *wifiGateway = cJSON_GetObjectItemCaseSensitive(root, "wifiGateway");
// 	if(true == cJSON_IsString(wifiGateway))
// 	{
// 		strcpy(g_device_config.wifi_conf.gateway, wifiGateway->valuestring);
// 		LOG_I(TAG, "wifiGateway : %s", g_device_config.wifi_conf.gateway);
// 	}

// 	cJSON *wifiDNS = cJSON_GetObjectItemCaseSensitive(root, "wifiDNS");
// 	if(true == cJSON_IsString(wifiDNS))
// 	{
// 		strcpy(g_device_config.wifi_conf.dns, wifiDNS->valuestring);
// 		LOG_I(TAG, "wifiDNS : %s", g_device_config.wifi_conf.dns);
// 	}

// #ifndef TCP_DEBUG_ENABLE
// 	cJSON *tcpDebugPort = cJSON_GetObjectItemCaseSensitive(root, "tcpDebugPort");
// 	if(true == cJSON_IsString(tcpDebugPort))
// 	{
// 		if(strcmp(tcpDebugPort->valuestring, "on") == 0)
// 		{
// 			g_device_config.tcp_debug = TCP_DEBUG_ON;

// 			LOG_I(TAG, "tcp debug on");
// 		}
// 		else if(strcmp(tcpDebugPort->valuestring, "off") == 0)
// 		{
// 			g_device_config.tcp_debug = TCP_DEBUG_OFF;
// 			LOG_I(TAG, "tcp debug off");
// 		}
// 	}
// 	else
// 	{
// 		g_device_config.tcp_debug = TCP_DEBUG_OFF;
// 	}
// #else
// 	g_device_config.tcp_debug = TCP_DEBUG_ON;
// #endif

// 	if(strlen(g_product_config.cid) != CID_LENGTH)
// 	{
// 		LOG_E(TAG, "CID was missed !");

// 		// confignet_upload_realtime_log(ERR_CONFIG_STATION_CID_MISS,"CONFIG_STATION_CID_MISS");
// 	}
// 	else if(g_device_config.wifi_conf.ssid != 0 && g_device_config.mqtt_conf.server_ip != 0 &&
// 	        !strcmp(g_device_config.mqtt_conf.pid, g_product_config.pid))
// 	{
// 		g_device_config.sta_holder = DEVCONF_HOLDER;	//标记已初始化过设备

// 		vesync_init_mqtt_client(MQTT_INIT_FOR_CONFIG_NET);		    //配网初始化MQTT客户端

// 		uint32_t event = CONFIG_NET_MQTT_READY;
// 		xTaskNotify(event_center_taskhd, event, eSetBits);			//通知事件处理中心任务MQTT已准备好

// 		if(vesync_get_network_configmode() == SMART_CONFIG)
// 		{
// 			// confignet_upload_realtime_log(ERR_CONFIG_NET_DATA_OK,"CONFIG_NET_DATA_OK");
// 		}
// 		else
// 		{
// 			ret = vesync_regist_networkconnected_cb(config_network_success);
// 			if(ret == 0)
// 			{
// 				LOG_E(TAG, "Regist network connected callback failed !");
// 			}
// 		}
// 	}
// 	else
// 	{
// 		//配网的设备产品类型不匹配，不继续配网
// 		LOG_E(TAG, "PID was not matched !");

// 		if(strcmp(g_device_config.mqtt_conf.pid, g_product_config.pid) != 0)	//PID不匹配
// 		{
// 			// confignet_upload_realtime_log(ERR_CONFIG_PID_DO_NOT_MATCHED,"CONFIG_PID_DO_NOT_MATCHED");
// 		}
// 		else
// 		{
// 			// confignet_upload_realtime_log(ERR_CONFIG_STATION_MISSING_PARAMETER,"CONFIG_STATION_MISSING_PARAMETER");
// 		}
// 	}
}

/**
 * [vesync_handle_tcp_msg  解析APP端通过本地网络接口直接发送过来的数据]
 * @param  data   [网络数据]
 * @param  length [数据长度]
 * @return        [无]
 */
static void vesync_handle_tcp_msg(const uint8_t *data, int length)
{
	cJSON *root = cJSON_Parse((char*)data);
	if(NULL == root)
	{
		LOG_E(TAG, "Parse cjson error !");
		return;
	}

	// vesync_printf_cjson(root);				//json标准格式，带缩进

	//获取uri，判断是否为配网的json数据
	cJSON *uri = cJSON_GetObjectItemCaseSensitive(root, "uri");
	if(true == cJSON_IsString(uri))
	{
		LOG_I(TAG, "Found uri !");
		if(!strcmp(uri->valuestring, "/beginConfigRequest"))		//开始配网
		{
			//确认是配网信息
			handle_config_net_json(root); 							//解析处理json数据
		}
		else if(!strcmp(uri->valuestring, "/cancelConfig"))			//取消配网
		{
			// cancel_config_net();
		}
	}

	cJSON_Delete(root);									//务必记得释放资源！
}

/**
 * @brief 发送TCP消息数据到APP端
 * @param msg 		[原始数据]
 * @param msg_len 	[数据长度]
 * @return int 		[发送结果]
 */
int vesync_send_tcp_msg_to_app(uint8_t *msg, uint32_t msg_len)
{
	int ret = -1;

#ifdef TCP_AES_ENABLE
	uint8_t *encryptData = NULL;
	int encryptLen = 0;
	encryptLen = AES_Encrypt(msg, msg_len, &encryptData);
	if(encryptLen)  //加密成功
	{
		ret = vesync_tcp_server_send(encryptData, encryptLen);
		free(encryptData);
	}
	else
	{
		LOG_E(TAG, "TCP msg encrypt fail !");
	}
#else
	ret = vesync_tcp_server_send(msg, msg_len);
#endif

	return ret;
}

/**
 * [vesync_tcpserver_recv_callback  TCP服务器数据接收回调函数]
 * @param  clinet_fd	[TCP连接句柄]
 * @param  recvdata 	[TCP数据包]
 * @param  length   	[数据包长度]
 * @return          	[无]
 */
void vesync_tcpserver_recv_callback(int client_fd, char *recvdata, unsigned short length)
{
	if(length == 1)
	{
		LOG_I(TAG, "Next json length : %d ", recvdata[0]);
	}
	else
	{
		uint8_t *pendingData = NULL;
		int dataLen = 0;

#ifdef TCP_AES_ENABLE
		uint8_t *decryptData = NULL;
		int decryptLen = 0;
		decryptLen = AES_Decrypt((uint8_t*)recvdata, length, &decryptData);
		if(decryptLen)  //解密成功
		{
			LOG_I(TAG, "Decrypt success !");
			pendingData = malloc(decryptLen);
			if(NULL != pendingData)
			{
				memcpy(pendingData, decryptData, decryptLen);
				dataLen = decryptLen;
			}
			free(decryptData);
		}
#else
		pendingData = malloc(length);
		if(NULL != pendingData)
		{
			memcpy(pendingData, recvdata, length);
			dataLen = length;
		}
#endif

		if(NULL != pendingData)
		{
			vesync_handle_tcp_msg(pendingData, dataLen); 		//处理TCP数据，此处会改变了设备配置信息全局变量DevConfig
			free(pendingData);
		}
	}
}
