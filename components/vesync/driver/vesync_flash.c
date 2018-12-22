/**
 * @file vesync_flash.c
 * @brief vesync设备flash读写驱动
 * @author Jiven
 * @date 2018-12-18
 */
#include "vesync_flash.h"
#include "esp_err.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"

static const char *TAG = "vesync_FLASH";

#define CONFIG_FILE_MAX_SIZE             (4096)//1.5k

/**
 * @brief 擦除当前key_name对应的flash区域
 * @param label_name 
 * @param key_name 
 */
void vesync_flash_erase(const char *label_name,const char *key_name)
{
    esp_err_t err =0;
    nvs_handle fp;

    err = nvs_open_from_partition(label_name,key_name, NVS_READWRITE, &fp);
    ESP_LOGI(TAG, "NVS read err:%d",err);
    ESP_ERROR_CHECK(err);
    ESP_ERROR_CHECK(nvs_erase_all(fp));

    nvs_close(fp);
}
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

    esp_err_t err =0;
    int err_code = 0;
    nvs_handle fp;
    char *buf = malloc(CONFIG_FILE_MAX_SIZE);

    if (!buf) {
        err_code |= 0x01;
        goto error;
    }
    
    err = nvs_open_from_partition(label_name,key_name, NVS_READWRITE, &fp);
    ESP_LOGI(TAG, "NVS write %s ,err:%d",label_name,err);

    if (err != ESP_OK) {
        err_code |= 0x02;
        goto error;
    }

    size_t required_size = 0;
    err = nvs_get_blob(fp, key_name, NULL, &required_size); //获取当前键值对存储的数据总长度 
    ESP_LOGI(TAG, "NVS get len =%d,err =0x%04x",required_size,err);

    if(required_size == 0){      
        memcpy(buf,data,len);
        required_size = len;
    }else{
        err = nvs_get_blob(fp, key_name, buf, &required_size); //获取当前键值对存储的数据内容
        memcpy(&buf[required_size],(char *)data ,len);
        required_size += len;
    }
    ESP_LOGI(TAG, "NVS ready write len =%d,err =0x%04x",required_size,err);
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
            printf("0x%02x " ,r_buf[i]);
        }
    }
    printf("\r\n");
    printf("end----------------------------->\n");
    
    nvs_close(fp);
    free(buf);
    free(r_buf);
        return true;

error:
    printf("falsh error %d;err 0x%04x\r\n",err_code ,err);
    if (buf) {
        free(buf);
    }
    return false;
}

/**
 * @brief 读取key_name对应的flash存储内容
 * @param label_name 
 * @param key_name 
 * @param data 
 * @param len 
 * @return uint32_t 返回当前读取成功的数据长度
 */
void vesync_flash_read(const char *label_name,const char *key_name,const void *data,uint16_t *len)
{
    esp_err_t err =0;
    nvs_handle fp;
    size_t required_size = 0;

    err = nvs_open_from_partition(label_name,key_name, NVS_READONLY, &fp);
    ESP_LOGI(TAG, "NVS read err:%d",err);

    err = nvs_get_blob(fp, key_name, NULL, &required_size);     //获取当前键值对存储的数据总长度 
    ESP_ERROR_CHECK(err);

    ESP_LOGI(TAG, "vesync flash read len %s,%s,%d",label_name,key_name,required_size);

    if(required_size == 0){      
        *len = 0;
    }else{
        *len = required_size;
        err = nvs_get_blob(fp, key_name, (char *)data, &required_size); //获取当前键值对存储的数据内容
        ESP_ERROR_CHECK(err);
    }
    
    nvs_close(fp);
}
/**
 * @brief flash写入uint8_t类型数据
 * @param value 
 * @return uint32_t 
 */
uint32_t vesync_flash_write_i8(const char *label_name,const char *key_name,uint8_t value)
{
    uint32_t err = ESP_OK;
    nvs_handle handle;

    err = nvs_open(label_name, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGD(TAG, "%s: failed to open NVS namespace (0x%x)", __func__, err);
        return err;
    }
    err = nvs_set_u8(handle, key_name, value);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s: store pid failed(0x%x)\n", __func__, err);
        return err;
    }

    nvs_close(handle);
    return err;
}
/**
 * @brief flash读取uint8_t类型数据
 * @param value 
 * @return uint32_t 
 */
uint32_t vesync_flash_read_i8(const char *label_name,const char *key_name,uint8_t *value)
{
    nvs_handle handle;
    uint32_t err = ESP_OK;

    err = nvs_open(label_name, NVS_READONLY, &handle);
    if (err == ESP_ERR_NVS_NOT_INITIALIZED){
        ESP_LOGE(TAG, "%s: NVS has not been initialized. "
                "Call nvs_flash_init before starting WiFi/BT.", __func__);
        return err;        
    } else if(err != ESP_OK) {
        ESP_LOGE(TAG, "%s: failed to open NVS namespace (0x%x)", __func__, err);
        return err;
    }

    err = nvs_get_u8(handle, key_name, value);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s: read failed(0x%x)\n", __func__, err);
        return err;
    }
    nvs_close(handle);
    return err;
}

