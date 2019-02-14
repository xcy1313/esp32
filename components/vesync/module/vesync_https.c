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

#include "esp_event_loop.h"

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

/* Constants that aren't configurable in menuconfig */
#define WEB_SERVER "www.howsmyssl.com"
#define WEB_PORT "443"
#define WEB_URL "https://www.howsmyssl.com/a/check"

static const char *TAG = "vesync_https";

static const char *REQUEST = "POST " "%s" " HTTP/1.0\r\n"
                             "Host: %s \r\n"
                             "User-Agent: esp-idf esp32\r\n"
                             "Content-Type:application/json\r\n"
                             "Content-Length:%d\r\n"
                             "\r\n"
                             "%s";
    
static mbedtls_entropy_context 	s_entropy;
static mbedtls_ctr_drbg_context s_ctr_drbg;
static mbedtls_ssl_context 		s_ssl_context;
static mbedtls_x509_crt 		s_cacert;
static mbedtls_ssl_config 		s_ssl_conf;
static mbedtls_net_context 		s_server_fd;

/**
 * @brief 初始化https模块
 * @return int [初始化结果，0为成功]
 */
int vesync_init_https_module(const char * ca_cert)
{
    int ret;

    mbedtls_ssl_init(&s_ssl_context);
    mbedtls_x509_crt_init(&s_cacert);
    mbedtls_ctr_drbg_init(&s_ctr_drbg);
    mbedtls_ssl_config_init(&s_ssl_conf);
    mbedtls_entropy_init(&s_entropy);

    LOG_I(TAG, "Seeding the random number generator");
    if((ret = mbedtls_ctr_drbg_seed(&s_ctr_drbg, mbedtls_entropy_func, &s_entropy, NULL, 0)) != 0)
    {
        LOG_E(TAG, "mbedtls_ctr_drbg_seed returned %d", ret);
        return ret;
    }

    LOG_I(TAG, "Loading the CA root certificate...");
    ret = mbedtls_x509_crt_parse(&s_cacert, (unsigned char*)ca_cert, strlen(ca_cert) + 1);
    if(ret < 0)
    {
        LOG_E(TAG, "mbedtls_x509_crt_parse returned -0x%x\n\n", -ret);
        return ret;
    }

    // LOG_I(TAG, "Setting hostname for TLS session...");
    // /* Hostname set here should match CN in server certificate */
    // if((ret = mbedtls_ssl_set_hostname(&s_ssl_context, WEB_SERVER)) != 0)
    // {
    // 	LOG_E(TAG, "mbedtls_ssl_set_hostname returned -0x%x", -ret);
    // 	return ret;
    // }

    LOG_I(TAG, "Setting up the SSL/TLS structure...");
    if((ret = mbedtls_ssl_config_defaults(&s_ssl_conf,
                                          MBEDTLS_SSL_IS_CLIENT,
                                          MBEDTLS_SSL_TRANSPORT_STREAM,
                                          MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
    {
        LOG_E(TAG, "mbedtls_ssl_config_defaults returned %d", ret);
        return ret;
    }

    /* MBEDTLS_SSL_VERIFY_OPTIONAL is bad for security, in this example it will print
       a warning if CA verification fails but it will continue to connect.
       You should consider using MBEDTLS_SSL_VERIFY_REQUIRED in your own code.
    */
    mbedtls_ssl_conf_authmode(&s_ssl_conf, MBEDTLS_SSL_VERIFY_REQUIRED );    //MBEDTLS_SSL_VERIFY_OPTIONAL
    mbedtls_ssl_conf_ca_chain(&s_ssl_conf, &s_cacert, NULL);
    mbedtls_ssl_conf_rng(&s_ssl_conf, mbedtls_ctr_drbg_random, &s_ctr_drbg);
#ifdef CONFIG_MBEDTLS_DEBUG
    mbedtls_esp_enable_debug_log(&s_ssl_conf, 4);
#endif

    if((ret = mbedtls_ssl_setup(&s_ssl_context, &s_ssl_conf)) != 0)
    {
        LOG_E(TAG, "mbedtls_ssl_setup returned -0x%x\n\n", -ret);
        mbedtls_ssl_session_reset(&s_ssl_context);
        return ret;
    }

    return 0;
}

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
    char https_buffer[9000];
    int ret, flags, len;

    LOG_D(TAG, "Waiting for network connected...");
    if(vesync_wait_network_connected(wait_time_ms) != 0)
    {
        LOG_E(TAG, "Network was disconnect !");
        return -1;
    }
    LOG_D(TAG, "Network connected.");

    mbedtls_net_init(&s_server_fd);

    LOG_D(TAG, "Connecting to %s:%s...", server_addr, port);
    if((ret = mbedtls_net_connect(&s_server_fd, server_addr, port, MBEDTLS_NET_PROTO_TCP)) != 0)
    {
        LOG_E(TAG, "mbedtls_net_connect returned -0x%x", -ret);
        goto exit;
    }
    LOG_I(TAG, "Connected.");

    mbedtls_ssl_set_bio(&s_ssl_context, &s_server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

    LOG_D(TAG, "Performing the SSL/TLS handshake...");
    while((ret = mbedtls_ssl_handshake(&s_ssl_context)) != 0)
    {
        if(ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
        {
            LOG_E(TAG, "mbedtls_ssl_handshake returned -0x%x", -ret);
            goto exit;
        }
    }

    LOG_D(TAG, "Verifying peer X.509 certificate...");
    if((flags = mbedtls_ssl_get_verify_result(&s_ssl_context)) != 0)
    {
        /* In real life, we probably want to close connection if ret != 0 */
        LOG_W(TAG, "Failed to verify peer certificate!");
        bzero(https_buffer, sizeof(https_buffer));
        mbedtls_x509_crt_verify_info(https_buffer, sizeof(https_buffer), "  ! ", flags);
        LOG_W(TAG, "verification info: %s", https_buffer);
    }
    else
    {
        LOG_I(TAG, "Certificate verified.");
    }

    LOG_D(TAG, "Cipher suite is %s", mbedtls_ssl_get_ciphersuite(&s_ssl_context));


    sprintf(https_buffer, REQUEST, url, server_addr,strlen(send_body),send_body);
    size_t written_bytes = 0;
    do
    {
        ret = mbedtls_ssl_write(&s_ssl_context,
                                (const unsigned char *)https_buffer + written_bytes,
                                strlen(https_buffer) - written_bytes);
        if(ret >= 0)
        {
            LOG_D(TAG, "%d bytes written", ret);
            written_bytes += ret;
        }
        else if(ret != MBEDTLS_ERR_SSL_WANT_WRITE && ret != MBEDTLS_ERR_SSL_WANT_READ)
        {
            LOG_E(TAG, "mbedtls_ssl_write returned -0x%x", -ret);
            goto exit;
        }
        LOG_I(TAG, "Writing HTTP request len [%d],content is [%s]",strlen(https_buffer),https_buffer);
    } while(written_bytes < strlen(https_buffer));

    LOG_I(TAG, "Reading HTTP response...");
    do
    {
        len = sizeof(https_buffer) - 1;
        bzero(https_buffer, sizeof(https_buffer));
        ret = mbedtls_ssl_read(&s_ssl_context, (unsigned char *)https_buffer, len);
        LOG_I(TAG, "mbedtls_ssl_read result :%d" ,ret);

        if(ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE)
            continue;

        if(ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY)
        {
            ret = 0;
            break;
        }

        if(ret < 0)
        {
            LOG_E(TAG, "mbedtls_ssl_read returned -0x%x", -ret);
            break;
        }

        if(ret == 0)
        {
            LOG_I(TAG, "connection closed");
            break;
        }

        len = ret;
        LOG_I(TAG, "%d bytes read", len);
        for(int i = 0; i < len; i++) {
                putchar(https_buffer[i]);
        }
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

    mbedtls_ssl_close_notify(&s_ssl_context);

exit:
    mbedtls_ssl_session_reset(&s_ssl_context);
    mbedtls_net_free(&s_server_fd);

    if(ret != 0)
    {
        mbedtls_strerror(ret, https_buffer, 100);
        LOG_E(TAG, "Last error was: -0x%x - %s", -ret, https_buffer);
        return ret;
    }

    return 0;
}
