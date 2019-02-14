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

static EventGroupHandle_t ble_event_group;

static char upgrade_url[128] = {'\0'};
static char new_version[10] = {'\0'};


static char    inquiry_user_store_key[12] ={0};	//用于用户分配时创建存储的key键值;
static uint32_t account_id =0;
static uint8_t inquiry_cnt =0;
/**
 * @brief 应答app升级状态
 * @param trace_id 
 * @param result 
 */
static void app_handle_upgrade_response_ack(char *trace_id ,uint8_t result,uint8_t percent)
{
    frame_ctrl_t res_ctl ={     //应答包res状态  
                .data = 0x0,
            };
    struct{                     //应答数据包
        uint8_t buf[2];
        uint16_t len;
    }resp_strl ={{0},0};         
    static uint8_t cnt = 1;
    uint16_t upgrade_cmd = CMD_UPGRADE;

    resp_strl.buf[1] = 0;
    resp_strl.buf[0] = result;
    resp_strl.len = 2;
    cnt++;
    if(result == RESPONSE_UPGRADE_PROCESS){
        resp_strl.buf[1] = percent;
    }
#if 0
    cJSON *report = cJSON_CreateObject();
    if(NULL != report){
        cJSON_AddStringToObject(report, "traceId", trace_id);		        //==TODO==，需要修改成毫秒级
        cJSON_AddStringToObject(report, "cid",  (char *)product_config.cid);		//设备cid
        cJSON_AddNumberToObject(report, "code", result);
    }
    switch(result){
        case RESPONSE_UPGRADE_BUSY:{
                cJSON* firmware = NULL;
                res_ctl.data = 0x10;
                cJSON_AddItemToObject(report, "firmware", firmware = cJSON_CreateObject());
                if(NULL != firmware){
                    cJSON_AddStringToObject(firmware, "newVersion",new_version);
                    cJSON_AddStringToObject(firmware, "url", upgrade_url);
                }
            }
            break;
        case RESPONSE_UPGRADE_PROCESS:
                cJSON_AddNumberToObject(report, "process", percent);
            break;
        default:
                res_ctl.data = 0x10;
            break;
    }    
    char *out = cJSON_PrintUnformatted(report);
    resp_strl.len = strlen(out);
    LOG_I("JSON", "\n%s", out);
    ESP_LOGI(TAG, "BT send len[%d]\r\n" ,resp_strl.len);
    memcpy(resp_strl.buf,(char *)out ,resp_strl.len);
    free(out);
    cJSON_Delete(report);
#endif    
    vesync_bt_notify(res_ctl,&cnt,upgrade_cmd,(unsigned char *)&resp_strl.buf[0],resp_strl.len);  //返回应答设置或查询包
}

static void app_handle_upgrade_process_response(uint32_t data)
{
    frame_ctrl_t res_ctl ={     //应答包res状态  
                .data = 0x10,
            };
    struct{                     //应答数据包
        uint8_t buf[300];
        uint16_t len;
    }resp_strl ={{0},0};         
    static uint8_t cnt = 1;
    uint16_t upgrade_cmd = CMD_UPGRADE;

    resp_strl.len = sizeof(data);
    ESP_LOGI(TAG, "BT send len[%d]\r\n" ,resp_strl.len);
    memcpy(resp_strl.buf,(uint8_t *)&data ,resp_strl.len);    
    vesync_bt_notify(res_ctl,&cnt,upgrade_cmd,(unsigned char *)&resp_strl.buf[0],resp_strl.len);  //返回应答设置或查询包

    cnt++;
}

/**
 * @brief 
 * @param status 
 */
