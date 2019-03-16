/**
 * @file vesync_developer.c
 * @brief vesync设备SDK开发者模式
 * @author WatWu
 * @date 2018-08-03
 */

#include <string.h>
#include <unistd.h>

#include "freertos/FreeRTOS.h"
#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "lwip/sockets.h"
#include "lwip/sockets.h"
#include "netif/etharp.h"

#include "cJSON.h"

#include "vesync_log.h"
#include "vesync_task_def.h"
#include "vesync_interface.h"
#include "vesync_build_cfg.h"
#include "vesync_ota.h"

//开发者模式的TCP服务器监听端口
#define DEVELOPER_LISTEN_PORT			55555

static const char* TAG = "vesync_developer";

static struct sockaddr_in server_addr;
static int socket_fd;
static int client_fd;

/**
 * @brief 打印系统任务管理器到缓存数组
 * @param buffer 	[缓存数组]
 * @param buf_len 	[缓存数组长度]
 */
static void printf_os_task_manager_to_buf(char *buffer, int buf_len)
{
	uint8_t* pcWriteBuffer = malloc(1024);
	char* printf_buffer = malloc(2048);
	int pos = 0;
	int free_mem = 0;

	sprintf(printf_buffer + pos, "\n=====================================================\n");
	pos = strlen(printf_buffer);
	sprintf(printf_buffer + pos, "name \t\tstatus \tprio \tfree \tpid\n");
	pos = strlen(printf_buffer);
	vTaskList((char *)pcWriteBuffer);
	sprintf(printf_buffer + pos, "%s", pcWriteBuffer);
	pos = strlen(printf_buffer);
	sprintf(printf_buffer + pos, "-----------------------------------------------------\n");
	pos = strlen(printf_buffer);
	sprintf(printf_buffer + pos, "name \t\trun_count \tusage\n");
	pos = strlen(printf_buffer);
	vTaskGetRunTimeStats((char *)pcWriteBuffer);
	sprintf(printf_buffer + pos, "%s", pcWriteBuffer);
	pos = strlen(printf_buffer);
	sprintf(printf_buffer + pos, "-----------------------------------------------------\n");
	pos = strlen(printf_buffer);
	free_mem = esp_get_free_heap_size();
	sprintf(printf_buffer + pos, "memory free : \t%d byte\n", free_mem);
	pos = strlen(printf_buffer);
	sprintf(printf_buffer + pos, "=====================================================\n\n");
	pos = strlen(printf_buffer);

	if(pos < buf_len)
	{
		memcpy(buffer, printf_buffer, pos + 1);
	}
	else
	{
		LOG_E(TAG, "Buffer length is too short.");
	}

	free(pcWriteBuffer);
	free(printf_buffer);
}

/**
 * @brief 开发者模式tcp发送数据
 * @param data 		[待发送的数据]
 * @param length 	[待发送数据长度]
 * @return int 		[发送结果]
 */
int developer_tcp_send(uint8_t *data, uint32_t length)
{
	int ret;
	ret = send(client_fd, data, length, MSG_DONTWAIT);
	if(ret <= 0)
	{
		LOG_E(TAG, "Developer client send error : %d.", ret);
	}
	return ret;
}

static void ota_event_handler(uint32_t len,vesync_ota_status_t status)
{
	uint8_t resp[2];
    switch(status){
        case OTA_TIME_OUT:
                LOG_I(TAG, "OTA_TIME_OUT");
            break;
        case OTA_BUSY:
                LOG_I(TAG, "OTA_BUSY");
            break;
        case OTA_PROCESS:
                LOG_I(TAG, "OTA_PROCESS ...%d",len);
            break;
        case OTA_FAILED:
                LOG_I(TAG, "OTA_FAILED");
            break;
        case OTA_SUCCESS:
                LOG_I(TAG, "OTA_SUCCESS");
            break;
        default:
            break;
    }
	resp[0] = status;
	if(status != OTA_PROCESS){
		resp[1] = len;
	}else{
		resp[1] = 0;
	}
	developer_tcp_send(resp ,2);
}

/**
 * @brief 固件升级处理
 * @param json [接收到的json数据]
 * @return int [处理结果]
 */
static int cjson_handle_upgrade(cJSON *json)
{
	int ret = -1;
	cJSON *root = json;
	if(NULL != root)
	{
		cJSON *jsonCmd = cJSON_GetObjectItemCaseSensitive(root, "jsonCmd");
		if (jsonCmd != NULL)
		{
			cJSON *firmware = cJSON_GetObjectItemCaseSensitive(jsonCmd, "firmware");
			if (firmware != NULL)
			{
				ret = 0;
				char new_version[10];
				char upgrade_url[256];
				LOG_I(TAG, "upgrade test start!");
				cJSON* newVersion = cJSON_GetObjectItemCaseSensitive(firmware, "newVersion");
				if(cJSON_IsString(newVersion))
				{
					strcpy(new_version, newVersion->valuestring);
					LOG_I(TAG, "upgrade new_version %s",new_version);
				}
				cJSON* url = cJSON_GetObjectItemCaseSensitive(firmware, "url");
				if(cJSON_IsString(url))
				{
					uint8_t url_len;
					strcpy(upgrade_url, url->valuestring);
					url_len = strlen(url->valuestring);
					sprintf(&upgrade_url[url_len],"%s.V%s.bin",DEVICE_TYPE,new_version);
					LOG_I(TAG, "upgrade url %s",upgrade_url);
					vesync_ota_init(upgrade_url,ota_event_handler);
				}
			}
		}
	}
	return ret;
}