/**
 * @brief 写配网参数flash
 * @param info 
 * @return esp_err_t 
 */
uint32_t vesync_flash_write_net_info(net_info_t *info)
{
    esp_err_t err;
    nvs_handle handle;

    err = nvs_open(INFO_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGD(TAG, "%s: failed to open NVS namespace (0x%x)", __func__, err);
        return err;
    }
    ESP_ERROR_CHECK(nvs_erase_all(handle));

    err = nvs_set_blob(handle, INFO_pid_KEY, info->mqtt_config.pid, sizeof(info->mqtt_config.pid));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s: store pid failed(0x%x)\n", __func__, err);
        return err;
    }
    err = nvs_set_blob(handle, INFO_config_KEY, info->mqtt_config.configKey, sizeof(info->mqtt_config.configKey));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s: store configkey failed(0x%x)\n", __func__, err);
        return err;
    }
    err = nvs_set_blob(handle, INFO_DN_KEY, info->mqtt_config.serverDN, sizeof(info->mqtt_config.serverDN));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s: store server_dn failed(0x%x)\n", __func__, err);
        return err;
    }
    err = nvs_set_blob(handle, INFO_IP_KEY, info->mqtt_config.serverIP, sizeof(info->mqtt_config.serverIP));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s: store server_IP failed(0x%x)\n", __func__, err);
        return err;
    }
    err = nvs_set_blob(handle, INFO_SSID_KEY, info->station_config.wifiSSID, sizeof(info->station_config.wifiSSID));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s: store ssid failed(0x%x)\n", __func__, err);
        return err;
    }
    err = nvs_set_blob(handle, INFO_PWD_KEY, info->station_config.wifiPassword, sizeof(info->station_config.wifiPassword));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s: store pwd failed(0x%x)\n", __func__, err);
        return err;
    }
    err = nvs_set_blob(handle, INFO_static_IP_KEY, info->station_config.wifiStaticIP, sizeof(info->station_config.wifiStaticIP));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s: store static ip failed(0x%x)\n", __func__, err);
        return err;
    }
    err = nvs_set_blob(handle, INFO_gateWay_KEY, info->station_config.wifiGateway, sizeof(info->station_config.wifiGateway));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s: store gate way failed(0x%x)\n", __func__, err);
        return err;
    }
    err = nvs_set_blob(handle, INFO_DNS_KEY, info->station_config.wifiDNS, sizeof(info->station_config.wifiDNS));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s: store dns failed(0x%x)\n", __func__, err);
        return err;
    }

    err = nvs_commit(handle);       //写完后需要更新flash
    if (err != ESP_OK) {
        nvs_close(handle);
        return err;
    }

    ESP_LOGI(TAG, "NVS store net info ok!!!!!");
    nvs_close(handle);
    return err;
}

/**
 * @brief 读取info 设备信息
 * @return device_info_t 
 */
