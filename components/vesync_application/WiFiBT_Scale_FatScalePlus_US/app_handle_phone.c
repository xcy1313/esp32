/**
 * @file app_handle_uart.C
 * @brief 处理和手机交互逻辑相关
 * @author Jiven
 * @date 2018-12-18
 */
#include "app_handle_scales.h"
#include "app_handle_phone.h"
#include "app_handle_server.h"

#include "vesync_wifi.h"
#include "vesync_bt_hal.h"
#include "vesync_uart.h"
#include "vesync_net_service.h"
#include "vesync_device.h"
#include "app_public_events.h"
#include "vesync_crc8.h"
#include "vesync_production.h"
#include "vesync_flash.h"

#include "vesync_interface.h"
#include "vesync_build_cfg.h"
#include "vesync_log.h"

static const char* TAG = "app_handle_phone";

static bt_frame_t  bt_prase ={0};

static void app_ble_start(void);

/**
 * @brief 应答app升级状态
 * @param trace_id 
 * @param result 
 */
static void app_handle_upgrade_response_ack(char *trace_id ,uint8_t result)
{
    frame_ctrl_t res_ctl ={     //应答包res状态  
                .data = 0x10,
            };
    struct{                     //应答数据包
        uint8_t buf[200];
        uint8_t len;
    }resp_strl ={{0},0};         
    static uint8_t cnt = 0;
    uint16_t upgrade_cmd = CMD_UPGRADE;

    cJSON *report = cJSON_CreateObject();
    if(NULL != report){
        cJSON_AddStringToObject(report, "traceId", trace_id);		        //==TODO==，需要修改成毫秒级
        cJSON_AddStringToObject(report, "cid",  (char)product_config.cid);		//设备cid
        cJSON_AddNumberToObject(report, "code", result);
    }
    switch(result){
        case RESPONSE_UPGRADE_BUSY:{
                cJSON* firmware = NULL;
                res_ctl.data = 0x10;
                cJSON_AddItemToObject(report, "firmware", firmware = cJSON_CreateObject());
                if(NULL != firmware){
                    cJSON_AddStringToObject(firmware, "newVersion",NULL);
                    cJSON_AddStringToObject(firmware, "url", NULL);
                }
            }
            break;
        default:
                res_ctl.data = 0x00;
            break;
    }
    vesync_printf_cjson(report);
    resp_strl.len = strlen(report);

    memcpy((char *)resp_strl.buf,(char *)report ,resp_strl.len);
    vesync_bt_notify(res_ctl,cnt,upgrade_cmd,resp_strl.buf,resp_strl.len);  //返回应答设置或查询包

    char* out = cJSON_PrintUnformatted(report);
    cJSON_Delete(report);
    cnt++;
}


/**
 * @brief 
 * @param status 
 */
