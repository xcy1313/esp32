/**
 * @file main.c
 * @brief ESP32 FreeRTOS应用程序入口
 * @author WatWu
 * @date 2018-12-28
 */

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "vesync_main.h"
#include "vesync_uart.h"
#include "nvs_flash.h"

#include "esp_log.h"

static const char *TAG = "APP_MAIN";

/**
 * @brief ESP32 FreeRTOS应用程序入口函数
 */
void app_main()
{
    esp_err_t ret;
    /* Initialize NVS. */
    ret = nvs_flash_init();
    if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_LOGW(TAG, "nvs_flash_init failed (0x%x), erasing partition and retrying", ret);
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    xTaskCreate(vesync_entry, "vesync_entry", 4096, NULL, 5, NULL);
    xTaskCreate(vesync_user_entry, "vesync_user_entry", 4096, NULL, 5, NULL);
}
