/**
 * @file tcp_server.c
 * @brief 本地TCP服务器
 * @author WatWu
 * @date 2018-11-17
 */

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "lwip/sockets.h"
#include "lwip/sockets.h"
#include "netif/etharp.h"

#include "vesync_task_def.h"
#include "vesync_tcp_server.h"
#include "vesync_log.h"

static const char* TAG = "vesync_tcp_server";

static struct sockaddr_in server_addr;
static int tcp_socket_fd = -1;
static int vesync_tcp_client = -1;				//指向最新连接上的客户端
static vesync_tcpserver_recv_cb tcpserver_recv_callback = NULL;
static TaskHandle_t tcp_server_taskhd = NULL;

/**
 * @brief vesync的本地TCP服务器线程，阻塞式读取来自APP的配网注册信息
 * @param args [无]
 */
static void vesync_tcp_server_thread(void *args)
{
	while(1)
	{
		int recv_len;
		char recv_buf[512] = {0};

		socklen_t sockaddr_len = sizeof(server_addr);
		vesync_tcp_client = accept(tcp_socket_fd, (struct sockaddr*)&server_addr, &sockaddr_len);

		if(vesync_tcp_client < 0)
		{
			LOG_E(TAG, "Vesync tcp server accept error : %d.", vesync_tcp_client);
			continue;
		}

		LOG_I(TAG, "APP client connected.");

		// stop_device_find();
		// confignet_upload_history_log();					//发送配网历史记录

		while(1)
		{
			recv_len = recv(vesync_tcp_client, recv_buf, sizeof(recv_buf) - 1, 0);
			if(recv_len < 0)
			{
				LOG_E(TAG, "Tcp read error.");
				break;
			}
			if(recv_len == 0)
			{
				LOG_I(TAG, "Client disconnected.");
				break;
			}

			recv_buf[recv_len] = 0;
			// LOG_I(TAG, "Vesync tcp server received data : %s", recv_buf);

			if(NULL != tcpserver_recv_callback)
				tcpserver_recv_callback(vesync_tcp_client, recv_buf, recv_len);
		}

		close(vesync_tcp_client);
		vesync_tcp_client = -1;
	}
	close(tcp_socket_fd);
	tcp_socket_fd = -1;

	if(NULL != tcp_server_taskhd)
	{
		vTaskDelete(tcp_server_taskhd);
		tcp_server_taskhd = NULL;
	}
}

/**
 * [vesync_tcp_server_send  TCP发送数据]
 * @param  buf    [待发送数据]
 * @param  length [待发送数据长度]
 * @return        [发送结果]
 */
int vesync_tcp_server_send(const uint8_t *buf, uint32_t length)
{
	if(vesync_tcp_client < 0)
	{
		LOG_E(TAG, "Vesync tcp server not established or has been closed.");
		return -1;
	}

	int err = send(vesync_tcp_client, buf, length, MSG_DONTWAIT);
	if(err <= 0)
	{
		LOG_E(TAG, "Vesync tcp server send data failed, error code : %d", err);
	}

	return err;
}

/**
 * @brief tcp服务器端主动断开客户端连接
 */
static void vesync_disconnect_app_client(void)
{
	if(vesync_tcp_client > 0)
	{
		close(vesync_tcp_client);
		vesync_tcp_client = -1;
	}
}

/**
 * @brief 启动TCP服务监听APP的配网注册信息
 * @param port 			[TCP监听端口]
 * @param recv_callback [TCP数据接收回调]
 * @return int 			[启动结果，0为成功]
 */
int vesync_tcp_server_start(uint32_t port, vesync_tcpserver_recv_cb recv_callback)
{
	tcpserver_recv_callback = recv_callback;
	vesync_disconnect_app_client();

	if(tcp_socket_fd > 0)			//tcp socket已存在
		return 0;

	LOG_I(TAG, "Vesync tcp server starts.");

	int ret;
	int reuse = 1;

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_len = sizeof(server_addr);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = htonl(IPADDR_ANY);

	tcp_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(tcp_socket_fd < 0)
	{
		LOG_E(TAG, "TCP server create failed.");
		return -1;
	}

	setsockopt(tcp_socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

	ret = bind(tcp_socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
	if(ret < 0)
	{
		LOG_E(TAG, "TCP server bind failed.");
		close(tcp_socket_fd);
		tcp_socket_fd = -1;
		return -1;
	}

	ret = listen(tcp_socket_fd, 2);
	if(ret < 0)
	{
		LOG_E(TAG, "TCP server listen failed.");
		close(tcp_socket_fd);
		tcp_socket_fd = -1;
		return -1;
	}

	if(pdPASS != xTaskCreate(vesync_tcp_server_thread,
	                         TCP_SERVER_TASK_NAME,
	                         TCP_SERVER_TASK_STACSIZE / sizeof(portSTACK_TYPE),
	                         NULL,
	                         TCP_SERVER_TASK_PRIO,
	                         &tcp_server_taskhd))
	{
		LOG_E(TAG, "Cannot create vesync tcp server thread.");
		close(tcp_socket_fd);
		tcp_socket_fd = -1;
		return -1;
	}

	return 0;
}

/**
 * @brief 关闭配网TCP服务器
 */
void vesync_tcp_server_stop(void)
{
	vesync_disconnect_app_client();
	if(tcp_socket_fd > 0)
	{
		close(tcp_socket_fd);
		tcp_socket_fd = -1;
	}
	if(NULL != tcp_server_taskhd)
	{
		vTaskDelete(tcp_server_taskhd);
		tcp_server_taskhd = NULL;
	}
}
