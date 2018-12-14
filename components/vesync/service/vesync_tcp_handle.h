/**
 * @file vesync_tcp_handle.h
 * @brief 本地TCP服务器的通信及数据交互处理，用于配网时与APP进行交互
 * @author WatWu
 * @date 2018-11-17
 */

#ifndef VESYNC_TCP_HANDLE_H
#define VESYNC_TCP_HANDLE_H

/**
 * @brief 发送TCP消息数据到APP端
 * @param msg 		[原始数据]
 * @param msg_len 	[数据长度]
 * @return int 		[发送结果]
 */
int vesync_send_tcp_msg_to_app(uint8_t *msg, uint32_t msg_len);

/**
 * [vesync_tcpserver_recv_callback  TCP服务器数据接收回调函数]
 * @param  clinet_fd	[TCP连接句柄]
 * @param  recvdata 	[TCP数据包]
 * @param  length   	[数据包长度]
 * @return          	[无]
 */
void vesync_tcpserver_recv_callback(int client_fd, char *recvdata, unsigned short length);

#endif
