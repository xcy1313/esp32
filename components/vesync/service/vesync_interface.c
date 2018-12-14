/**
 * @file vesync_interface.c
 * @brief vesync设备的独立业务接口
 * @author WatWu
 * @date 2018-11-21
 */

#include "vesync_main.h"
#include "vesync_interface.h"

#include "cJSON.h"

#define NET_CB_MAX_NUM          20

typedef struct
{
	int cb_count;               //已注册的回调函数数量
	network_connected_cb_t (*cb_buffer[NET_CB_MAX_NUM])(void);
} vesync_network_cb_t;

static const char* TAG = "vesync_device";

static vesync_network_cb_t 		network_callback_center;				//网络连接成功的统一回调中心

static json_recv_cb_t 			json_recv_callback = NULL;				//定义json接收回调函数指针
static connected_server_cb_t 	connected_server_callback = NULL;		//定义连接服务器成功回调函数指针
static restore_device_cb_t		restore_device_callback = NULL;			//定义复位设备回调函数指针
// static production_status_cb_t 	production_status_callback = NULL;		//定义产测状态回调函数指针

/**
 * [vesync_get_vesync_sdk_version 获取VeSync版本号]
 * @return  [版本号字符串]
 */
const char* vesync_get_vesync_sdk_version(void)
{
	return VESYNC_VERSION;
}

/**
 * [vesync_regist_recvjson_cb 注册json接收回调函数]
 * @param  cb [json接收回调函数，该函数由应用层定义，mqtt收到json数据时会调用该函数]
 * @return    [无]
 */
void vesync_regist_recvjson_cb(json_recv_cb_t cb)
{
	json_recv_callback = cb;
}

/**
 * [vesync_regist_connectedserver_cb 注册连接服务器成功后的回调函数]
 * @param  cb [连接服务器成功回调函数，该函数由应用层定义，每次成功连接上服务器后会调用该函数，可用于上报设备上电数据信息]
 * @return    [无]
 */
void vesync_regist_connectedserver_cb(connected_server_cb_t cb)
{
	connected_server_callback = cb;
}

/**
 * @brief 上报客户端固件版本信息
 * @param devName		[设备名称]
 * @param devVersion 	[设备版本]
 * @param firmName 		[设备固件名称]
 * @param firmVersion 	[设备固件版本]
 */
void vesync_report_client_firmversion(const char *devName, const char *devVersion, const char *firmName, const char *firmVersion)
{
	cJSON* root = NULL;
	cJSON* state = NULL;

	root = cJSON_CreateObject();

	if(root == NULL)
	{
		LOG_E(TAG, "Create cJSON objects error !");
		return;
	}

	//添加root成员
	cJSON_AddItemToObject(root, "state", state = cJSON_CreateObject());

	if(state != NULL)
	{
		cJSON* reported = NULL;

		//添加state目录成员
		cJSON_AddItemToObject(state, "reported", reported = cJSON_CreateObject());
		cJSON_AddNullToObject(state, "desired");	//desired成员赋值为null

		//添加reported目录成员
		if(reported != NULL)
		{
			cJSON* version = NULL;

			//添加version目录成员
			cJSON_AddItemToObject(reported, "version", version = cJSON_CreateObject());
			if(version != NULL)
			{
				cJSON_AddStringToObject(version, "deviceName", devName);
				cJSON_AddStringToObject(version, "deviceVersion", devVersion);
				cJSON_AddStringToObject(version, "firmName", firmName);
				cJSON_AddStringToObject(version, "firmVersion", firmVersion);
			}
		}

		char* out = cJSON_PrintUnformatted(root);	//不带缩进格式

		// if(vesync_mqtt_get_status() == ON_LINE)
		// {
		// 	vesync_mqtt_publish_rawdata_to_topic(client_public_topic, out, strlen(out), 0, 0);
		// }

		free(out);
	}

	cJSON_Delete(root);
}

/**
 * [vesync_regist_restoredevice_cb 注册复位设备的回调函数]
 * @param  cb [复位设备回调函数，该函数由应用层定义，在APP端进行复位设备且设备响应后会调用该函数]
 * @return    [无]
 */
void vesync_regist_restoredevice_cb(restore_device_cb_t cb)
{
	restore_device_callback = cb;
}