void ota_event_handler(vesync_ota_status_t status)
{
    ESP_LOGI(TAG, "ota_event_handler %d\r\n" ,status);
    uint8_t bt_conn;
    switch(status){
        case OTA_TIME_OUT:

            break;
        case OTA_BUSY:
                bt_conn = 3;
                resend_cmd_bit |= RESEND_CMD_BT_STATUS_BIT;
                app_uart_encode_send(MASTER_SET,CMD_BT_STATUS,(unsigned char *)&bt_conn,sizeof(uint8_t),true);
                if(vesync_get_production_status() == RPODUCTION_RUNNING){
                    app_handle_production_upgrade_response_result("1547029501512",5); //升级中
                }else if(vesync_get_production_status() == PRODUCTION_EXIT){
                    app_handle_upgrade_response_ack("1547029501512",RESPONSE_UPGRADE_BUSY);
                }
            break;
        case OTA_FAILED:
            if(vesync_get_production_status() == RPODUCTION_RUNNING){
                app_handle_production_upgrade_response_result("1547029501599",1);     //升级失败
            }else if(vesync_get_production_status() == PRODUCTION_EXIT){
                app_handle_upgrade_response_ack("1547029501512",RESPONSE_UPGRADE_FAIL);
            }
            break;
        case OTA_SUCCESS:
            if(vesync_get_production_status() == RPODUCTION_RUNNING){
                app_handle_production_upgrade_response_result("1547029501529",0);     //升级成功
            }else if(vesync_get_production_status() == PRODUCTION_EXIT){
                bt_conn = 4;
                resend_cmd_bit |= RESEND_CMD_BT_STATUS_BIT;
                app_uart_encode_send(MASTER_SET,CMD_BT_STATUS,(unsigned char *)&bt_conn,sizeof(uint8_t),true);
                app_handle_upgrade_response_ack("1547029501512",RESPONSE_UPGRADE_SUCCESS);
            }
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

    switch(opt[0]){
        case 0:
        case 1:
        case 2:
            if(vesync_flash_write_i8(UNIT_NAMESPACE,UNIT_KEY,opt[0]) == 0){
                memcpy((uint8_t *)&info->user_config_data.measu_unit,(uint8_t *)opt,len);
                resend_cmd_bit |= RESEND_CMD_MEASURE_UNIT_BIT;
                app_uart_encode_send(MASTER_SET,CMD_MEASURE_UNIT,(unsigned char *)&opt[0],sizeof(uint8_t),true);
                ret = true;
            }
            break;
        default:
            break;
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
    app_uart_encode_send(MASTER_SET,CMD_BODY_FAT,(unsigned char *)opt,len,true);    //透传app体脂计算参数 
    ret = true;
    
    return ret;
}

/**
 * @brief 解析app下发的url链接
 * @param url 
 */
void vesync_prase_upgrade_url(char *url)
{
    char upgrade_url[128] = {'\0'};
    char new_version[10] = {'\0'};

    LOG_I(TAG, "User recv : %s", url);
    cJSON *root = cJSON_Parse(url);
    if(NULL == root){
        LOG_E(TAG, "Parse cjson error !");
        return;
    }
    vesync_printf_cjson(root);

    cJSON *firmware = cJSON_GetObjectItemCaseSensitive(root, "firmware");
    if(firmware != NULL){
        LOG_I(TAG, "upgrade test start!");
        cJSON* newVersion = cJSON_GetObjectItemCaseSensitive(firmware, "newVersion");
        if(cJSON_IsString(newVersion)){
            strcpy(new_version, newVersion->valuestring);	//记录升级的新版本
            LOG_I(TAG, "upgrade new_version %s",new_version);
        }
        cJSON* url = cJSON_GetObjectItemCaseSensitive(firmware, "url");
        if(cJSON_IsString(url)){
            if(NULL != upgrade_url){
                uint8_t url_len;
                strcpy(upgrade_url, url->valuestring);
                url_len = strlen(url->valuestring);
                sprintf(&upgrade_url[url_len],"%s.V%s%s",PRODUCT_WIFI_NAME,new_version,".bin");
                LOG_I(TAG, "upgrade url %s",upgrade_url);
                vesync_client_connect_wifi((char *)net_info.station_config.wifiSSID, (char *)net_info.station_config.wifiPassword);
                vesync_ota_init(upgrade_url,ota_event_handler);
            }
        }
    }
    cJSON_Delete(root);								
}
/**
 * @brief app下发升级指令
 * @param info 
 * @param opt 
 * @param len 
 * @return true 
 * @return false 
 */
bool vesync_upgrade_config(hw_info *info,uint8_t *opt,uint8_t len)
{
    bool ret = false;
    LOG_I(TAG, "vesync_upgrade_config");
    if(app_handle_get_net_status() > NET_CONFNET_NOT_CON){       //设备已配网
        // int ret =0;
        // char recv_buff[1024];
        // int buff_len = sizeof(recv_buff);
        // vesync_client_connect_wifi((char *)net_info.station_config.wifiSSID, (char *)net_info.station_config.wifiPassword);
        // ret = vesync_https_client_request("deviceRegister", "hello", recv_buff, &buff_len, 2 * 1000);
        // if(buff_len > 0 && ret == 0){
        //     LOG_I(TAG, "Https recv %d byte data : \n%s", buff_len, recv_buff);
        // }
        vesync_client_connect_wifi((char *)net_info.station_config.wifiSSID, (char *)net_info.station_config.wifiPassword);
        //app_handle_net_service_task_notify_bit(UPGRADE_ADDR_REQ);
        vesync_prase_upgrade_url((char *)opt);
        ret = true;
    }
    return ret;
}

/**
 * @brief 产测模式获取蓝牙rssi信号强度
 * @param info 
 * @param opt 
 * @param len 
 * @return true 
 * @return false 
 */
bool vesync_factory_get_bt_rssi(hw_info *info,uint8_t *opt,uint8_t len)
{
    bool ret = false;
    int8_t rssi ;
    rssi = opt[0];

    if(vesync_get_production_status() == RPODUCTION_RUNNING){
        if((factory_test_bit & FACTORY_TEST_SYNC_BT_BIT) == FACTORY_TEST_SYNC_BT_BIT){
            app_handle_production_response_bt_rssi(vesync_get_time(),rssi);
            factory_test_bit &= ~FACTORY_TEST_SYNC_BT_BIT;
            vesync_bt_disconnect();
            app_ble_start();
        }
        ESP_LOGI(TAG, "bt rssi %d,0x%02x\n",rssi,rssi);
        ret = true;
    }
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
    //Rtc_SyncSet_Time((unsigned int *)&unix_time,(char)opt[0]);
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
    bool ret = false;
    user_config_data_t user_list[MAX_CONUT] ={0};
    uint16_t length =0;

    uint8_t crc8 = vesync_crc8(0,opt,len); //过滤crc字段
    //ESP_LOGI(TAG, "crc8= %d config_crc8:%d",crc8,info->user_config_data.crc8);

    if(len > sizeof(user_config_data_t))    return false;
    
    if(crc8 != info->user_config_data.crc8){
        info->user_config_data.crc8 = crc8;
        info->user_config_data.length = len;

        switch(info->user_config_data.action){
            case 3: //app同步当前用户配置
                memcpy((uint8_t *)&info->user_config_data.action,opt,len);                          //内存保存当前使用的用户模型配置
                break;
            case 2:{//修改旧账户模型信息
                    uint8_t  user_cnt =0;
                    uint32_t if_modyfy_account = *(uint32_t *)&opt[1];
                    vesync_flash_read(USER_MODEL_NAMESPACE,USER_MODEL_KEY,(char *)user_list,&length);   //读取当前所有配置用户模型 
                    
                    if(length > 0){
                        for(uint8_t i=0;i<(length/sizeof(user_config_data_t));i++){
                            if(user_list[i].account == if_modyfy_account){
                                user_cnt++;
                                memcpy((uint8_t *)&user_list[i].action,(uint8_t *)&opt[0],len);        //定位当前修改用户模型在flash中的位置并拷贝数据;
                                break;
                            }
                        }
                        if(user_cnt !=0){
                            if(vesync_flash_write(USER_MODEL_NAMESPACE,USER_MODEL_KEY,(uint8_t *)user_list,length)){   //按4字节整数倍保存用户信息
                                ESP_LOGI(TAG, "store user config ok! crc8 =%d len =%d\r\n",info->user_config_data.crc8 ,length);
                                ret = true;     //修改信息完毕;
                            }
                        }
                    }
                } 
                break;
            case 1:{ //删除对应的旧账户模型信息
                    uint8_t nlen =0;
                    uint32_t if_delete_account = *(uint32_t *)&opt[1];

                    vesync_flash_read(USER_MODEL_NAMESPACE,USER_MODEL_KEY,(char *)user_list,&length);   //读取当前所有配置用户模型
                    nlen = length/sizeof(user_config_data_t)-1;
                    ESP_LOGI(TAG, "nlen[%d]",nlen);

                    if(1< nlen){
                        user_config_data_t *p_buf = (user_config_data_t *)malloc(nlen*sizeof(user_config_data_t));          //当前存储账户大于1个
                        memset(p_buf,0,nlen*sizeof(user_config_data_t));
                        int8_t j=-1;

                        for(uint8_t i=0;i<nlen;i++){
                            ESP_LOGI(TAG, "user account[0x%04x],delete account[0x%04x]",user_list[i].account,if_delete_account);
                            if(user_list[i].account == if_delete_account){     //当前欲删除的用户模型
                                j=(i==0?1:i+1);                                               //记录当前的序列
                            }else{
                                j= j+1;
                            }
                            memcpy((user_config_data_t *)&p_buf[i],(user_config_data_t *)&user_list[j],sizeof(user_config_data_t));
                        }
                        vesync_flash_erase(USER_MODEL_NAMESPACE,USER_MODEL_KEY);
                        if(vesync_flash_write(USER_MODEL_NAMESPACE,USER_MODEL_KEY,(uint8_t *)&p_buf[0].action,nlen*sizeof(user_config_data_t))){   //按4字节整数倍保存用户信息
                            ESP_LOGI(TAG, "store user re-flash ok!");
                            ret = true;
                        }else{
                            ret = false;
                        }
                        free(p_buf);
                    }else{
                        vesync_flash_erase(USER_MODEL_NAMESPACE,USER_MODEL_KEY);
                        ESP_LOGI(TAG, "user account just one!");
                    }
                    ESP_LOGI(TAG, "delete user account config!");
                    ret = true;
                }
                break;
            case 0:{//创建新账户模型信息
                    uint8_t nlen =0;
                    vesync_flash_read(USER_MODEL_NAMESPACE,USER_MODEL_KEY,(char *)user_list,&length);   //读取当前所有配置用户模型
                    nlen = length/sizeof(user_config_data_t);
                    ESP_LOGI(TAG, "nlen[%d]",nlen);

                    if(nlen < MAX_CONUT){
                        user_config_data_t    user_config_data = {0};
                        memcpy((uint8_t *)&user_config_data.action,(uint8_t *)&opt[0],len);
                        ESP_LOGI(TAG, "account ID[0x%04x]",user_config_data.account);
                        snprintf((char *)user_config_data.user_store_key,sizeof(user_config_data.user_store_key),"his_%x",user_config_data.account); //根据不同的账号创建存储的用户键值对用来保存沉淀数据
                        ESP_LOGI(TAG, "create user key_value[%s]",user_config_data.user_store_key);
                        user_config_data.crc8 = vesync_crc8(0,&user_config_data.action,len);
                        user_config_data.length = len;

                        if(vesync_flash_write(USER_MODEL_NAMESPACE,USER_MODEL_KEY,(uint8_t *)&user_config_data.action,sizeof(user_config_data_t))){   //按4字节整数倍保存用户信息
                            ESP_LOGI(TAG, "store user config ok! crc8 =%d len =%d\r\n",user_config_data.crc8 ,user_config_data.length);
                            ret = true;
                        }
                    }
                } 
                break;
            default:
                break;
        }
    }
    LOG_I(TAG, "action[0x%02x],account[0x%04x]",info->user_config_data.action,info->user_config_data.account);
   
    return ret;
}

/**
 * @brief 查询角色沉淀数据 app需要带账户名下发来查询本地用户模型数据库
 * @param len 
 * @return true 
 * @return false 
 */
static bool vesync_inquiry_weight_history(uint32_t user_account ,uint16_t *len)
{
    bool ret = false;
    uint32_t ret_value =0;
    user_config_data_t user_list[MAX_CONUT] ={0};

    uint8_t nlen =0;
    uint16_t length =0;

    ret_value = vesync_flash_read(USER_MODEL_NAMESPACE,USER_MODEL_KEY,(char *)user_list,&length);   //读取当前所有配置用户模型
    nlen = length/sizeof(user_config_data_t);
    if((nlen >= 1) && (ret_value == 0)){
        uint8_t  user_cnt =0;
        uint8_t i =0;
        for(;i<nlen;i++){
            if(user_list[i].account == user_account){
                user_cnt++;
                ESP_LOGI(TAG, "current user ID[0x%04x]",user_list[i].account);
                break;
            }
        }
        if(user_cnt != 0){
            user_history_t user_history; 
            ESP_LOGI(TAG, "read user key_value[%s]",user_list[i].user_store_key);
            if(vesync_flash_read(USER_HISTORY_DATA_NAMESPACE,user_list[i].user_store_key,(char *)user_list,&len) == 0){
                ESP_LOGI(TAG, "read user history len is[%d]",*len);
            }
            ret = true;
        }
    }
    return ret;
}
/**
 * @brief 删除所有用户模型,执行flash user区用户模型擦除功能
 * @param opt 
 * @return true 
 * @return false 
 */
static bool vesync_delete_account(uint8_t *opt)
{
    bool ret = false;
    if(opt[0] == 1){    //为1表示删除所有用户模型信息
        vesync_flash_erase(USER_MODEL_NAMESPACE,USER_MODEL_KEY);
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
    *(unsigned short *)&opt[0] = 0x0001;//info->response_version_data.hardware;
    *(unsigned short *)&opt[2] = 0x0001;//info->response_version_data.firmware;
    *(unsigned short *)&opt[4] = 0x0001;//info->response_version_data.protocol;
    
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
    *(unsigned short *)&opt[0] = 0x0100;//info->response_encodeing_data.type;
    *(unsigned short *)&opt[2] = 0x0100;//info->response_encodeing_data.item;

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

static void app_bt_set_status(BT_STATUS_T bt_status)
{
    uint8_t bt_conn;
    LOG_I(TAG, "set ble status :%d" ,bt_status);
    switch(bt_status){
        case BT_CREATE_SERVICE:
            break;
        case BT_ADVERTISE_START:
                vesync_bt_advertise_start(APP_ADVERTISE_TIMEOUT);
            break;
        case BT_ADVERTISE_STOP:
            break;
        case BT_CONNTED:
                vesync_bt_advertise_stop();
                bt_conn = 2;

                resend_cmd_bit |= RESEND_CMD_BT_STATUS_BIT;
                app_uart_encode_send(MASTER_SET,CMD_BT_STATUS,(unsigned char *)&bt_conn,sizeof(uint8_t),true);
            break;
        case BT_DISCONNTED:
                bt_conn = 0;
                vesync_bt_advertise_start(APP_ADVERTISE_TIMEOUT);

                resend_cmd_bit |= RESEND_CMD_BT_STATUS_BIT;
                app_uart_encode_send(MASTER_SET,CMD_BT_STATUS,(unsigned char *)&bt_conn,sizeof(uint8_t),true);
            break;
        default:
            break;
    }
}
/**
 * @brief 队列解析蓝牙接收数据
 * @param hw_data 硬件设备信息
 * @param data_buf app下发数据
 * @param length 
 */
static void app_ble_recv_cb(const unsigned char *data_buf, unsigned char length)
{
#if 0    
    //esp_log_buffer_hex(TAG, data_buf, length);
    switch(data_buf[0]){
        case 1:
                vesync_set_production_status(RPODUCTION_RUNNING);   //状态调整为产测模式已开始;
                resend_cmd_bit &= ~RESEND_CMD_ALL_BIT;
                resend_cmd_bit |= RESEND_CMD_FACTORY_START_BIT;
                app_uart_encode_send(MASTER_SET,CMD_FACTORY_SYNC_START,0,0,true);
            break;
        case 2:
                resend_cmd_bit |= RESEND_CMD_FACTORY_CHARGE_BIT;
                app_uart_encode_send(MASTER_SET,CMD_FACTORY_CHARGING,0,0,true);
            break;
        case 3:
                resend_cmd_bit |= RESEND_CMD_FACTORY_WEIGHT_BIT;
                app_uart_encode_send(MASTER_SET,CMD_FACTORY_WEIGHT,0,0,true);
            break;
        case 4:
                vesync_set_production_status(PRODUCTION_EXIT);   //状态调整为产测模式已结束;
                resend_cmd_bit |= RESEND_CMD_FACTORY_STOP_BIT;
                app_uart_encode_send(MASTER_SET,CMD_FACTORY_SYNC_STOP,0,0,true);
            break;
    }
#else
    // switch(data_buf[0]){
    //     case 1:
    //         app_handle_net_service_task_notify_bit(NETWORK_CONFIG_REQ);
    //         break;
    //     case 2:
    //         app_handle_net_service_task_notify_bit(REFRESH_TOKEN_REQ);
    //         break;
    //     case 3:
    //         app_handle_net_service_task_notify_bit(UPLOAD_WEIGHT_DATA_REQ);
    //         break;
    //     case 4:
    //         app_handle_net_service_task_notify_bit(UPGRADE_ADDR_REQ);
    //         break;
    //     case 5:
    //         app_handle_net_service_task_notify_bit(REFRESH_DEVICE_ATTRIBUTE);
    //         break;
    // }    
    for(unsigned char i=0;i<length;++i){
        if(bt_data_frame_decode(data_buf[i],0,&bt_prase) == 1){
            frame_ctrl_t res_ctl ={     //应答包res状态  
                .data =0,
            };
            struct{                     //应答数据包
                uint8_t buf[20];
                uint8_t len;
            }resp_strl ={{0},0};         
            uint8_t *cnt = NULL;
            hw_info *info = &info_str;     
            uint8_t *opt = &bt_prase.frame_data[0]; //指针指向data_buf数据域；
            uint8_t len = bt_prase.frame_data_len-sizeof(uint16_t); //长度减去包含2个字节的命令包
            cnt = &bt_prase.frame_cnt;

            if(bt_prase.frame_ctrl.bitN.ack_flag == PACKET_COMMAND){        //下发的是命令包
                ESP_LOGI(TAG, "app set cmd [0x%04x]",bt_prase.frame_cmd);
                switch(bt_prase.frame_cmd){
                    case CMD_RESP_VERSION:                                  
                            vesync_get_hw_version(info,resp_strl.buf ,&resp_strl.len);
                            break;
                    case CMD_RESP_CODING:                                   
                            vesync_get_hw_coding(info,resp_strl.buf,&resp_strl.len);
                        break;
                    case CMD_RESP_POWER:                                    
                            vesync_get_battery_power(info,resp_strl.buf,&resp_strl.len);
                        break;         
                    case CMD_SET_WEIGHT_UNIT:
                        if(vesync_config_unit(info,opt,len)){
                            ESP_LOGI(TAG, "CMD_SET_WEIGHT_UNIT");
                        }else{
                            res_ctl.bitN.error_flag = 1;
                        } 
                        break;
                    case CMD_RESP_NET_STATUS:
                            resp_strl.buf[0] = app_handle_get_net_status();  
                            resp_strl.len = 1;
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
                            if(vesync_delete_account(opt)){
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
                            uint32_t if_inquiry_account = *(uint32_t *)&opt[0];
                            if(vesync_inquiry_weight_history(if_inquiry_account,&len)){
                                res_ctl.data = 0x10;
                                if(len != 0){
                                    *(uint32_t *)&resp_strl.buf[0] = if_inquiry_account;//账号id
                                    *(uint16_t *)&resp_strl.buf[4] = len;               //总长度
                                    ESP_LOGI(TAG, "CMD_INQUIRY_HISTORY");
                                }else{
                                    *(uint32_t *)&resp_strl.buf[0] = if_inquiry_account;//账号id
                                    *(uint16_t *)&resp_strl.buf[4] = len;               //总长度
                                    resp_strl.len = 6;
                                }
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
                            if(vesync_upgrade_config(info,opt,len)){
                                ESP_LOGI(TAG, "CMD_UPGRADE");
                            }else{
                                resp_strl.buf[0] = 1;   //具体产品对应的错误码
                                resp_strl.len = 1;
                                res_ctl.bitN.error_flag = 1;
                            }
                        break;   
                    case CMD_FACTORY_GET_BT_RSSI:   //获取产测蓝牙rssi信号强度
                            if(vesync_factory_get_bt_rssi(info,opt,len)){
                                ESP_LOGI(TAG, "CMD_FACTORY_GET_BT_RSSI");
                            }else{
                                resp_strl.buf[0] = 1;   
                                resp_strl.len = 1;
                                res_ctl.bitN.error_flag = 1;
                            }
                        break;
                    default:
                        ESP_LOGE(TAG, "app set other cmd!");
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
                            ESP_LOGI(TAG, "app resp other[0x%02x]",bt_prase.frame_cmd);
                            break;
                }
            }
        }
    }
#endif    
}

/**
 * @brief 初始化蓝牙广播,使能蓝牙配网服务
 */
void app_ble_init(void)
{
	vesync_bt_client_init(PRODUCT_NAME,PRODUCT_VER,PRODUCT_TYPE,PRODUCT_NUM,NULL,true,app_bt_set_status,app_ble_recv_cb);
    //vesync_bt_client_init(PRODUCT_NAME,PRODUCT_VER,PRODUCT_TEST_TYPE,PRODUCT_TEST_NUM,NULL,true,app_bt_set_status,app_ble_recv_cb);
    //vesync_bt_advertise_start(APP_ADVERTISE_TIMEOUT);
}
/**
 * @brief 初始化产测广播服务模式
 */
void app_product_ble_start(void)
{
    vesync_bt_dynamic_ble_advertise_para(PRODUCT_TEST_TYPE,PRODUCT_TEST_NUM);
}

/**
 * @brief 初始化产测广播服务模式
 */
static void app_ble_start(void)
{
    vesync_bt_dynamic_ble_advertise_para(PRODUCT_TYPE,PRODUCT_NUM);
}