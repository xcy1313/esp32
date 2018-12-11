/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_blufi_api.h"
#include "body_fat_calc.h"
#include "etekcity_bt_prase.h"
#include "driver/rtc_io.h"
#include "driver/adc.h"


#include "vesync_public.h"
#include "vesync_mqtt_client.h"
#include "vesync_ota.h"
#include "vesync_uart.h"
#include "vesync_bt.h"
#include "vesync_wifi.h"
#include "vesync_blufi.h"
#include "vesync_button.h"
#include "vesync_sntp.h"
#include "vesync_unixtime.h"
#include "vesync_flash.h"
#include "vesync_crc8.h"
#include "vesync_https.h"

#include "esp_log.h"

static const char *TAG = "main";

bt_frame_t  bt_prase;
/**
 * @brief 
 * @param p_event_data 
 */
void ui_event_handler(void *p_event_data){
    ESP_LOGI(TAG, "key [%d]\r\n" ,*(uint8_t *)p_event_data);
    switch(*(uint8_t *)p_event_data){
        case Short_key:{
                uint8_t unit;
                uint8_t backup_unix = info_str.user_config_data.measu_unit;
                if(backup_unix == UNIT_LB){
                    backup_unix = UNIT_KG;
                }else if(backup_unix == UNIT_KG){
                    backup_unix = UNIT_ST;
                }else if(backup_unix == UNIT_ST){
                    backup_unix = UNIT_LB;
                }
                vesync_flash_read_i8(&unit);
                ESP_LOGI(TAG, "unit [%d]",unit);

                info_str.user_config_data.measu_unit = backup_unix;
                vesync_flash_write_i8(info_str.user_config_data.measu_unit);
                ESP_LOGI(TAG, "unit is %d\r\n" ,info_str.user_config_data.measu_unit);
                resend_cmd_bit |= RESEND_CMD_MEASURE_UNIT_BIT;
                uart_encode_send(MASTER_SET,CMD_MEASURE_UNIT,(char *)&info_str.user_config_data.measu_unit,sizeof(uint8_t),true);
            }
			return;
        case Double_key:
			return;
        case Reapet_key:{
                static uint8_t status = 2;
                if(status == 2){
                    status = 3;
                }else{
                    status = 2;
                }
                uart_encode_send(MASTER_SET,CMD_BT_STATUS,(char *)&status,1,false);		
            }
			return;
        case Very_Long_key:

			return;
        default:
        return;  
    }
}

/**
 * @brief 
 * @param status 
 */
static void ota_event_handler(vesync_ota_status_t status)
{
    ESP_LOGI(TAG, "ota status: %02x", status);
    switch(status){
        case OTA_TIME_OUT:

            break;
        case OTA_BUSY:
            
            break;
        case OTA_SUCCESS:
            xEventGroupClearBits(user_event_group, OTA_BIT);
            break;
        default:
            break;
    }
}

/**
 * @brief app切换测重单位，需要串口下发称体显示,保存当前称体单位
 * @param opt 
 * @return true 
 * @return false 
 */
bool vesync_config_unit(hw_info *info,uint8_t *opt,uint8_t len)
{
    bool ret = false;
    if(len > sizeof(info->user_config_data.measu_unit))  return false;

    if(opt[0] != info->user_config_data.measu_unit){
        memcpy((uint8_t *)&info->user_config_data.measu_unit,(uint8_t *)opt,len);
        vesync_flash_write_i8(info->user_config_data.measu_unit);  
        uart_encode_send(MASTER_SET,CMD_MEASURE_UNIT,(char *)&opt[0],sizeof(uint8_t),true);
    }
    ESP_LOGI(TAG, "unit is %d\r\n" ,opt[0]);

    return ret;
}
/**
 * @brief app下发体脂计算参数，需要串口下发称体显示
 * @param opt 
 * @return true 
 * @return false 
 */
bool vesync_config_fat(hw_info *info,uint8_t *opt,uint8_t len)
{
    bool ret = false;
    if(len > sizeof(info->user_fat_data))  return false;

    memcpy((uint8_t *)&info->user_fat_data.fat,(uint8_t *)opt,len);
    resend_cmd_bit |= RESEND_CMD_BODY_FAT_BIT;
    uart_encode_send(MASTER_SET,CMD_BODY_FAT,(char *)opt,len,true);    //透传app体脂计算参数 

    return ret;
}

/**
 * @brief app下发unix时间戳，设置本地utc时间
 * @param opt 
 * @return true 
 * @return false 
 */
