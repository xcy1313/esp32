/**
 * @file vesync_ota.c
 * @brief 
 * @author Jiven 
 * @date 2018-11-19
 */
#include "vesync_ota.h"
#include "vesync_wifi.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_flash_partitions.h"
#include "esp_partition.h"

#include "nvs.h"
#include "nvs_flash.h"

#define MAX_URL_LEN 128
#define BUFFSIZE 2048
#define HASH_LEN 32 

static const char *TAG = "vesync_OTA";

static vesync_ota_event_cb_t vesync_ota_status_handler_cb = NULL;

static void http_cleanup(esp_http_client_handle_t client)
{
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
}

static void __attribute__((noreturn)) task_fatal_error()
{
    ESP_LOGE(TAG, "Exiting task due to fatal error...");
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    esp_restart();
    (void)vTaskDelete(NULL);

    while (1) {
        ;
    }
}

void print_sha256 (const uint8_t *image_hash, const char *label)
{
    char hash_print[HASH_LEN * 2 + 1];
    hash_print[HASH_LEN * 2] = 0;
    for (int i = 0; i < HASH_LEN; ++i) {
        sprintf(&hash_print[i * 2], "%02x", image_hash[i]);
    }
    ESP_LOGI(TAG, "%s: %s", label, hash_print);
}

static void vesync_ota_event_post_to_user(vesync_ota_status_t status)
{
    if (vesync_ota_status_handler_cb) {
        return (*vesync_ota_status_handler_cb)(status);
    }
}

static void vesync_ota_task_handler(void *pvParameters)
{
    esp_err_t err;
    esp_ota_handle_t update_handle = 0 ;
    static uint8_t time_out =0;
    const esp_partition_t *update_partition = NULL;

    esp_http_client_config_t client_config ={0};
    client_config.url = malloc(MAX_URL_LEN);
    
    strcpy(client_config.url,(char *)pvParameters);
    ESP_LOGI(TAG, "remote url is %s" ,client_config.url);

    char *ota_write_data = (char *)malloc(BUFFSIZE);
    memset(ota_write_data ,NULL,BUFFSIZE);

    const esp_partition_t *configured = esp_ota_get_boot_partition();
    ESP_LOGI(TAG, "configured partition type %d subtype %d (offset 0x%08x)",
             configured->type, configured->subtype, configured->address);

    switch(configured->subtype){
        case ESP_PARTITION_SUBTYPE_APP_FACTORY:
                update_partition = esp_partition_find_first(ESP_PARTITION_TYPE_APP,ESP_PARTITION_SUBTYPE_APP_OTA_0, NULL);
                if(update_partition == NULL){
                    ESP_LOGE(TAG, "Factory update_partition NULL");
                    update_partition = esp_ota_get_next_update_partition (NULL);
                }
                ESP_LOGI(TAG, "ESP_PARTITION_SUBTYPE_APP_FACTORY");
            break;
        case ESP_PARTITION_SUBTYPE_APP_OTA_0:
                update_partition = esp_partition_find_first(ESP_PARTITION_TYPE_APP ,ESP_PARTITION_SUBTYPE_APP_FACTORY ,NULL);
                if(update_partition == NULL){
                    ESP_LOGE(TAG, "OTA_0 update_partition NULL");
                    update_partition = esp_ota_get_next_update_partition (NULL);
                }
                ESP_LOGI(TAG, "ESP_PARTITION_SUBTYPE_APP_OTA_0");
            break;
        default:
            break;
    }
    ESP_LOGI(TAG, "Writing to partition subtype %d at offset 0x%x",update_partition->subtype, update_partition->address);

    vesync_wait_network_connected(5000);

    esp_http_client_handle_t client = esp_http_client_init(&client_config);
    if (client == NULL) {
        ESP_LOGE(TAG, "Failed to initialise HTTP connection");
        task_fatal_error();
    }
    err = esp_http_client_open(client, 0);
    if (err != ESP_OK) {
        vesync_ota_event_post_to_user(OTA_FAILED);
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
        
        esp_http_client_cleanup(client);
        task_fatal_error();
    }
    esp_http_client_fetch_headers(client);

    if(err != ESP_OK){
        vesync_ota_event_post_to_user(OTA_FAILED);
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
        
        esp_http_client_cleanup(client);
        task_fatal_error();
    }

    err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
    if ((err != ESP_OK) && (err != ESP_ERR_INVALID_ARG)){
        ESP_LOGE(TAG, "esp_ota_begin failed (%s)", esp_err_to_name(err));

        vesync_ota_event_post_to_user(OTA_FAILED);
        http_cleanup(client);
        task_fatal_error();
    }else{
    	  vesync_ota_event_post_to_user(OTA_BUSY);
    }
    
    int binary_file_length = 0;
    /*deal with all receive packet*/
    while (1) {
        int data_read = esp_http_client_read(client, ota_write_data, BUFFSIZE);
        if (data_read < 0) {
            time_out++;
            if(time_out >=3){
                time_out = 0; 
                vesync_ota_event_post_to_user(OTA_URL_ERROR);
                http_cleanup(client);
                task_fatal_error();
            }
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            ESP_LOGE(TAG, "Error: SSL data read error");
        } else if (data_read > 0) {
            err = esp_ota_write( update_handle, (const void *)ota_write_data, data_read);
            if (err != ESP_OK) {
                vesync_ota_event_post_to_user(OTA_FAILED);
                http_cleanup(client);
                task_fatal_error();
            }
            binary_file_length += data_read;
            ESP_LOGD(TAG, "Written image length %d", binary_file_length);
        } else if (data_read == 0) {
            ESP_LOGI(TAG, "Connection closed,all data received");
            break;
        }
    }
    ESP_LOGI(TAG, "Total Write binary data length : %d", binary_file_length);
    free(client_config.url);
    free(ota_write_data);

    if (esp_ota_end(update_handle) != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_end failed!");
        vesync_ota_event_post_to_user(OTA_FAILED);
        http_cleanup(client);
        task_fatal_error();
    }

    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));
        http_cleanup(client);
        task_fatal_error();
    }
    vesync_ota_event_post_to_user(OTA_SUCCESS);

    vTaskDelay(1000 / portTICK_PERIOD_MS);
    esp_restart();
    return ;
}

