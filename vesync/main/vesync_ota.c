/**
 * @file vesync_ota.c
 * @brief 
 * @author Jiven 
 * @date 2018-11-19
 */
#include "vesync_ota.h"
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


#define SERVER_URL "http://192.168.16.25:8888/firmware-debug/esp32/vesync_sdk_esp32.bin"
#define BUFFSIZE 1024
#define HASH_LEN 32 /* SHA-256 digest length */

static const char *TAG = "vesync_OTA";
/*an ota data write buffer ready to write to the flash*/
static char ota_write_data[BUFFSIZE + 1] = { 0 };
const uint8_t server_cert_pem_start[] asm("_binary_ca_cert_pem_start");
const uint8_t server_cert_pem_end[] asm("_binary_ca_cert_pem_end");

static vesync_ota_event_cb_t vesync_ota_status_handler_cb = NULL;
static bool vesync_ota_init_flag = false;

static void http_cleanup(esp_http_client_handle_t client)
{
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
}

static void __attribute__((noreturn)) task_fatal_error()
{
    ESP_LOGE(TAG, "Exiting task due to fatal error...");
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
    return OTA_IDLE;
}

static void vesync_ota_task_handler(void *pvParameters)
{
    esp_err_t err;
    esp_ota_handle_t update_handle = 0 ;
    const esp_partition_t *update_partition = NULL;

    ESP_LOGI(TAG, "Starting OTA ...");

    const esp_partition_t *configured = esp_ota_get_boot_partition();   //查看boot image数据区内容
    const esp_partition_t *running = esp_ota_get_running_partition();

    if (configured != running) {
        ESP_LOGW(TAG, "Configured OTA boot partition at offset 0x%08x, but running from offset 0x%08x",
                 configured->address, running->address);
        ESP_LOGW(TAG, "(This can happen if either the OTA boot data or preferred boot image become corrupted somehow.)");
    }
    ESP_LOGI(TAG, "Running partition type %d subtype %d (offset 0x%08x)",
             running->type, running->subtype, running->address);

    xEventGroupWaitBits(user_event_group, WIFI_CONNECTED_BIT|OTA_BIT,
                        false, true, portMAX_DELAY);    //第二个false 不用等待所有的bit都要置位;
    ESP_LOGI(TAG, "Connect to Wifi ! Start to Connect to Server....");

    esp_http_client_config_t config = {
        .url = SERVER_URL,
        .cert_pem = (char *)server_cert_pem_start,
    };
    
    esp_http_client_handle_t client = esp_http_client_init(&config);
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

    update_partition = esp_ota_get_next_update_partition(NULL);
    ESP_LOGI(TAG, "Writing to partition subtype %d at offset 0x%x",
             update_partition->subtype, update_partition->address);
    assert(update_partition != NULL);

    err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_begin failed (%s)", esp_err_to_name(err));
        http_cleanup(client);
        task_fatal_error();
    }
    vesync_ota_event_post_to_user(OTA_BUSY);
    ESP_LOGI(TAG, "esp_ota_begin succeeded");
    
    int binary_file_length = 0;
    /*deal with all receive packet*/
    while (1) {
        int data_read = esp_http_client_read(client, ota_write_data, BUFFSIZE);
        if (data_read < 0) {
            ESP_LOGE(TAG, "Error: SSL data read error");
            vesync_ota_event_post_to_user(OTA_FAILED);
            http_cleanup(client);
            task_fatal_error();
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
            vesync_ota_event_post_to_user(OTA_SUCCESS);
            break;
        }
    }
    ESP_LOGI(TAG, "Total Write binary data length : %d", binary_file_length);

    if (esp_ota_end(update_handle) != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_end failed!");
        vesync_ota_event_post_to_user(OTA_TIME_OUT);
        http_cleanup(client);
        task_fatal_error();
    }

    if (esp_partition_check_identity(esp_ota_get_running_partition(), update_partition) == true) {
        ESP_LOGI(TAG, "The current running firmware is same as the firmware just downloaded");
        int i = 0;
        ESP_LOGI(TAG, "When a new firmware is available on the server, press the reset button to download it");
        while(1) {
            ESP_LOGI(TAG, "Waiting for a new firmware ... %d", ++i);
            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }
    }

    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));
        http_cleanup(client);
        task_fatal_error();
    }

    vesync_ota_init_flag = false;

    ESP_LOGI(TAG, "Prepare to restart system!");
    esp_restart();
    return ;

}

vesync_ota_status_t vesync_ota_init(vesync_ota_event_cb_t cb)
{
    uint8_t sha_256[HASH_LEN] = { 0 };
    esp_partition_t partition;
    
    if (vesync_ota_init_flag) {
        return OTA_FAILED;
    }

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
    xTaskCreate(vesync_ota_task_handler, "vesync_ota_task_handler", 2048, NULL, 5, NULL);
    
    return OTA_IDLE;
}
