/**
 * @file vesync_net_service.c
 * @brief 网络基础业务
 * @author WatWu
 * @date 2018-12-05
 */

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "vesync_main.h"
#include "vesync_wifi.h"
#include "vesync_mqtt.h"

#include "vesync_net_service.h"
#include "vesync_production.h"
#include "vesync_interface.h"
#include "vesync_device.h"

#include "vesync_ca_cert.h"
#include "vesync_build_cfg.h"
#include "vesync_bt_hal.h"
#include "vesync_flash.h"

#include "vesync_https.h"
#include "vesync_ca_cert.h"

static const char* TAG = "net_service";

static vesync_mqtt_client_t s_vesync_client = {0};
static vesync_mqtt_status_e s_vesync_mqtt_status = MQTT_OFFLINE;

#define HTTPS_SERVER_ADDR_LEN		128
static char s_https_server_addr[HTTPS_SERVER_ADDR_LEN] = {"test-online.vesync.com"};
static char vesync_https_service_token[64] ={"\0"};

/**
 * @brief 返回token
 * @param token 
 */
void vesync_get_https_token(char *token)
{
	strcpy(token,vesync_https_service_token);
	LOG_I(TAG, "https token：[%s]" ,token);
}

/**
 * @brief 连接WiFi的结果回调
 * @param status [WiFi连接状态，包括连接成功和连接掉线，以及连接失败时的错误原因]
 */
static void vesync_connect_wifi_callback(vesync_wifi_status_e status)
{
	switch(status)
	{
		case VESYNC_WIFI_GOT_IP:
			xTaskNotify(event_center_taskhd, NETWORK_CONNECTED, eSetBits);			//通知事件处理中心任务
			break;
		case VESYNC_WIFI_LOST_IP:
			xTaskNotify(event_center_taskhd, NETWORK_DISCONNECTED, eSetBits);		//通知事件处理中心任务
			break;
		default:
			break;
	}
}

/**
 * @brief 初步处理接收到的mqtt数据
 * @param topic 	[接收的主题]
 * @param topic_len [主题长度]
 * @param data 		[接收的数据]
 * @param data_len 	[数据长度]
 */
static void vesync_handle_mqtt_data(char *topic, int topic_len, char *data, int data_len)
{
	// vesync_prase_production_json_packet(event->topic ,event->data);
	// LOG_I(TAG, "MQTT recv topic=%.*s", topic_len, topic);
	// LOG_I(TAG, "MQTT recv data=%.*s", data_len, data);
	char *data_buf = malloc(data_len + 1);
	if(NULL != data_buf)
	{
		memcpy(data_buf, data, data_len);
		data_buf[data_len] = '\0';
		vesync_call_recvjson_cb(data_buf);
		free(data_buf);
	}
}

/**
 * @brief mqtt事件回调函数
 * @param event 		[mqtt事件]
 * @return esp_err_t 	[错误码]
 */