/**
 * @brief 注册vesync网络连接成功回调函数，可以多次注册多个，在网络连接成功后会逐个调用
 * @param cb   [网络连接成功回调函数，在网络连接成功后会被调用]
 * @return int [注册结果，0 - 注册失败；其他数值则是已经成功注册的回调函数数量]
 */
int vesync_regist_networkconnected_cb(network_connected_cb_t cb)
{
	if(network_callback_center.cb_count < NET_CB_MAX_NUM)
	{
		if(cb != NULL)
		{
			int i, cb_exit_flag = false;
			for(i = 0; i < network_callback_center.cb_count; i++)
			{
				if((void*)cb == network_callback_center.cb_buffer[i])	//已经存在
				{
					LOG_W(TAG, "The callback funtion was exited !");
					cb_exit_flag = true;
					break;
				}
			}

			if(cb_exit_flag == false)
			{
				LOG_I(TAG, "Add callback funtion success !");
				network_callback_center.cb_buffer[network_callback_center.cb_count++] = (void*)cb;
				return network_callback_center.cb_count;
			}
		}
	}

	return 0;
}

/**
 * @brief 网络连接成功回调函数统一调用
 */
void network_connected_cb_run(void)
{
	int i;
	for(i = 0; i < network_callback_center.cb_count; i++)
	{
		(*(network_callback_center.cb_buffer[i]))();
	}
}

/**
 * @brief 发布Cjson格式的数据到服务器，使用内部默认主题
 * @param json 		[Cjson格式的数据，由应用层保证]
 * @param qos 		[发布消息的qos]
 * @param retain 	[发布消息的retain]
 * @return int 	[true - 成功存入mqtt消息发布队列]
 */
int vesync_publish_cjson(cJSON* json, int qos, int retain)
{
	int ret = false;
	char *out = cJSON_PrintUnformatted(json);				//不带缩进格式
	// ret = vesync_mqtt_publish_rawdata_to_topic(client_public_topic, out, strlen(out), qos, retain);
	free(out);											//务必记得释放资源！

	return ret;
}

/**
 * @brief 发布原始数据到服务器，使用内部默认主题
 * @param data 		[原始通用格式的数据]
 * @param datalen	[数据长度]
 * @param qos 		[发布消息的qos]
 * @param retain 	[发布消息的retain]
 * @return int		[发布结果，TRUE - 成功存入mqtt消息发布队列]
 */
int vesync_publish_rawdata(char* data, int datalen, int qos, int retain)
{
	int ret = false;
	// ret = vesync_mqtt_publish_rawdata_to_topic(client_public_topic, data, datalen, qos, retain);

	return ret;
}

/**
 * @brief 发布cjson格式的数据到指定主题
 * @param json 		[cjson格式的数据，由应用层保证]
 * @param topic 	[发布的主题]
 * @param qos 		[发布消息的qos]
 * @param retain 	[发布消息的retain]
 * @return int 	[发布结果，TRUE - 成功存入mqtt消息发布队列]
 */
int vesync_publish_cjson_to_topic(cJSON* json, char* topic, int qos, int retain)
{
	int ret = false;
	char *out = cJSON_PrintUnformatted(json);				//不带缩进格式
	// ret = vesync_mqtt_publish_rawdata_to_topic(topic, out, strlen(out), qos, retain);
	free(out);												//务必记得释放资源！

	return ret;
}

/**
 * @brief 发布原始格式的数据到指定主题
 * @param data 		[原始通用格式的数据]
 * @param datalen 	[数据长度]
 * @param topic 	[发布的主题]
 * @param qos 		[发布消息的qos]
 * @param retain 	[发布消息的retain]
 * @return int 	[发布结果，TRUE - 成功存入mqtt消息发布队列]
 */
int vesync_publish_rawdata_to_topic(char* data, int datalen, char* topic, int qos, int retain)
{
	int ret = false;
	// ret = vesync_mqtt_publish_rawdata_to_topic(topic, data, datalen, qos, retain);

	return ret;
}

/**
 * @brief 直接按照标准缩进格式打印cjson数据，内部实现转换
 * @param json [cjson格式的数据]
 */
void vesync_printf_cjson(cJSON *json)
{
	char *out = cJSON_Print(json);
	printf("\n%s\n", out);
	free(out);
}
