/**
 * @file vesync_tcp_server.h
 * @brief 本地TCP服务器
 * @author WatWu
 * @date 2018-11-17
 */

#ifndef VESYNC_TCP_SERVER_H
#define VESYNC_TCP_SERVER_H

/**
 * @brief vesync TCP服务器数据接收回调函数指针
 * @param client_fd	[客户端socket文件描述符]
 * @param recvdata 	[接收到的tcp数据]
 * @param length 	[接收到的tcp数据长度]
 */
typedef void (*vesync_tcpserver_recv_cb)(int client_fd, char *recvdata, unsigned short length);

/**
 * @brief 启动TCP服务监听APP的配网注册信息
 * @param port 			[TCP监听端口]
 * @param recv_callback [TCP数据接收回调函数]
 * @return int 			[启动结果，0为成功]
 */
int vesync_tcp_server_start(uint32_t port, vesync_tcpserver_recv_cb recv_callback);

/**
 * [vesync_tcp_server_send  TCP发送数据]
 * @param  buf    [待发送数据]
 * @param  length [待发送数据长度]
 * @return        [发送结果]
 */
int vesync_tcp_server_send(const uint8_t *buf, uint32_t length);

/**
 * @brief 关闭配网TCP服务器
 */
void vesync_tcp_server_stop(void);

#endif
