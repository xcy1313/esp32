#pragma once

#include "blufi_security.h"

#define ENABLE_BLUFI_ADVERTISE

typedef struct{
    uint8_t       sta_bssid[6];
    uint8_t       sta_ssid[32];
    int           sta_ssid_len;
    wifi_mode_t   mode;
    wifi_config_t sta_config;
    wifi_config_t ap_config;
}vesync_blufi_param_t;
extern vesync_blufi_param_t vesync_blufi_param;




void vesync_blufi_init(void);
