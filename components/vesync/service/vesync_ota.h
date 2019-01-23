/**
 * @file vesync_ota.h
 * @brief 
 * @author Jiven
 * @date 2019-01-19
 */
#ifndef _VESYNC_OTA_H
#define _VESYNC_OTA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"

#include "etekcity_comm.h"
#include "vesync_wifi.h"

typedef enum{
    OTA_IDLE,
    OTA_BUSY,
    OTA_TIME_OUT,
    OTA_FLASH_ERROR,
    OTA_FAILED,
    OTA_URL_ERROR,
    OTA_PROCESS,
    OTA_SUCCESS
}vesync_ota_status_t;

typedef void(*vesync_ota_event_cb_t)(uint32_t len,vesync_ota_status_t status);

vesync_ota_status_t vesync_ota_init(char *url,vesync_ota_event_cb_t cb);

#endif