bool vesync_set_unix_time(uint8_t *opt ,uint8_t len)
{
    uint32_t unix_time = *(uint32_t *)&opt[1];
    Rtc_SyncSet_Time(unix_time,opt[0]);
    return true;
}

/**
 * @brief 配置用户模型
 * @param opt 
 * @return true 
 * @return false 
 */
bool vesync_config_account(hw_info *info,uint8_t *opt ,uint8_t len)
{
    bool ret = true;
    user_config_data_t user_list[MAX_CONUT] ={0};
    uint16_t length;
    uint8_t  user_cnt =0;

    uint8_t crc8 = vesync_crc8(0,opt,len); //过滤crc字段
    ESP_LOGI(TAG, "crc8= %d config_crc8:%d",crc8,info->user_config_data.crc8);

    if(len > sizeof(user_config_data_t))    return false;

    if(crc8 != info->user_config_data.crc8){
        memcpy((uint8_t *)&info->user_config_data.action,opt,len);
        info->user_config_data.crc8 = crc8;
        info->user_config_data.length = len;

        switch(info->user_config_data.action){
            case 2: //修改旧账户模型信息
                vesync_flash_read("userdata","config",(char *)user_list,&length);   //读取当前所有配置用户模型 
                
                for(uint8_t i=0;i<(length/sizeof(user_config_data_t));i++){
                    if(user_list[i].account == info->user_config_data.account){
                        user_cnt++;
                        memcpy((uint8_t *)&user_list[i].action,info->user_config_data.action,len);        //定位当前修改用户模型在flash中的位置并拷贝数据;
                        break;
                    }
                }
                if(user_cnt !=0){
                    if(vesync_flash_write("userdata","config",(uint8_t *)user_list,length)){   //按4字节整数倍保存用户信息
                        ESP_LOGI(TAG, "store user config ok! crc8 =%d len =%d\r\n",info->user_config_data.crc8 ,length);
                        ret = true;
                    }
                }
                break;
            case 1:{ //删除对应的旧账户模型信息
                    uint8_t sub_cnt=0;
                    uint8_t nlen =0;
                    uint8_t slen =0;
                    vesync_flash_read("userdata","config",(char *)user_list,&length);   //读取当前所有配置用户模型
                    nlen = length/sizeof(user_config_data_t)-1;
                    ESP_LOGI(TAG, "nlen[%d]",nlen);

                    if(1< nlen){
                        user_config_data_t *p_buf = (user_config_data_t *)malloc(nlen*sizeof(user_config_data_t));          //当前存储账户大于1个
                        memset(p_buf,NULL,nlen*sizeof(user_config_data_t));
                        int8_t j=-1;

                        for(uint8_t i=0;i<nlen;i++){
                            ESP_LOGI(TAG, "user account[0x%04x],delete account[0x%04x]",user_list[i].account,info->user_config_data.account);
                            if(user_list[i].account == info->user_config_data.account){     //当前欲删除的用户模型
                                j=(i==0?1:i+1);                                               //记录当前的序列
                            }else{
                                j= j+1;
                            }
                            memcpy((user_config_data_t *)&p_buf[i],(user_config_data_t *)&user_list[j],sizeof(user_config_data_t));
                        }
                        vesync_flash_erase("userdata","config");
                        if(vesync_flash_write("userdata","config",(uint8_t *)&p_buf[0].action,nlen*sizeof(user_config_data_t))){   //按4字节整数倍保存用户信息
                            ESP_LOGI(TAG, "store user re-flash ok!");
                            ret = true;
                        }else{
                            ret = false;
                        }
                        free(p_buf);
                    }else{
                        ESP_LOGI(TAG, "user account just one!");
                        vesync_flash_erase("userdata","config");
                    }
                    ESP_LOGI(TAG, "delete user account config!");
                    ret = true;
                }
                break;
            case 0: //创建新账户模型信息
                if(vesync_flash_write("userdata","config",(uint8_t *)&info->user_config_data.action,sizeof(info->user_config_data))){   //按4字节整数倍保存用户信息
                    ESP_LOGI(TAG, "store user config ok! crc8 =%d len =%d\r\n",info->user_config_data.crc8 ,sizeof(info->user_config_data));
                    ret = true;
                }else{
                    ret = false;
                }
                break;
            default:
                ret = false;
                break;
        }
    }
    ESP_LOGI(TAG, "action[0x%02x],account[0x%04x]",info->user_config_data.action,info->user_config_data.account);
   
    return ret;
}

/**
 * @brief 查询角色沉淀数据 app需要带账户名下发来查询本地用户模型数据库
 * @param len 
 * @return true 
 * @return false 
 */
