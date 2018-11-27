#ifndef _VESYNC_FLASH_H
#define _VESYNC_FLASH_H

#include "vesync_wifi.h"
#include "c_types.h"
#include <stdbool.h>


#define USER_DATA_LABEL         “userdata”









void vesync_flash_init(const char *part_name);
void vesync_flash_deinit(const char *part_name);

bool vesync_flash_write(const char *label_name,const char *key_name,const void *data,uint32_t len);
esp_err_t vesync_flash_write_info(device_info_t *info);
device_info_t vesync_flash_read_info(void);












































#endif