void ota_event_handler(uint32_t len,vesync_ota_status_t status)
{
    uint8_t bt_conn;
    uint8_t ota_souce = app_get_upgrade_source();
    switch(status){
        case OTA_TIME_OUT:
                if(ota_souce == UPGRADE_PRODUCTION){
                    app_handle_production_upgrade_response_result("1547029501599",1);     //升级失败
                }else if(ota_souce == UPGRADE_APP){
                    app_handle_upgrade_response_ack("1547029501512",RESPONSE_UPGRADE_TIMEOUT,0);
                }
            break;
        case OTA_BUSY:
                bt_conn = 3;
                resend_cmd_bit |= RESEND_CMD_BT_STATUS_BIT;
                app_uart_encode_send(MASTER_SET,CMD_BT_STATUS,(unsigned char *)&bt_conn,sizeof(uint8_t),true);

                if(ota_souce == UPGRADE_PRODUCTION){
                    app_handle_production_upgrade_response_result("1547029501512",5); //升级中
                }else if(ota_souce == UPGRADE_APP){
                    //app_handle_upgrade_response_ack("1547029501512",RESPONSE_UPGRADE_BUSY);
                }
            break;
        case OTA_FAILED:
                if(ota_souce == UPGRADE_PRODUCTION){
                    app_handle_production_upgrade_response_result("1547029501599",1);     //升级失败
                }else if(ota_souce == UPGRADE_APP){
                    app_handle_upgrade_response_ack("1547029501512",RESPONSE_UPGRADE_FAIL,0);
                }
            break;
        case OTA_PROCESS:
                ESP_LOGI(TAG, "upgrade process %d\r\n" ,len);
                app_handle_upgrade_response_ack("1547029501512",RESPONSE_UPGRADE_PROCESS,len);
            break;
        case OTA_SUCCESS:
            bt_conn = 4;
            resend_cmd_bit |= RESEND_CMD_BT_STATUS_BIT;
            app_uart_encode_send(MASTER_SET,CMD_BT_STATUS,(unsigned char *)&bt_conn,sizeof(uint8_t),true);  //发送称体升级成功指令

            if(ota_souce == UPGRADE_PRODUCTION){
                resend_cmd_bit |= RESEND_CMD_FACTORY_STOP_BIT;
                app_uart_encode_send(MASTER_SET,CMD_FACTORY_SYNC_STOP,0,0,true);      //发送称体产测结束指令
                app_handle_production_upgrade_response_result("1547029501529",0);     //升级成功
            }else if(ota_souce == UPGRADE_APP){
                app_handle_upgrade_response_ack("1547029501512",RESPONSE_UPGRADE_SUCCESS,0);
                app_handle_net_service_task_notify_bit(REFRESH_DEVICE_ATTRIBUTE,0,0);
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
    if(len > sizeof(user_fat_data_t))  {
        ESP_LOGE(TAG, "vesync_config_fat len[%d]\r\n" ,len);
        return false;
    }
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
                sprintf(&upgrade_url[url_len],"/%s.V%s%s",PRODUCT_WIFI_NAME,new_version,".bin");
                LOG_I(TAG, "upgrade url %s",upgrade_url);
                app_set_upgrade_source(UPGRADE_APP);
                if((vesync_get_router_link() == false)){
                    vesync_client_connect_wifi((char *)net_info.station_config.wifiSSID, (char *)net_info.station_config.wifiPassword);
                }
                //vesync_client_connect_wifi("R6100-2.4G", "12345678");
                vesync_ota_init(upgrade_url,ota_event_handler);
                //vesync_ota_init("http://192.168.16.25:8888/firmware-debug/esp32/vesync_sdk_esp32.bin",ota_event_handler);
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
    //if(app_handle_get_net_status() > NET_CONFNET_NOT_CON)
    {       //设备已配网
        static bool status = false;
        if(!status){
            status = true;
            vesync_prase_upgrade_url((char *)opt);
        }
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
 * @param info 
 * @param opt 
 * @param len 
 * @return uint8_t 0为操作成功，
 */
static uint8_t vesync_config_account(hw_info *info,uint8_t *opt ,uint8_t len)
{
    uint8_t ret = 0;
    user_config_data_t user_list[MAX_CONUT] ={0};
    uint16_t length =0;

    if(len > (sizeof(user_config_data_t)-2))    return false;
    
    switch(opt[0]){
        case 3:{ //app同步当前用户配置
                uint8_t nlen =0;
                ESP_LOGI(TAG, "sync user\n");
                vesync_flash_read(USER_MODEL_NAMESPACE,USER_MODEL_KEY,(char *)user_list,&length);   //读取当前所有配置用户模型 
                nlen = length/sizeof(user_config_data_t);
                ESP_LOGI(TAG, "nlen[%d]",nlen);
                if(nlen >= 0 && nlen <= MAX_CONUT){
                    memcpy((uint8_t *)&info->user_config_data.action,opt,len);                          //内存保存当前使用的用户模型配置
                    ret = 0;     //修改信息完毕;
                }else{
                    ret = 2;    //超过最大用户下限(0)
                    ESP_LOGE(TAG, "sync user account config is NULL");
                }
            }
            break;
        case 2:{//修改旧账户模型信息
                uint8_t  user_cnt =0;
                uint8_t nlen =0;
                uint32_t if_modyfy_account = *(uint32_t *)&opt[1];
                ESP_LOGI(TAG, "modify user\n");

                vesync_flash_read(USER_MODEL_NAMESPACE,USER_MODEL_KEY,(char *)user_list,&length);   //读取当前所有配置用户模型 
                nlen = length/sizeof(user_config_data_t);
                ESP_LOGI(TAG, "nlen[%d]",nlen);

                if(nlen > 0 && nlen<= MAX_CONUT){
                    for(uint8_t i=0;i< nlen;i++){
                        if(user_list[i].account == if_modyfy_account){
                            user_cnt++;
                            memcpy((uint8_t *)&user_list[i].action,(uint8_t *)&opt[0],len);        //定位当前修改用户模型在flash中的位置并拷贝数据;
                            break;
                        }
                    }
                    if(user_cnt !=0){
                        vesync_flash_erase_all_key(USER_MODEL_NAMESPACE,USER_MODEL_KEY);
                        if(vesync_flash_write(USER_MODEL_NAMESPACE,USER_MODEL_KEY,(uint8_t *)user_list,length)){   //按4字节整数倍保存用户信息
                            ESP_LOGI(TAG, "store user config ok! crc8 =%d len =%d\r\n",info->user_config_data.crc8 ,length);
                            ret = 0;     //修改信息完毕;
                        }else{
                            ret = 1;     //写入flash信息出错;
                        }
                    }else{
                        ret = 3;    //表示当前固件本地用户模型无此用户
                        ESP_LOGE(TAG, "flash user mode not match");
                    }
                }else{
                    ret = 2;    //超过最大用户下限(0)
                    ESP_LOGE(TAG, "change user account config is NULL");
                }
            } 
            break;
        case 1:{ //删除对应的旧账户模型信息
                uint8_t nlen =0;
                uint32_t if_delete_account = *(uint32_t *)&opt[1];
                ESP_LOGI(TAG, "delete user\n");

                vesync_flash_read(USER_MODEL_NAMESPACE,USER_MODEL_KEY,(char *)user_list,&length);   //读取当前所有配置用户模型
                nlen = length/sizeof(user_config_data_t);
                ESP_LOGI(TAG, "nlen[%d]",nlen);

                if(1< nlen && nlen<= MAX_CONUT){
                    uint8_t i=0;
                    bool find = false;
                    for(;i<nlen;i++){
                        if(user_list[i].account == if_delete_account){                    
                            ESP_LOGI(TAG, "delete account[0x%04x]",if_delete_account);    //记录当前的删除位置
                            find = true;
                            break;
                        }
                    }
                    if(find){
                        user_config_data_t *p_buf = (user_config_data_t *)malloc(nlen*sizeof(user_config_data_t));          //当前存储账户大于1个
                        memset(p_buf,0,nlen*sizeof(user_config_data_t));

                        for(uint8_t j=0,k=0;j<nlen;j++,k++){
                            if(j == i){
                                k = k+1;
                            }
                            memcpy((user_config_data_t *)&p_buf[j],(user_config_data_t *)&user_list[k],sizeof(user_config_data_t));
                        }
                        vesync_flash_erase_all_key(USER_MODEL_NAMESPACE,USER_MODEL_KEY);
                        if(vesync_flash_write(USER_MODEL_NAMESPACE,USER_MODEL_KEY,(uint8_t *)&p_buf[0].action,(nlen-1)*sizeof(user_config_data_t))){   //按4字节整数倍保存用户信息
                            ESP_LOGI(TAG, "store user re-flash ok!");
                            ret = 0;
                        }else{
                            ret = 1;
                            ESP_LOGE(TAG, "delete user and write flash error");
                        }
                        free(p_buf);
                    }else{
                        ret = 3;    //表示当前固件本地用户模型无此用户
                        ESP_LOGE(TAG, "flash store user mode not match");
                    }
                }else if(nlen == 1){
                    vesync_flash_erase_key(USER_MODEL_NAMESPACE,USER_MODEL_KEY);
                    ESP_LOGI(TAG, "user account just one!");
                    ret = 0;
                }else{
                    ESP_LOGE(TAG, "delete user account config is NULL");
                    ret = 2;
                }
            }
            break;
        case 0:{//创建新账户模型信息
                uint8_t nlen =0;
                uint8_t  add_cnt =0;
                uint32_t if_add_account = *(uint32_t *)&opt[1];

                ESP_LOGI(TAG, "add user\n");
                vesync_flash_read(USER_MODEL_NAMESPACE,USER_MODEL_KEY,(char *)user_list,&length);   //读取当前所有配置用户模型
                nlen = length/sizeof(user_config_data_t);
                ESP_LOGI(TAG, "nlen[%d]",nlen);

                if(nlen >= 0){
                    if(nlen < MAX_CONUT){
                        for(uint8_t i=0;i<nlen;i++){
                            if(user_list[i].account == if_add_account){
                                add_cnt++;
                                ESP_LOGI(TAG, "match userID[0x%04x],add_cnt=%d,i=%d",user_list[i].account,add_cnt,i);
                                break;
                            }
                        }
                        if(add_cnt ==0){    //当前存储的用户模型与下发的模型没有重合
                            user_config_data_t    user_config_data = {0};
                            memcpy((uint8_t *)&user_config_data.action,(uint8_t *)&opt[0],len);
                            ESP_LOGI(TAG, "account ID[0x%04x]",user_config_data.account);
                            snprintf((char *)user_config_data.user_store_key,sizeof(user_config_data.user_store_key),"his_%x",user_config_data.account); //根据不同的账号创建存储的用户键值对用来保存沉淀数据
                            ESP_LOGI(TAG, "create user key_value[%s]",user_config_data.user_store_key);

                            user_config_data.crc8 = vesync_crc8(0,&user_config_data.action,len);
                            user_config_data.length = len;

                            if(vesync_flash_write(USER_MODEL_NAMESPACE,USER_MODEL_KEY,(uint8_t *)&user_config_data.action,sizeof(user_config_data_t))){   //按4字节整数倍保存用户信息
                                ESP_LOGI(TAG, "store user config ok! crc8 =%d len =%d\r\n",user_config_data.crc8 ,user_config_data.length);
                                ret = 0;         //添加新用户完成
                            }else{
                                ret = 1;         //写flash出错
                                ESP_LOGE(TAG, "user mode ok!store flash error,account:[0x%04x]",if_add_account);
                            }
                        }else{
                            ret = 3;             //下发的用户模型有重合
                            ESP_LOGE(TAG, "user mode is same ,account:[0x%04x]",if_add_account);
                        }
                    }else{
                        ret = 2;                //超过最大用户添加配置
                        ESP_LOGE(TAG, "store flash overflow with len[%d]",nlen);
                    }
                }
            } 
            break;
        default:
            break;
    }
   
    return ret;
}

/**
 * @brief 查询角色沉淀数据 app需要带账户名下发来查询本地用户模型数据库
 * @param len 
 * @return true 
 * @return false 
 */
//0为
//1为账户ID错误
static uint8_t vesync_inquiry_weight_history(uint32_t user_account ,uint16_t *len)
{
    uint32_t ret_value =0;
    user_config_data_t user_list[MAX_CONUT] ={0};

    uint8_t nlen =0;
    uint16_t length =0;

    if(user_account == 0)   return 1;

    ret_value = vesync_flash_read(USER_MODEL_NAMESPACE,USER_MODEL_KEY,(char *)user_list,&length);   //读取当前所有配置用户模型
    nlen = length/sizeof(user_config_data_t);
    ESP_LOGI(TAG, "history data len %d",nlen);
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
            strcpy((char *)inquiry_user_store_key,(char *)user_list[i].user_store_key);
            account_id = user_list[i].account;
            xEventGroupSetBits(ble_event_group, READY_READ_FLASH);
            return 2;
        }else{
            ESP_LOGI(TAG, "read history data but not match user\n");
        }
    }
    return 1;
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
        if(0 == vesync_flash_erase_partiton(USER_MODEL_NAMESPACE)){
            if( 0 == vesync_flash_erase_partiton(USER_HISTORY_DATA_NAMESPACE)){ //同步删除用户数据
                ret = true;
            }
        }
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
    esp_log_buffer_hex(TAG, data_buf, length);
#if 0    
    switch(data_buf[0]){
        case 1:
                app_handle_upgrade_response_ack("1547029501512",RESPONSE_UPGRADE_BUSY);
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
    //         app_handle_net_service_task_notify_bit(NETWORK_CONFIG_REQ,0,0);
    //         break;
    //     case 2:
    //         app_handle_net_service_task_notify_bit(REFRESH_TOKEN_REQ,0,0);
    //         break;
    //     case 3:
    //         app_handle_net_service_task_notify_bit(UPLOAD_WEIGHT_DATA_REQ,0,0);
    //         break;
    //     case 4:
    //         app_handle_net_service_task_notify_bit(UPGRADE_ADDR_REQ,0,0);
    //         break;
    //     case 5:
    //         app_handle_net_service_task_notify_bit(REFRESH_DEVICE_ATTRIBUTE,0,0);
    //         break;
    // }    
    for(unsigned char i=0;i<length;++i){
        if(bt_data_frame_decode(data_buf[i],0,&bt_prase) == 1){
            frame_ctrl_t res_ctl ={     //应答包res状态  
                .data =0,
            };
            struct{                     //应答数据包
                uint8_t  buf[20];
                uint16_t len;
            }resp_strl ={{0},0};         
            uint8_t *cnt = NULL;
            hw_info *info = &info_str;     
            uint8_t *opt = &bt_prase.frame_data[0]; //指针指向data_buf数据域；
            uint8_t len = bt_prase.frame_data_len-sizeof(uint16_t); //长度减去包含2个字节的命令包
            cnt = &bt_prase.frame_cnt;
            inquiry_cnt = bt_prase.frame_cnt;
            static uint32_t if_inquiry_account =0;
            ESP_LOGI(TAG, "bt_prase.frame_ctrl.bitN.ack_flag = %d",bt_prase.frame_ctrl.bitN.ack_flag);

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
                    case CMD_CONFIG_ACCOUNT:{
                            uint8_t ret;
                            ret = vesync_config_account(info,opt,len);
                            if(ret != 0){
                                resp_strl.buf[0] = ret;   //具体产品对应的错误码
                                resp_strl.len = 1;
                                res_ctl.bitN.error_flag = 1;
                            }
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
                            uint8_t ret =0;
                            uint16_t len = 0;
                            if_inquiry_account = *(uint32_t *)&opt[0];
                            ESP_LOGI(TAG, "inquirt account[0x%04x]",if_inquiry_account);
                            ret = vesync_inquiry_weight_history(if_inquiry_account,&len);
                            if(ret == 1){
                                resp_strl.buf[0] = 1;   //具体产品对应的错误码
                                resp_strl.len = 1;
                                res_ctl.bitN.error_flag = 1;
                            }else if(ret == 2){
                                return;
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
                        case CMD_INQUIRY_HISTORY:{
                            uint32_t ack_inquiry_account = *(uint32_t *)&opt[0];
                            ESP_LOGI(TAG, "delete data [%s]",inquiry_user_store_key);
                            if(if_inquiry_account == ack_inquiry_account){
                                vesync_flash_erase_key(USER_HISTORY_DATA_NAMESPACE,inquiry_user_store_key);
                                ESP_LOGI(TAG, "delete success");
                            }
                        }
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

uint32_t app_handle_get_flash_data(const char *label_name,const char *key_name,uint8_t *data,uint32_t *len)
{
    esp_err_t err =0;
    nvs_handle fp;
    uint32_t ret_len =0;
    
    err = nvs_open_from_partition(label_name,key_name, NVS_READONLY, &fp);
    if(err != ESP_OK){
        ESP_LOGE(TAG, "vesync flash fine partion err:0x%04x",err);
        return err;
    }
    err = nvs_get_blob(fp, key_name, NULL, &ret_len); //获取当前键值对存储的数据内容
    if(err != ESP_OK){
        ESP_LOGE(TAG, "vesync flash read size err:0x%04x",err);
        return err;
    }
    if(ret_len !=0){
        err = nvs_get_blob(fp, key_name, data, &ret_len); //获取当前键值对存储的数据内容
        if(err != ESP_OK){
            ESP_LOGE(TAG, "vesync flash read data err:0x%04x",err);
            return err;
        }
        *len = ret_len;
    }
    nvs_close(fp);
    
    return err;
}

static void app_handle_create_packet_head_send(uint32_t account,uint32_t total_len,uint8_t *packet_buf)
{
    uint8_t cnt =0;
    uint8_t send_buf[15] ={0};
    send_buf[0] = 0xA5;
    send_buf[1] = 0x10;  // 状态码
    send_buf[2] = cnt++; // 计数
    *(uint16_t *)&send_buf[3] = 2+total_len+sizeof(account)+sizeof(total_len);
    *(uint16_t *)&send_buf[5] = CMD_INQUIRY_HISTORY; //
    *(uint32_t *)&send_buf[7] = account;
    *(uint32_t *)&send_buf[11] = total_len;

    memcpy(packet_buf,send_buf,sizeof(send_buf));
    vesync_bt_notify_send(send_buf,sizeof(send_buf));
    esp_log_buffer_hex(TAG, packet_buf, sizeof(send_buf));
}

/**
 * @brief 计算累加和
 * @param frame 
 * @param len 
 * @return unsigned char 
 */
static uint8_t ble_sum_verify(uint8_t *frame ,uint16_t len)
{
	uint8_t sum = 0;

	for (uint16_t i = 0; i < len; i++){	//去掉cmd
		sum += frame[i];
	}

	return sum;
}

static void ble_send_inquiry_history_null_data(uint32_t account,uint8_t cnt){
    uint8_t send_buf[17] ={0};
    send_buf[0] = 0xA5;
    send_buf[1] = 0x10;  // 状态码
    send_buf[2] = cnt; // 计数
    *(uint16_t *)&send_buf[3] = 8+2;
    *(uint16_t *)&send_buf[5] = CMD_INQUIRY_HISTORY; //
    *(uint32_t *)&send_buf[7] = account;
    *(uint32_t *)&send_buf[11] = 0;
    send_buf[15] = ble_sum_verify(&send_buf[1],sizeof(send_buf)-1);
    send_buf[16] = 0x5A;
    vesync_bt_notify_send(send_buf,sizeof(send_buf));
    esp_log_buffer_hex(TAG, send_buf, sizeof(send_buf));
}
/**
 * @brief 蓝牙数据发送任务
 * @param pvParameters 
 */
static void app_handle_ble_send_task_handler(void *pvParameters)
{
    uint8_t user_read_data[6000] ={0};
    uint16_t read_len =0;
    static uint8_t gUserConfig =0;
    static uint16_t send_len =0;
    static uint16_t total_size =0;
    static uint16_t rst_len =0;
    static uint8_t packet_buf[15]={0};
    static uint8_t head_checksum8 =0;

    frame_ctrl_t res_ctl ={     //应答包res状态  
                .data = 0x10,
            };
    static uint8_t cnt = 1;
    uint16_t upgrade_cmd = CMD_INQUIRY_HISTORY;

    while(1){
        int bits = xEventGroupWaitBits(ble_event_group, READY_READ_FLASH,
	 			false, false, portMAX_DELAY);	//第一个在返回之前需要将WR_SD_BIT清除, 第二个false 不用等待所有的bit都要置位;

        if(bits & READY_READ_FLASH){
            switch(gUserConfig){
                case 0: //查询flash数据状态
                    ESP_LOGI(TAG, "read user key_value[%s]",inquiry_user_store_key);
                    if(app_handle_get_flash_data(USER_HISTORY_DATA_NAMESPACE,inquiry_user_store_key,user_read_data,&total_size) != 0){
                        xEventGroupClearBits(ble_event_group, READY_READ_FLASH);
                        ble_send_inquiry_history_null_data(account_id,inquiry_cnt);
                    }else{
                        gUserConfig = 1; 
                    }
                    ESP_LOGI(TAG, "history data %d",total_size);
                    break;
                case 1: //当前内存数据已拷贝完整存储的用户数据并已发送包头信息
                        app_handle_create_packet_head_send(account_id,total_size,packet_buf);
                        head_checksum8 = ble_sum_verify(&packet_buf[1],sizeof(packet_buf)-1);
                        gUserConfig = 2; 
                    break;
                case 2: //发送数据载荷
                    rst_len = total_size- send_len;
                    ESP_LOGI(TAG, "rst_len:%d",rst_len);
                    if(rst_len > 20){
                        esp_log_buffer_hex(TAG, &user_read_data[send_len], 20);
                        vesync_bt_notify_send(&user_read_data[send_len],20);
                        send_len +=20;
                    }else{
                        vesync_bt_notify_send(&user_read_data[send_len],rst_len);
                        esp_log_buffer_hex(TAG, &user_read_data[send_len], rst_len);
                        send_len += rst_len;
                        gUserConfig = 3;
                        if(send_len == total_size){
                            ESP_LOGI(TAG, "send data over!!!!!!");
                        }
                        send_len =rst_len =total_size =0;
                    }
                    ESP_LOGI(TAG, "\r\nhistory total len:%d send_len :%d, rst_len:%d\r\n",total_size,send_len,rst_len);
                    break;
                case 3:{    //计算checksum
                        uint8_t databuf[2]={0};
                        databuf[0] = (ble_sum_verify(user_read_data,total_size) + head_checksum8);
                        databuf[1] = 0x5A;
                        gUserConfig = 0; 
                        vesync_bt_notify_send(databuf,sizeof(databuf));
                        esp_log_buffer_hex(TAG, databuf, sizeof(databuf));
                        xEventGroupClearBits(ble_event_group, READY_READ_FLASH);
                        ESP_LOGI(TAG, "history data send finish crc %d",databuf[0]);
                    }
                    break;
            }
        }
        vTaskDelay(50 / portTICK_PERIOD_MS);	//正常使用10ms；
     }
    vTaskDelete(NULL);
}

/**
 * @brief 初始化蓝牙广播,使能蓝牙配网服务
 */
void app_ble_init(void)
{
    char bt_version[8] = {0};
    char version[8];
    strcpy(version,FIRM_VERSION);
    sprintf(bt_version,"%c%c%c%c",version[0],version[2],version[4],version[5]);
	vesync_bt_client_init(PRODUCT_NAME,PRODUCT_VER,bt_version,PRODUCT_TYPE,PRODUCT_NUM,NULL,true,app_bt_set_status,app_ble_recv_cb);
    vesync_bt_advertise_start(APP_ADVERTISE_TIMEOUT);

    ble_event_group= xEventGroupCreate();
    xTaskCreate(app_handle_ble_send_task_handler, "app_handle_ble_send_task_handler", 8192, NULL, 8, NULL);
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