vesync_ota_status_t vesync_ota_init(char *url,vesync_ota_event_cb_t cb)
{
    uint8_t sha_256[HASH_LEN] = { 0 };
    esp_partition_t partition;
    
    char server_url[MAX_URL_LEN] ={'\0'};
    strcpy(server_url,url);
    ESP_LOGI(TAG, "copy url is %s" ,url);
    // get sha256 digest for the partition table
    partition.address   = ESP_PARTITION_TABLE_OFFSET;
    partition.size      = ESP_PARTITION_TABLE_MAX_LEN;
    partition.type      = ESP_PARTITION_TYPE_DATA;
    esp_partition_get_sha256(&partition, sha_256);
    print_sha256(sha_256, "SHA-256 for the partition table: ");

    // get sha256 digest for bootloader
    partition.address   = ESP_BOOTLOADER_OFFSET;
    partition.size      = ESP_PARTITION_TABLE_OFFSET;
    partition.type      = ESP_PARTITION_TYPE_APP;
    esp_partition_get_sha256(&partition, sha_256);
    print_sha256(sha_256, "SHA-256 for bootloader: ");

    // get sha256 digest for running partition
    esp_partition_get_sha256(esp_ota_get_running_partition(), sha_256);
    print_sha256(sha_256, "SHA-256 for current firmware: ");

    vesync_ota_status_handler_cb = cb;
    xTaskCreate(vesync_ota_task_handler, "vesync_ota_task_handler", 8192, server_url, 13, NULL);
    
    return OTA_IDLE;
}
