/**
 * @file vesync_https.h
 * @brief vesync平台客户端https接口
 * @author WatWu
 * @date 2018-12-11
 */

#ifndef VESYNC_HTTPS_H
#define VESYNC_HTTPS_H

/**
 * @brief 初始化https模块
 * @return int [初始化结果，0为成功]
 */
int vesync_init_https_module(const char * ca_cert);

/**
 * @brief 发起https请求
 * @param server_addr 	[服务器地址]
 * @param port 			[服务器端口]
 * @param url 			[请求的url]
 * @param send_body 	[请求的url数据体]
 * @param recv_buff 	[返回的数据内容缓存buffer]
 * @param recv_len 		[返回的数据内容长度指针，传入时为缓存buffer的长度，供内部判断buffer大小是否足够，足够时内部把返回的数据拷贝至buffer，并赋值该值为数据长度]
 * @param wait_time_ms 	[超时等待时间]
 * @return int 			[请求结果]
 */
int vesync_https_request(char *server_addr, char *port, char *url, char *send_body, char *recv_buff, int *recv_len, int wait_time_ms);

#endif