bool vesync_inquiry_weight_history(uint32_t account,uint16_t *len)
{
     bool ret = false;
     
     *len = 160;
     return ret;
}
/**
 * @brief 删除所有用户模型,执行flash user区用户模型擦除功能
 * @param opt 
 * @return true 
 * @return false 
 */
bool vesync_delete_account(uint8_t *opt)
{
    bool ret = false;
    if(opt[0] == 1){    //为1表示删除所有用户模型信息
        vesync_flash_erase("userdata","config");
        ret = true;
    }
    return ret;
}

/**
 * @brief 获取硬件玛
 * @param buf 
 * @return true 
 * @return false 
 */
bool vesync_get_hw_version(hw_info *info,uint8_t *opt ,uint8_t *len)
{
    *(unsigned short *)&opt[0] = info->response_version_data.hardware;
    *(unsigned short *)&opt[2] = info->response_version_data.firmware;
    *(unsigned short *)&opt[4] = info->response_version_data.protocol;
    
    *len = 6;
    return true;
}

/**
 * @brief 获取硬件产品编码
 * @param buf 
 * @return true 
 * @return false 
 */
bool vesync_get_hw_coding(hw_info *info,uint8_t *opt,uint8_t *len)
{
    *(unsigned short *)&opt[0] = info->response_encodeing_data.type;
    *(unsigned short *)&opt[2] = info->response_encodeing_data.item;

    *len = 4;
    return true;
}

/**
 * @brief 获取电池电量百分比和开关机状态
 * @param opt 
 * @return true 
 * @return false 
 */
bool vesync_get_battery_power(hw_info *info,uint8_t *opt,uint8_t *len)
{
    *(unsigned char *)&opt[0] = info->response_hardstate.battery_level;      //开关机状态，1为开机 0为关机
    *(unsigned char *)&opt[1] = info->response_hardstate.power;              //电池电量百分比;

    *len = 2;
    return true;
}

/**
 * @brief 队列解析蓝牙接收数据
 * @param hw_data 硬件设备信息
 * @param data_buf app下发数据
 * @param length 
 */
