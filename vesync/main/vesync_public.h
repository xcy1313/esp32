
#ifndef _VESYNC_PUBLIC_H
#define _VESYNC_PUBLIC_H

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "vesync_bt.h"
#include "vesync_uart.h"
#include "vesync_wifi.h"

extern EventGroupHandle_t user_event_group;

extern const int WIFI_CONNECTED_BIT;
extern const int OTA_BIT;
extern const int BT_CONNECTED_BIT ;
extern const int BT_DIS_CONNECTED_BIT;
extern const int DISCONNECTED_BIT;

#define  WAKE_UP_PIN    25


void vesync_power_save_enter(uint32_t pin);

void vesync_public_init(void);

#endif 

