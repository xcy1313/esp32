/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
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
    OTA_FAILED,
    OTA_URL_ERROR,
    OTA_SUCCESS
}vesync_ota_status_t;

typedef void(*vesync_ota_event_cb_t)(vesync_ota_status_t status);

vesync_ota_status_t vesync_ota_init(char *url,vesync_ota_event_cb_t cb);

#endif
