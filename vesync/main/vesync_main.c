/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_system.h"
#include "nvs_flash.h"

#include "vesync_ota.h"
#include "vesync_uart.h"
#include "vesync_bt.h"
#include "vesync_wifi.h"
#include "vesync_blufi.h"
#include "vesync_button.h"
#include "vesync_unixtime.h"

#include "esp_log.h"

static const char *TAG = "main";

uni_frame_t bt_frame;

/**
 * @brief 
 * @param p_event_data 
 */
void ui_event_handler(void *p_event_data){

    ESP_LOGI(TAG, "key [%d]\r\n" ,*(uint8_t *)p_event_data);
    switch(*(uint8_t *)p_event_data){
        case Short_key:
							
			return;
        case Double_key:
							
			return;
        case Reapet_key:
						
			return;
        case Very_Long_key:

			return;
        default:
        return;  
    }
}

/**
 * @brief 
 * @param status 
 */
static void ota_event_handler(vesync_ota_status_t status)
{
    ESP_LOGI(TAG, "ota status: %02x", status);
    switch(status){
        case OTA_TIME_OUT:
            break;
        case OTA_BUSY:
            break;
        case OTA_SUCCESS:
            xEventGroupClearBits(user_event_group, OTA_BIT);
            break;
        default:
            break;
    }
}

/**
 * @brief 
 * @param bt_buf 
 * @param length 
 */
void bt_event_handler(const void*bt_buf, int length){
    esp_log_buffer_hex(TAG, (char *)bt_buf, length);
    WriteCoreQueue((char *)bt_buf, length);

    for(int i=0;i<length;i++){
        if(Comm_frame_parse(*(unsigned char *)&bt_buf[i],1,&bt_frame) == 1){
            ESP_LOGI(TAG, "frame_data_len %d\r\n", bt_frame.frame_data_len);

            for(uint8_t i=0;i< bt_frame.frame_data_len;i++){
                printf("%02x ",bt_frame.frame_data[i]);
            }
            printf("\r\n ctl =%d ,len =%d\r\n ",bt_frame.frame_ctrl,bt_frame.frame_data_len);

            for(uint8_t j=0;j<sizeof(command_type)/sizeof(command_type[0]);j++){
                if(bt_frame.frame_ctrl == command_type[j].command){    // 蓝牙协议控制码即为指令;
                    uint8_t *opt = NULL;
                    opt = &bt_frame.frame_data[1];    //过滤命令id字节
                    switch(command_type[j].command){
                        case CMD_CREATE_USER:
                             memcpy((uint8_t *)&info_str.response_weight_data.weight,opt,bt_frame.frame_data_len-1);
                             ESP_LOGI(TAG, "CMD_CREATE_USER \r\n");
                            break;
                        case CMD_DELETE_USER:
                             ESP_LOGI(TAG, "CMD_CREATE_USER \r\n");
                            break;
                        case CMD_DELETE_HIS_ITEM:
                              ESP_LOGI(TAG, "CMD_DELETE_HIS_ITEM \r\n");
                            break;
                        case CMD_SYNC_UTC:
                              ESP_LOGI(TAG, "CMD_SYNC_UTC \r\n");
                            break;
                        case CMD_MODIFY_USER:
                              ESP_LOGI(TAG, "CMD_MODIFY_USER \r\n");
                            break;
                        case CMD_HISTORY_TOTALLEN:
                              ESP_LOGI(TAG, "CMD_HISTORY_TOTALLEN \r\n");
                            break;
                        case CMD_USER_AMOUT:
                              ESP_LOGI(TAG, "CMD_USER_AMOUT \r\n");
                            break;
                        default:
                              ESP_LOGI(TAG, "error cmd! \r\n");
                            break; 
                    }
                    if(command_type[j].record){
                        if(command_type[j].transfer_callback != NULL){
                            command_type[j].transfer_callback((uint8_t *)&bt_frame.frame_data[0] ,bt_frame.frame_data_len);  //应答app
                        }
                    }
                }
            }
        }
    }
}

/**
 * @brief 
 * @param uart_buf 
 * @param length 
 */
void  uart_event_handler(const void*uart_buf, int length){


}

/**
 * @brief 
 */
void app_main()
{
    esp_err_t ret;

    /* Initialize NVS. */
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    esp_read_mac(&info_str.macaddr[0], ESP_MAC_BT);
    ESP_LOGI(TAG, "BT mac:%02x%02x%02x%02x%02x%02x", info_str.macaddr[0][0],info_str.macaddr[0][1],info_str.macaddr[0][2],info_str.macaddr[0][3],info_str.macaddr[0][4],info_str.macaddr[0][5]);

    esp_read_mac(&info_str.macaddr[1], ESP_MAC_WIFI_STA);
    ESP_LOGI(TAG, "STATION mac:%02x%02x%02x%02x%02x%02x", info_str.macaddr[1][0],info_str.macaddr[1][1],info_str.macaddr[1][2],info_str.macaddr[1][3],info_str.macaddr[1][4],info_str.macaddr[1][5]);
    
    vesync_ota_init(ota_event_handler);
    vesync_wifi_init();
    vesync_bt_init(bt_event_handler);
    vesync_blufi_init();
    vesync_button_init(ui_event_handler);
    vesync_uart_int(uart_event_handler);
    printf("silicon revision %d, ", chip_info.revision);
#if 0
    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    for (int i = 10; i >= 0; i--) {
        printf("Restarting in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
#endif    
}
