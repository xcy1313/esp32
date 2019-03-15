/**
 * @file vesync_https.c
 * @brief vesync平台客户端https接口
 * @author WatWu
 * @date 2018-12-10
 */

#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_task_wdt.h"

#include "esp_event_loop.h"
#include "esp_tls.h"

#include <netdb.h>
#include <sys/socket.h>

#include "mbedtls/platform.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/esp_debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"

#include "vesync_log.h"
#include "vesync_wifi.h"
#include "vesync_ca_cert.h"

#define HTTPS_BUFFER_SIZE                   (6 * 1024)          //https通信的数据交互缓存区大小

static const char *TAG = "vesync_https";

static const char *REQUEST = "POST " "%s" " HTTP/1.0\r\n"
                             "Host: %s \r\n"
                             "User-Agent: esp-idf esp32\r\n"
                             "Content-Type:application/json\r\n"
                             "Content-Length:%d\r\n"
                             "\r\n"
                             "%s";

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
int vesync_https_request(char *server_addr, char *port, char *url, char *send_body, char *recv_buff, int *recv_len, int wait_time_ms)
{
    LOG_D(TAG, "Waiting for network connected...");
    if(vesync_wait_network_connected(wait_time_ms) != 0)
    {
        LOG_E(TAG, "Network was disconnect !");
        return -1;
    }
    LOG_D(TAG, "Network connected.");

	esp_tls_cfg_t cfg =
	{
		.cacert_pem_buf = (unsigned char*)vesync_https_ca_cert_pem_start,
		.cacert_pem_bytes = vesync_https_ca_cert_pem_end - vesync_https_ca_cert_pem_start,
	};
    int ret = -1, len = 0;

    int server_port = atoi(port);
    esp_tls_t *tls = esp_tls_conn_new(server_addr, strlen(server_addr), server_port, &cfg);

    if(tls != NULL)
    {
        LOG_I(TAG, "Connection established...");
    }
    else
    {
        LOG_E(TAG, "Connection failed...");
        goto exit;
    }

    char *https_buffer = NULL;
    https_buffer = malloc(HTTPS_BUFFER_SIZE);
    if(NULL != https_buffer)
    {
        sprintf(https_buffer, REQUEST, url, server_addr,strlen(send_body),send_body);

        size_t written_bytes = 0;
        do
        {
            ret = esp_tls_conn_write(tls,
                                    https_buffer + written_bytes,
                                    strlen(https_buffer) - written_bytes);
            if(ret >= 0)
            {
                LOG_I(TAG, "%d bytes written", ret);
                written_bytes += ret;
            }
            else if(ret != MBEDTLS_ERR_SSL_WANT_READ  && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
            {
                LOG_E(TAG, "esp_tls_conn_write  returned 0x%x", ret);
                goto exit;
            }
        } while(written_bytes < strlen(https_buffer));

        LOG_I(TAG, "Reading HTTP response...");

        do
        {
            len = HTTPS_BUFFER_SIZE - 1;
            bzero(https_buffer, HTTPS_BUFFER_SIZE);
            ret = esp_tls_conn_read(tls, (char *)https_buffer, len);

            if(ret == MBEDTLS_ERR_SSL_WANT_WRITE  || ret == MBEDTLS_ERR_SSL_WANT_READ)
                continue;

            if(ret < 0)
            {
                LOG_E(TAG, "esp_tls_conn_read  returned -0x%x", -ret);
                break;
            }

            if(ret == 0)
            {
                LOG_I(TAG, "connection closed");
                break;
            }

            len = ret;
            LOG_I(TAG, "%d bytes read", len);
            // for(int i = 0; i < len; i++) {
            //         putchar(https_buffer[i]);
            // }
            if(len < *recv_len)
            {
                memcpy(recv_buff, https_buffer, len);
                recv_buff[len] = '\0';
                *recv_len = len;
            }
            else
            {
                LOG_E(TAG, "Recv buffer length is too short !");
                *recv_len = -1;
            }
        } while(1);
    }
    else
    {
        LOG_E(TAG, "Https buffer is null !");
    }

    if(NULL != https_buffer)
        free(https_buffer);

exit:
    esp_tls_conn_delete(tls);
    putchar('\n'); // JSON output doesn't have a newline at end
    return ret;
}