bool vesync_flash_read_net_info(net_info_t *x_info)
{
    nvs_handle handle;
    net_info_t *info = x_info;
    nvs_stats_t nvs_stats;
    
    nvs_get_stats(NULL, &nvs_stats);

    ESP_LOGI(TAG,"Count: UsedEntries = (%d), FreeEntries = (%d), AllEntries = (%d)\n",
       nvs_stats.used_entries, nvs_stats.free_entries, nvs_stats.total_entries);

    esp_err_t err = nvs_open(INFO_NAMESPACE, NVS_READONLY, &handle);
    if (err == ESP_ERR_NVS_NOT_INITIALIZED) {
        ESP_LOGE(TAG, "%s: NVS has not been initialized. "
                "Call nvs_flash_init before starting WiFi/BT.", __func__);
        return false;        
    } else if(err != ESP_OK) {
        ESP_LOGE(TAG, "%s: failed to open NVS namespace (0x%x)", __func__, err);
        return false;
    }

    size_t length = 0;
    char *buf = NULL;

    err = nvs_get_blob(handle, INFO_pid_KEY, NULL, &length);
    buf = (char *)malloc(length);
    err |= nvs_get_blob(handle, INFO_pid_KEY, buf, &length);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "pid len[%d]",length);
        memcpy(info->mqtt_config.pid,buf,length);
        esp_log_buffer_char(TAG,(char *)info->mqtt_config.pid,length);
    }
    free(buf);
    ESP_ERROR_CHECK(err);

    err = nvs_get_blob(handle, INFO_config_KEY, NULL, &length);
    buf = (char *)malloc(length);
    err |= nvs_get_blob(handle, INFO_config_KEY, buf, &length);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "config_key len[[%d]",length);
        memcpy(info->mqtt_config.configKey,buf,length);
        esp_log_buffer_char(TAG,(char *)info->mqtt_config.configKey,length);
    }
    free(buf);
    ESP_ERROR_CHECK(err);

    err = nvs_get_blob(handle, INFO_DN_KEY, NULL, &length);
    buf = (char *)malloc(length);
    err |= nvs_get_blob(handle, INFO_DN_KEY, buf, &length);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "INFO_server_DN_KEY len[%d]",length);
        memcpy(info->mqtt_config.serverDN,buf,length);
        esp_log_buffer_char(TAG,(char *)info->mqtt_config.serverDN,length);
    }
    free(buf);
    ESP_ERROR_CHECK(err);
    
    err = nvs_get_blob(handle, INFO_IP_KEY, NULL, &length);
    buf = (char *)malloc(length);
    err |= nvs_get_blob(handle, INFO_IP_KEY, buf, &length);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "INFO_server_IP_KEY len[%d]",length);
        memcpy(info->mqtt_config.serverIP,buf,length);
        esp_log_buffer_char(TAG,(char *)info->mqtt_config.serverIP,length);
    }
    free(buf);
    ESP_ERROR_CHECK(err);

    err = nvs_get_blob(handle, INFO_SSID_KEY, NULL, &length);
    buf = (char *)malloc(length);
    err |= nvs_get_blob(handle, INFO_SSID_KEY, buf, &length);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "INFO_SSID_KEY len[%d]",length);
        memcpy(info->station_config.wifiSSID,buf,length);
        esp_log_buffer_char(TAG,(char *)info->station_config.wifiSSID,length);
    }
    free(buf);
    ESP_ERROR_CHECK(err);

    err = nvs_get_blob(handle, INFO_PWD_KEY, NULL, &length);
    buf = (char *)malloc(length);
    err |= nvs_get_blob(handle, INFO_PWD_KEY, buf, &length);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "INFO_PWD_KEY len[%d]",length);
        memcpy(info->station_config.wifiPassword,buf,length);
        esp_log_buffer_char(TAG,(char *)info->station_config.wifiPassword,length);
    }
    free(buf);
    ESP_ERROR_CHECK(err);

    err = nvs_get_blob(handle, INFO_static_IP_KEY, NULL, &length);
    buf = (char *)malloc(length);
    err |= nvs_get_blob(handle, INFO_static_IP_KEY, buf, &length);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "INFO_static_IP_KEY len[%d]",length);
        memcpy(info->station_config.wifiStaticIP,buf,length);
        esp_log_buffer_char(TAG,(char *)info->station_config.wifiStaticIP,length);
    }
    free(buf);
    ESP_ERROR_CHECK(err);

    err = nvs_get_blob(handle, INFO_gateWay_KEY, NULL, &length);
    buf = (char *)malloc(length);
    err |= nvs_get_blob(handle, INFO_gateWay_KEY, buf, &length);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "INFO_gateWay_KEY len[%d]",length);
        memcpy(info->station_config.wifiGateway,buf,length);
        esp_log_buffer_char(TAG,(char *)info->station_config.wifiGateway,length);
    }
    free(buf);
    ESP_ERROR_CHECK(err);

    err = nvs_get_blob(handle, INFO_DNS_KEY, NULL, &length);
    buf = (char *)malloc(length);
    err |= nvs_get_blob(handle, INFO_DNS_KEY, buf, &length);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "INFO_DNS_KEY len[%d]",length);
        memcpy(info->station_config.wifiDNS,buf,length);
        esp_log_buffer_char(TAG,(char *)info->station_config.wifiDNS,length);
    }
    free(buf);
    ESP_ERROR_CHECK(err);
    
    nvs_close(handle);

    return true;
}

/**
 * @brief 初始化 自定义partition区
 * @param enable 
 * @param part_name 
 */
void vesync_flash_config(bool enable ,const char *part_name)
{
    esp_err_t ret;
    /* Initialize Special NVS. */
	if(enable){
    	ret = nvs_flash_init_partition(part_name);
		ESP_LOGI(TAG, "NVS init %s ,ret:%d",part_name,ret);  
	}else{
		ret = nvs_flash_deinit_partition(part_name);
		ESP_LOGI(TAG, "NVS deinit %s ,ret:%d",part_name,ret);
	}
    ESP_LOGI(TAG, "NVS init %s ,ret:%d",part_name,ret);  
    ESP_ERROR_CHECK( ret );
}