static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
	// esp_mqtt_client_handle_t client = event->client;
	// your_context_t *context = event->context;
	switch(event->event_id)
	{
		case MQTT_EVENT_CONNECTED:
			LOG_I(TAG, "MQTT_EVENT_CONNECTED");
			s_vesync_mqtt_status = MQTT_ONLINE;
			xTaskNotify(event_center_taskhd, MQTT_CONNECTED, eSetBits);			//通知事件处理中心任务
			break;

		case MQTT_EVENT_DISCONNECTED:
			LOG_I(TAG, "MQTT_EVENT_DISCONNECTED");
			s_vesync_mqtt_status = MQTT_OFFLINE;
			xTaskNotify(event_center_taskhd, MQTT_DISCONNECTED, eSetBits);		//通知事件处理中心任务
			break;

		case MQTT_EVENT_SUBSCRIBED:
			LOG_I(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
			break;

		case MQTT_EVENT_UNSUBSCRIBED:
			LOG_I(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
			break;

		case MQTT_EVENT_PUBLISHED:
			LOG_I(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
			break;

		case MQTT_EVENT_DATA:
			LOG_I(TAG, "MQTT_EVENT_DATA");
			// LOG_I(TAG, "TOPIC=%.*s", event->topic_len, event->topic);
			// LOG_I(TAG, "DATA=%.*s", event->data_len, event->data);
			vesync_handle_mqtt_data(event->topic, event->topic_len, event->data, event->data_len);
			break;

		case MQTT_EVENT_ERROR:
			LOG_I(TAG, "MQTT_EVENT_ERROR");
			break;
	}
	return ESP_OK;
}

/**
 * @brief 获取mqtt客户端在线状态
 * @return vesync_mqtt_status_e [mqtt客户端在线状态]
 */
vesync_mqtt_status_e vesync_get_mqtt_client_status(void)
{
	return s_vesync_mqtt_status;
}

/**
 * @brief 获取云平台mqtt客户端指针
 * @return vesync_mqtt_client_t* [vesync云平台mqtt客户端指针]
 */
vesync_mqtt_client_t* vesync_get_cloud_mqtt_client(void)
{
	return &s_vesync_client;
}

/**
 * @brief vesync客户端连接WiFi
 * @param wifi_ssid 	[WiFi账号]
 * @param wifi_password [WiFi密码]
 */
void vesync_client_connect_wifi(char *wifi_ssid, char *wifi_password)
{
	vesync_connect_wifi(wifi_ssid, wifi_password);
}

/**
 * @brief 配置vesync云平台的mqtt客户端
 * @param client_id 	[设备cid]
 * @param server_addr 	[mqtt服务器地址]
 * @param server_port 	[mqtt服务器端口]
 * @param username 		[mqtt用户名]
 * @param password 		[mqtt用户密码]
 */
void vesync_config_cloud_mqtt_client(char *client_id, char *server_addr, uint16_t server_port, char *username, char *password)
{
	if(vesync_get_mqtt_client_status() == MQTT_ONLINE)
	{
		vesync_mqtt_client_disconnet_from_cloud();
	}
	if(NULL != s_vesync_client.client_handle)
		vesync_delete_mqtt_client(&s_vesync_client);
	vesync_init_mqtt_client(&s_vesync_client, client_id, server_addr, server_port, username, password, mqtt_event_handler);
}

/**
 * @brief 配置vesync云平台的mqtt客户端
 * @param client_id 	[设备cid]
 * @param server_addr 	[mqtt服务器地址]
 * @param server_port 	[mqtt服务器端口]
 * @param username 		[mqtt用户名]
 * @param password 		[mqtt用户密码]
 */
void vesync_config_cloud_mqtt_client_with_tls(char *client_id, char *server_addr, uint16_t server_port, char *username, char *password)
{
	if(vesync_get_mqtt_client_status() == MQTT_ONLINE)
	{
		vesync_mqtt_client_disconnet_from_cloud();
	}
	if(NULL != s_vesync_client.client_handle)
		vesync_delete_mqtt_client(&s_vesync_client);
	vesync_init_mqtt_client_with_tls(&s_vesync_client, client_id, server_addr, server_port,
	                                 username, password, mqtt_event_handler,
	                                 vesync_ca_cert_pem, vesync_client_cert_pem, vesync_client_key_pem);
}

/**
 * @brief vesync平台mqtt客户端连接与服务器
 */
void vesync_mqtt_client_connect_to_cloud(void)
{
	vesync_mqtt_connect(&s_vesync_client);
}

/**
 * @brief vesync平台mqtt客户端断开与服务器的连接
 */
void vesync_mqtt_client_disconnet_from_cloud(void)
{
	vesync_mqtt_disconnect(&s_vesync_client);
}

/**
 * @brief 用户调用初始化wifi模块
 */
void vesync_clinet_wifi_module_init(bool power_save)
{
	vesync_init_wifi_module(vesync_connect_wifi_callback,power_save);
}

/**
 * @brief 为cjson格式的协议方法添加固定的接口头部
 * @param method 	[接口方法名]
 * @param body 		[方法的数据体]
 * @return cJSON* 	[添加了固定头部后的cjson格式数据，使用完后必须调用cJSON_Delete进行删除！！！]
 */
cJSON* vesync_json_add_method_head(char *trace_id,char *method,cJSON *body)
{
	cJSON *root = cJSON_CreateObject();
	if(NULL != root)
	{
		// time_t seconds;
		// seconds = time((time_t *)NULL);
		// char traceId_buf[64];
		// itoa(seconds, traceId_buf, 10);
		cJSON_AddStringToObject(root, "traceId", trace_id);		//==TODO==，需要修改成毫秒级
		cJSON_AddStringToObject(root, "method", method);
		cJSON_AddStringToObject(root, "pid", DEV_PID);
		cJSON_AddStringToObject(root, "cid", (const char*)(product_config.cid));
		if(NULL != body){
			cJSON_AddItemReferenceToObject(root, body->string, body);
		}
	}

	return root;
}

static uint8_t vesync_json_https_service_parse(uint8_t mask,char *read_buf)
{
	char *index = strchr(read_buf,'{');	//截取有效json
	cJSON *root = cJSON_Parse(index);
	uint8_t ret = 1;
	if(NULL == root){
		LOG_I(TAG,"Parse cjson error !\r\n");
		return ret;
	}
	vesync_printf_cjson(root);				//json标准格式，带缩进

	cJSON *traceId = cJSON_GetObjectItemCaseSensitive(root, "traceId");
	if(true == cJSON_IsString(traceId)){
		LOG_I(TAG,"trace_id : %s\r\n", traceId->valuestring);
	}

	cJSON *code = cJSON_GetObjectItemCaseSensitive(root, "code");
	if(true == cJSON_IsNumber(code)){
		LOG_I(TAG,"code : %d\r\n", code->valueint);
		if(code->valueint == 0){
			cJSON *result = cJSON_GetObjectItemCaseSensitive(root, "result");
			if(true == cJSON_IsObject(result)){
				cJSON *token = cJSON_GetObjectItemCaseSensitive(result, "token");
				if(true == cJSON_IsString(token)){
					strcpy((char *)vesync_https_service_token,token->valuestring);
					LOG_I(TAG,"token : %s\r\n", vesync_https_service_token);
				}
				cJSON *expireIn = cJSON_GetObjectItemCaseSensitive(result, "expireIn");
				if(true == cJSON_IsNumber(expireIn)){
					LOG_I(TAG,"expireIn : %d\r\n", expireIn->valueint);	//token的过期时间 单位s
					vesync_set_device_status(DEV_CONFNET_ONLINE);		//设备已连上服务器
					if(mask == NETWORK_CONFIG_REQ){
						vesync_flash_write_net_info(&net_info);
					}
				}
			}
			ret = 0;
		}
	}
	cJSON_Delete(root);	

	return ret;
}

/**
 * @brief vesync https配网信息注册
 * @param mask 
 */
void vesync_json_add_https_service_register(uint8_t mask)
{
	vesync_wait_network_connected(5000);
    cJSON *root = cJSON_CreateObject();
	if(NULL == root)    return;

    char *req_method = NULL;
    cJSON* info = NULL;

    int ret =0;
    char recv_buff[1024];
    int buff_len = sizeof(recv_buff);

	time_t seconds;
	seconds = time((time_t *)NULL);
	char traceId_buf[64];
	itoa(seconds, traceId_buf, 10);

    switch(mask){
        case NETWORK_CONFIG_REQ:
            cJSON_AddItemToObject(root, "info", info = cJSON_CreateObject());
            if(NULL != info){
                char mac[6 * 3];
                vesync_get_wifi_sta_mac_string(mac);
                cJSON_AddStringToObject(info, "configKey", (char *)net_info.mqtt_config.configKey);
                cJSON_AddStringToObject(info, "accountID", (char *)net_info.station_config.account_id);
                cJSON_AddStringToObject(info, "mac", mac);
                cJSON_AddStringToObject(info, "version", FIRM_VERSION);
                cJSON_AddNumberToObject(info, "rssi", -56);
                cJSON_AddStringToObject(info, "timeZone", "8");
                req_method = "deviceRegister";
				LOG_I("https", "\nNETWORK_CONFIG_REQ");
            }
            break;
        case REFRESH_TOKEN_REQ:
                req_method = "refreshDeviceToken";
				info = NULL;
				LOG_I("https", "\nREFRESH_TOKEN_REQ");
            break;
		default:
			break;
	}
	cJSON *report = vesync_json_add_method_head(traceId_buf,req_method,info);
    char* out = cJSON_PrintUnformatted(report);
    LOG_I("JSON", "\n%s", out);

	vesync_set_https_server_address((char *)net_info.station_config.server_url);
	LOG_I(TAG, "servel url %s",net_info.station_config.server_url);
	LOG_I(TAG, "servel account_id %s",net_info.station_config.account_id);

	ret = vesync_https_client_request(req_method, out, recv_buff, &buff_len, 2 * 1000);
	if(buff_len > 0 && ret == 0){
		LOG_I(TAG, "Https recv %d byte data : \n%s", buff_len, recv_buff);
		vesync_json_https_service_parse(mask,recv_buff);
	}

    free(out);
    cJSON_Delete(report);
    cJSON_Delete(root);
}

/**
 * @brief 设置https服务器地址
 * @param address 	[服务器域名或者IP地址]
 * @return int 		[设置结果，0为成功]
 */
int vesync_set_https_server_address(char *address)
{
    if(NULL == address)
    {
        LOG_E(TAG, "Https server address is null !");
        return -1;
    }
    if(strlen(address) >= HTTPS_SERVER_ADDR_LEN)
    {
        LOG_E(TAG, "Https server address is too long");
        return -1;
    }
    strcpy(s_https_server_addr, address);
    return 0;
}

/**
 * @brief vesync平台https客户端发起请求
 * @param method 		[请求的接口名]
 * @param body 			[请求接口的数据内容]
 * @param recv_buff 	[返回的数据内容缓存buffer]
 * @param recv_len 		[返回的数据内容长度指针，传入时为缓存buffer的长度，供内部判断buffer大小是否足够，足够时内部把返回的数据拷贝至buffer，并赋值该值为数据长度]
 * @param wait_time_ms 	[超时等待时间]
 * @return int 			[请求结果]
 */
int vesync_https_client_request(char *method, char *body, char *recv_buff, int *recv_len, int wait_time_ms)
{
    int ret;
    char url[64];
    sprintf(url, "/cloud/v1/deviceWeb/%s", method);
    ret = vesync_https_request(s_https_server_addr, "443", url, body, recv_buff, recv_len, wait_time_ms);
    return ret;
}

/**
 * @brief 刷新token
 * @return uint32_t 设备已有配网记录
 */
uint32_t vesync_refresh_https_token(void)
{
	xTaskNotify(event_center_taskhd, REFRESH_HTTPS_TROKEN, eSetBits);			//通知事件处理中心任务
	return 0;
}

/**
 * @brief 配网注册
 */
void vesync_register_https_net(void)
{
	xTaskNotify(event_center_taskhd, HTTPS_NET_CONFIG_REGISTER, eSetBits);			//通知事件处理中心任务
}
