
#include "vesync_flash.h"
#include "vesync_crc8.h"
#include "esp_err.h"

#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG = "vesync_FLASH";

#define CONFIG_FILE_MAX_SIZE             (4096)//1.5k


/**
 * @brief 
 * @param filename  存储的文件名，位于Nvs区 
 * @param item      条目名，每条存储数据可以分为不同的条目存放
 * @param data      欲存储的数据，void类型，可根据len长度强转
 * @param len       data数据长度
 * @return true     写入成功
 * @return false    写入失败
 */
bool vesync_flash_write(const char *label_name,const char *key_name,const void *data,uint32_t len)
{
    assert(data != NULL);
    assert(key_name != NULL);

    esp_err_t err;
    int err_code = 0;
    nvs_handle fp;
    char *buf = malloc(CONFIG_FILE_MAX_SIZE);

    if (!buf) {
        err_code |= 0x01;
        goto error;
    }
    
    err = nvs_open_from_partition(label_name,key_name, NVS_READWRITE, &fp);
    if (err != ESP_OK) {
        printf("open from error %d\n",err);
        err_code |= 0x02;
        goto error;
    }

    size_t required_size = 0;
    err = nvs_get_blob(fp, key_name, NULL, &required_size); //获取当前键值对存储的数据总长度 

    if(required_size == 0){      
        memcpy(buf,data,len);
        required_size = len;
    }else{
        err = nvs_get_blob(fp, key_name, buf, &required_size); //获取当前键值对存储的数据内容
        memcpy(&buf[required_size],(char *)data ,len);
        required_size += len;
    }
    printf("required_size (%d)\n", required_size);
#if 1      
    nvs_stats_t nvs_stats;
    nvs_get_stats(label_name, &nvs_stats);

    printf("Count: UsedEntries = (%d), FreeEntries = (%d), AllEntries = (%d)\n",
       nvs_stats.used_entries, nvs_stats.free_entries, nvs_stats.total_entries);

    size_t used_entries;
    size_t total_entries_namespace =0;
    if(nvs_get_used_entry_count(fp, &used_entries) == ESP_OK){
        // the total number of records occupied by the namespace
        total_entries_namespace = used_entries + 1;
    }

    printf("used_entries (%d), total_entries_namespace = (%d)\n",
       used_entries, total_entries_namespace);
#endif
    err = nvs_set_blob(fp, key_name, buf, required_size);
    if (err != ESP_OK) {
        nvs_close(fp);
        err_code |= 0x04;
        goto error;
    }

    err = nvs_commit(fp);       //写完后需要更新flash
    if (err != ESP_OK) {
        nvs_close(fp);
        err_code |= 0x08;
        goto error;
    }
    char *r_buf = (char *)malloc(required_size);

    err = nvs_get_blob(fp, key_name, r_buf, &required_size);
    
    printf("\r\n");
    printf("read nvc------------------------>\n");
    if (err == ESP_OK) {
        for(int i=0;i<required_size;i++){
            printf("%d " ,r_buf[i]);
        }
    }
    printf("\r\n");
    printf("end----------------------------->\n");
    
    nvs_close(fp);
    free(buf);
    free(r_buf);
        return true;

error:
    printf("falsh error\r\n");
    if (buf) {
        free(buf);
    }
    return false;
}

#if 0
static esp_err_t vesync_flash_net_data_to_nvs_handle(const char *filename,const char *item,
        const esp_phy_calibration_data_t* cal_data)
{
    esp_err_t err;

    err = nvs_set_blob(handle, PHY_CAL_DATA_KEY, cal_data, sizeof(*cal_data));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s: store calibration data failed(0x%x)\n", __func__, err);
        return err;
    }

    uint8_t sta_mac[6];
    esp_efuse_mac_get_default(sta_mac);
    err = nvs_set_blob(handle, PHY_CAL_MAC_KEY, sta_mac, sizeof(sta_mac));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s: store calibration mac failed(0x%x)\n", __func__, err);
        return err;
    }

    uint32_t cal_format_version = phy_get_rf_cal_version() & (~BIT(16));
    ESP_LOGV(TAG, "phy_get_rf_cal_version: %d\n", cal_format_version);
    err = nvs_set_u32(handle, PHY_CAL_VERSION_KEY, cal_format_version);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s: store calibration version failed(0x%x)\n", __func__, err);
        return err;
    }

    err = nvs_commit(handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s: store calibration nvs commit failed(0x%x)\n", __func__, err);
    }
    
    return err;
}
#endif

/**
 * @brief 
 * @param part_name 
 */
void vesync_flash_init(const char *part_name)
{
    esp_err_t ret;
    /* Initialize Special NVS. */
    ret = nvs_flash_init_partition(part_name);
    ESP_ERROR_CHECK( ret );

}

void vesync_flash_deinit(const char *part_name)
{
    esp_err_t ret;
    /* Initialize Special NVS. */
    ret = nvs_flash_deinit_partition(part_name);
    ESP_ERROR_CHECK( ret );

}