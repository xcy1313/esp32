
/**
 * @file vesync_mqtt_client.h
 * @brief 
 * @author Jiven 
 * @date 2018-11-19
 */
#ifndef _VESYNC_MQTT_CLIENT_H
#define _VESYNC_MQTT_CLIENT_H
#include <stdint.h>

typedef enum{
    MQTT_ERROR  = -1,
    MQTT_CONNECTED = 0,
    MQTT_DISCONNECTED = 1,
}vesync_mqtt_client_state_t;

void vesync_mqtt_client_init(void);
uint8_t vesync_mqtt_client_is_connected(void);

#endif 

