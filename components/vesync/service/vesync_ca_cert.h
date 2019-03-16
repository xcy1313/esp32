/**
 * @file vesync_ca_cert.h
 * @brief vesync平台证书文件
 * @author WatWu
 * @date 2018-11-22
 */

#ifndef VESYNC_CA_CERT_H
#define VESYNC_CA_CERT_H

//vesync平台证书文件数组
extern const char vesync_mqtt_ca_cert_pem_start[] 			asm("_binary_mqtt_ca_crt_start");
extern const char vesync_mqtt_ca_cert_pem_end[] 			asm("_binary_mqtt_ca_crt_end");

extern const char vesync_mqtt_client_cert_pem_start[] 		asm("_binary_mqtt_client_crt_start");
extern const char vesync_mqtt_client_cert_pem_end[] 		asm("_binary_mqtt_client_crt_end");

extern const char vesync_mqtt_client_key_pem_start[] 		asm("_binary_mqtt_client_key_start");
extern const char vesync_mqtt_client_key_pem_end[] 			asm("_binary_mqtt_client_key_end");

extern const char vesync_https_ca_cert_pem_start[] 			asm("_binary_https_ca_crt_start");
extern const char vesync_https_ca_cert_pem_end[] 			asm("_binary_https_ca_crt_end");

// extern const char vesync_https_client_cert_pem_start[] 		asm("_binary_https_client_crt_start");
// extern const char vesync_https_client_cert_pem_end[] 		asm("_binary_https_client_crt_end");

// extern const char vesync_https_client_key_pem_start[] 		asm("_binary_https_client_key_start");
// extern const char vesync_https_client_key_pem_end[] 		asm("_binary_https_client_key_end");

#endif
