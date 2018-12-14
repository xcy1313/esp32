#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "vesync_main.h"


/**
 * @brief 
 */
void app_main()
{
    xTaskCreate(vesync_entry, "vesync_entry", 4096, NULL, 5, NULL);
}