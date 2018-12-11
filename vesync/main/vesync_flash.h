#ifndef _VESYNC_FLASH_H
#define _VESYNC_FLASH_H

#include "vesync_wifi.h"
#include "c_types.h"
#include <stdbool.h>


#define USER_DATA_LABEL         “userdata”








void vesync_flash_read(const char *label_name,const char *key_name,const void *data,uint16_t *len);
bool vesync_flash_write(const char *label_name,const char *key_name,const void *data,uint32_t len);
void vesync_flash_erase(const char *label_name,const char *key_name);
esp_err_t vesync_flash_write_info(device_info_t *info);
bool vesync_flash_read_info(device_info_t *x_info);
void vesync_flash_user(void);
uint32_t vesync_flash_write_i8(uint8_t value);
uint32_t vesync_flash_read_i8(uint8_t *value);











































#endif