/**
 * @brief 输出任务管理指令处理
 * @param json [接收到的json数据]
 * @return int [处理结果]
 */
static int cjson_handle_task_manager(cJSON *json)
{
	int ret = -1;
	cJSON *root = json;
	if(NULL != root)
	{
		cJSON *jsonCmd = cJSON_GetObjectItemCaseSensitive(root, "jsonCmd");
		if (jsonCmd != NULL)
		{
			cJSON *taskManager = cJSON_GetObjectItemCaseSensitive(jsonCmd, "taskManager");
			if (taskManager != NULL)
			{
				ret = 0;
				LOG_I(TAG, "Send task manager.");
				char* send_buf = malloc(2048);
				printf_os_task_manager_to_buf(send_buf, 2048);
				developer_tcp_send((uint8_t*)send_buf, strlen(send_buf));
				free(send_buf);
			}
		}
	}
	return ret;
}

/**
 * @brief 开发者模式json数据处理
 * @param data [tcp原始数据]
 * @return int [处理结果，0为数据是json格式，-1为非json格式]
 */
int developer_cjson_handle(uint8_t *data)
{
	cJSON *root = cJSON_Parse((const char*)data);
	if(NULL == root)
	{
		LOG_E(TAG, "developer_cjson_handle：Parse cjson error !");
		return -1;
	}

	// LOG_I(TAG, "Recive json data...");
	// vesync_printf_cjson(root);					//json标准格式，带缩进
	if(0 != cjson_handle_upgrade(root))
		if(0 != cjson_handle_task_manager(root))
			LOG_E(TAG, "No developer command !");

	cJSON_Delete(root);							//务必记得释放资源！
	return 0;
}

/**
 * @brief 开发者模式线程
 * @param args [无]
 */
static void developer_tcp_server_thread(void *args)
{
	while(1)
	{
		socklen_t sockaddr_len = sizeof(server_addr);
		client_fd = accept(socket_fd, (struct sockaddr*)&server_addr, &sockaddr_len);

		if(client_fd < 0)
		{
			LOG_E(TAG, "TCP server accept error : %d.", client_fd);
			continue;
		}

		LOG_I(TAG, "Developer client connected.");

		uint8_t* recv_buf = malloc(2048);
		int recv_len;
		while(1)
		{
			recv_len = recv(client_fd, recv_buf, 2048, MSG_WAITALL);
			if(recv_len <= 0)
			{
				LOG_E(TAG, "Developer client recv error : %d", recv_len);
				break;
			}
			else
			{
				recv_buf[recv_len] = '\0';
				if(developer_cjson_handle(recv_buf) != 0)
					LOG_I(TAG, "Developer recv %d byte data : %s", recv_len, recv_buf);
			}
		}
		free(recv_buf);

		close(client_fd);
	}
	close(socket_fd);

	vTaskDelete(NULL);
}

/**
 * @brief 启动开发者模式
 * @return int [启动结果，0为成功]
 */
int vesync_developer_start(void)
{
	LOG_I(TAG, "Developer tcp server start.");

	int ret;
	int reuse = 1;

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_len = sizeof(server_addr);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(DEVELOPER_LISTEN_PORT);
	server_addr.sin_addr.s_addr = htonl(IPADDR_ANY);

	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_fd < 0)
	{
		LOG_E(TAG, "TCP server create failed.");
		return -1;
	}

	setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

	ret = bind(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
	if(ret < 0)
	{
		LOG_E(TAG, "TCP server bind failed.");
		close(socket_fd);
		return -1;
	}

	ret = listen(socket_fd, 2);
	if(ret < 0)
	{
		LOG_E(TAG, "TCP server listen failed.");
		close(socket_fd);
		return -1;
	}

	if(pdPASS != xTaskCreate(developer_tcp_server_thread,
	                         DEVELOPER_TASK_NAME,
	                         DEVELOPER_TASK_STACSIZE / sizeof(portSTACK_TYPE),
	                         NULL,
	                         DEVELOPER_TASK_PRIO,
	                         NULL))
	{
		LOG_E(TAG, "Cannot create developer tcp server thread.");
		close(socket_fd);
		return -1;
	}

	return 0;
}

/**
 * @brief 直接串口打印系统任务管理器
 */
void printf_os_task_manager(void)
{
	uint8_t* pcWriteBuffer = malloc(1024);
	int free_mem = 0;

	printf("\n=====================================================\n");
	printf("name \t\t\tstatus \tprio \tfree \tpid\n");
	vTaskList((char *)pcWriteBuffer);
	printf("%s", pcWriteBuffer);
	printf("-----------------------------------------------------\n");
	printf("name \t\t\trun_count \tusage\n");
	vTaskGetRunTimeStats((char *)pcWriteBuffer);
	printf("%s", pcWriteBuffer);
	printf("-----------------------------------------------------\n");
	 free_mem = esp_get_free_heap_size();
	 printf("memory free : \t%d byte\n", free_mem);
	printf("=====================================================\n\n");

	free(pcWriteBuffer);
}
