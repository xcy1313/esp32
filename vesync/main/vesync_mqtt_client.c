/**
 * @file vesync_mqtt_client.c
 * @brief 
 * @author Jiven 
 * @date 2018-11-19
 */
#include "vesync_mqtt_client.h"
#include <stdint.h>
#include "mqtt_client.h"
#include "esp_log.h"
#include "cJSON.h"

#include "vesync_public.h"

static const char *TAG = "VESYNC_MQTT";

esp_mqtt_client_handle_t my_mqtt_client;
static vesync_mqtt_client_state_t vesync_mqtt_client_state = MQTT_DISCONNECTED;

/**
 * @brief 
 * @param event 
 * @return esp_err_t 
 */
static esp_err_t vesync_mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch (event->event_id){
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            vesync_mqtt_client_state = MQTT_CONNECTED;
            msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

            msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

            msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
            ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            vesync_mqtt_client_state = MQTT_DISCONNECTED;
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
    }
    return ESP_OK;
}

/**
 * @brief 
 * @return uint8_t 返回MQTT状态
 */
uint8_t vesync_mqtt_client_is_connected(void)
{
    ESP_LOGI(TAG, "vesync_mqtt_client_is_connected :%d",vesync_mqtt_client_state);
    return vesync_mqtt_client_state;
}

void vesync_mqtt_client_init(void)
{
    const esp_mqtt_client_config_t mqtt_cfg = {
        .event_handle = vesync_mqtt_event_handler,
        .cert_pem = NULL,
        .host = "192.168.16.25",
        //.port = "61613",
        .client_id = "64464d9993b347888adb3bef18c10404",
        .username = "etekcity",
        .password = "hardware",
    };

    my_mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(my_mqtt_client);
}

