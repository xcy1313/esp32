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

#include "vesync_log.h"
#include "vesync_task_def.h"

//开发者模式的TCP服务器监听端口
#define DEVELOPER_LISTEN_PORT			55555

static const char* TAG = "vesync_developer";

static struct sockaddr_in server_addr;
static int socket_fd;

/**
 * @brief 打印系统任务管理器到缓存数组
 * @param buffer 	[缓存数组]
 * @param buf_len 	[缓存数组长度]
 */
static void printf_os_task_manager_to_buf(char *buffer, int buf_len)
{
	uint8_t pcWriteBuffer[1024];
	char printf_buffer[2048];
	int pos = 0;
	int free_mem = 0;

	sprintf(printf_buffer + pos, "\n=====================================================\n");
	pos = strlen(printf_buffer);
	sprintf(printf_buffer + pos, "name \t\tstatus \tprio \tfree \tpid\n");
	pos = strlen(printf_buffer);
	vTaskList((char *)&pcWriteBuffer);
	sprintf(printf_buffer + pos, "%s", pcWriteBuffer);
	pos = strlen(printf_buffer);
	sprintf(printf_buffer + pos, "-----------------------------------------------------\n");
	pos = strlen(printf_buffer);
	sprintf(printf_buffer + pos, "name \t\trun_count \tusage\n");
	pos = strlen(printf_buffer);
	vTaskGetRunTimeStats((char *)&pcWriteBuffer);
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
}

/**
 * @brief 开发者模式线程
 * @param args [无]
 */
static void developer_tcp_server_thread(void *args)
{
	while(1)
	{
		int client_fd;

		socklen_t sockaddr_len = sizeof(server_addr);
		client_fd = accept(socket_fd, (struct sockaddr*)&server_addr, &sockaddr_len);

		if(client_fd < 0)
		{
			LOG_E(TAG, "TCP server accept error : %d.", client_fd);
			continue;
		}

		LOG_I(TAG, "Developer client connected.");

		while(1)
		{
			char send_buf[2048];
			int send_len;
			printf_os_task_manager_to_buf(send_buf, sizeof(send_buf));
			send_len = send(client_fd, send_buf, strlen(send_buf), MSG_DONTWAIT);
			if(send_len <= 0)
			{
				LOG_E(TAG, "Developer client send error : %d.", send_len);
				break;
			}

			sleep(1);
		}

		close(client_fd);
	}
	close(socket_fd);

	while(1)
	{
		sleep(10);
	}
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
	uint8_t pcWriteBuffer[512];
	int free_mem = 0;

	// printf("\n=====================================================\n");
	// printf("name \t\t\tstatus \tprio \tfree \tpid\n");
	// vTaskList((char *)&pcWriteBuffer);
	// printf("%s", pcWriteBuffer);
	// printf("-----------------------------------------------------\n");
	// printf("name \t\t\trun_count \tusage\n");
	// vTaskGetRunTimeStats((char *)&pcWriteBuffer);
	// printf("%s", pcWriteBuffer);
	// printf("-----------------------------------------------------\n");
	 free_mem = esp_get_free_heap_size();
	 printf("memory free : \t%d byte\n", free_mem);
	// printf("=====================================================\n\n");
}