void bt_event_handler(const void*hw_data,const void*data_buf, unsigned short length)
{
    for(int i=0;i<length;i++){
        if(bt_data_frame_decode(*(unsigned char *)&data_buf[i],0,&bt_prase) == 1){
            frame_ctrl_t res_ctl ={     //应答包res状态  
                .data =0,
            };
            struct{                     //应答数据包
                uint8_t buf[20];
                uint8_t len;
            }resp_strl ={{0},0};         
            uint8_t *cnt = NULL;
            hw_info *info = (hw_info *)hw_data;     
            uint8_t *opt = &bt_prase.frame_data[0]; //指针指向data_buf数据域；
            uint8_t len = bt_prase.frame_data_len-sizeof(uint16_t); //长度减去包含2个字节的命令包
            cnt = &bt_prase.frame_cnt;
            ESP_LOGI(TAG, "payload len[%d]",len);
            
            if(bt_prase.frame_ctrl.bitN.ack_flag == PACKET_COMMAND){        //下发的是命令包
                ESP_LOGI(TAG, "app set cmd [0x%04x]",bt_prase.frame_cmd);
                esp_log_buffer_hex(TAG,(char *)opt,len);   
                switch(bt_prase.frame_cmd){
                    case CMD_RESP_VERSION:                                  
                            vesync_get_hw_version(info,&resp_strl.buf ,&resp_strl.len);
                        break;
                    case CMD_RESP_CODING:                                   
                            vesync_get_hw_coding(info,&resp_strl.buf,&resp_strl.len);
                        break;
                    case CMD_RESP_POWER:                                    
                            vesync_get_battery_power(info,&resp_strl.buf,&resp_strl.len);
                        break;         
                    case CMD_SET_WEIGHT_UNIT:
                        if(vesync_config_unit(info,opt,len)){
                            //command_type[i].transfer_callback = vesync_bt_notify;
                            ESP_LOGI(TAG, "CMD_SET_WEIGHT_UNIT");
                        }else{
                            res_ctl.bitN.error_flag = 1;
                        } 
                        break;
                    case CMD_SYNC_TIME:
                        if(vesync_set_unix_time(opt,len)){
                            ESP_LOGI(TAG, "CMD_SYNC_TIME");
                        }else{
                            res_ctl.bitN.error_flag = 1;
                        }
                        break;
                    case CMD_CONFIG_ACCOUNT:
                        if(vesync_config_account(info,opt,len)){
                            ESP_LOGI(TAG, "CMD_CONFIG_ACCOUNT");
                        }else{
                            resp_strl.buf[0] = 1;   //具体产品对应的错误码
                            resp_strl.len = 1;
                            res_ctl.bitN.error_flag = 1;
                        }
                        break;
                    case CMD_DELETE_ACCOUNT:{
                            uint8_t action = opt;
                            if(vesync_delete_account(&action)){
                                ESP_LOGI(TAG, "CMD_DELETE_ACCOUNT");
                            }else{
                                resp_strl.buf[0] = 1;   //具体产品对应的错误码
                                resp_strl.len = 1;
                                res_ctl.bitN.error_flag = 1;
                            }
                        }
                        break;
                    case CMD_INQUIRY_HISTORY:{
                            uint16_t len = 0;
                            memcpy((uint8_t *)&info->user_config_data.account,(uint8_t *)opt,sizeof(info->user_config_data.account));
                            if(vesync_inquiry_weight_history(&info->user_config_data.account,&len)){
                                ESP_LOGI(TAG, "CMD_INQUIRY_HISTORY");
                            }else{
                                resp_strl.buf[0] = 1;   //具体产品对应的错误码
                                resp_strl.len = 1;
                                res_ctl.bitN.error_flag = 1;
                            }
                        }
                        break;
                    case CMD_SET_FAT_CONFIG:
                            if(vesync_config_fat(info,opt,len)){
                                ESP_LOGI(TAG, "CMD_SET_FAT_CONFIG");
                            }else{
                                resp_strl.buf[0] = 1;   //具体产品对应的错误码
                                resp_strl.len = 1;
                                res_ctl.bitN.error_flag = 1;
                            }
                        break;
                    case CMD_UPGRADE:
                        break;   
                    default:
                        break; 
                }
                if(bt_prase.frame_ctrl.bitN.request_flag == NEED_ACK){
                    res_ctl.bitN.ack_flag = 1;       //标示当前数据包为应答包;
                    if(res_ctl.bitN.error_flag == 1){
                        ESP_LOGE(TAG, "ERROR CODE！");
                        vesync_bt_notify(res_ctl,cnt,bt_prase.frame_cmd,resp_strl.buf,resp_strl.len);  //返回1个字节的具体错误码
                    }else{
                        vesync_bt_notify(res_ctl,cnt,bt_prase.frame_cmd,resp_strl.buf,resp_strl.len);  //返回应答设置或查询包
                    }
                    ESP_LOGI(TAG, "ack is need with command[0x%04x] ctrl[0x%02x].............",bt_prase.frame_cmd,res_ctl.data);
                }
            }else if(bt_prase.frame_ctrl.bitN.ack_flag == PACKET_RESP){  //app返回的应答
               ESP_LOGI(TAG, "app response");
               switch(bt_prase.frame_cmd){
                    case CMD_REPORT_VESION:
                        ESP_LOGI(TAG, "CMD_REPORT_VESION");
                        break;
                    case CMD_REPORT_CODING:
                        ESP_LOGI(TAG, "CMD_REPORT_CODING");
                        break;
                    case CMD_REPORT_WEIGHT:
                        ESP_LOGI(TAG, "CMD_REPORT_WEIGHT");
                        break;
                    case CMD_REPORT_ERRPR:
                        ESP_LOGI(TAG, "CMD_REPORT_ERRPR");
                        break;
                    case CMD_REPORT_POWER:
                        ESP_LOGI(TAG, "CMD_REPORT_POWER");
                        break;
                    case CMD_SYNC_TIME:
                        ESP_LOGI(TAG, "CMD_SYNC_TIME");
                        break;
                    default:
                        break;
               }
            }
        }
    }
}

/**
 * @brief 
 * @param uart_buf 
 * @param length 
 */
void  uart_event_handler(const void*uart_buf, unsigned short length)
{


}

/**
 * @brief 
 */
void app_main()
{
    esp_err_t ret;
    /* Initialize NVS. */
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "nvs_flash_init failed (0x%x), erasing partition and retrying", ret);
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    vesync_public_init();
    //vesync_ota_init(ota_event_handler);
    vesync_flash_user();
    vesync_wifi_init();
    vesync_init_sntp_service(1544410793,8,"ntp.vesync.com");
    vesync_bt_init(bt_event_handler);
    vesync_uart_int(uart_event_handler);

    vesync_blufi_init();
    vesync_button_init(ui_event_handler);
    //vesync_mqtt_client_init();

    //vesync_https_init();

    //vesync_power_save_enter(WAKE_UP_PIN);
}
