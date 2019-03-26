/**
 * @file app_public_events.c
 * @brief 处理和网络对接相关
 * @author Jiven
 * @date 2018-12-21
 */

#include "vesync_flash.h"
#include "vesync_production.h"
#include "vesync_net_service.h"
#include "vesync_interface.h"
#include "vesync_wifi.h"
#include "vesync_ota.h"
#include "esp_task_wdt.h"

#include "app_body_fat_calc.h"
#include "app_handle_phone.h"
#include "app_handle_server.h"
#include "app_handle_scales.h"
#include <time.h>

#include "vesync_ca_cert.h"
#include "vesync_https.h"

#include "vesync_build_cfg.h"
#include "vesync_log.h"

static const char *TAG = "app_handle_server";
static char trace_time[14] = {'\0'};
static char production_upgrade_url[128] = {'\0'};
static char new_version[8] = {'\0'};
factory_test_bit_t factory_test_bit = 0;

#define HTTPS_QUEUE_LEN 300
typedef struct{
    uint16_t len;
    uint8_t  buff[HTTPS_QUEUE_LEN];
}https_send_frame_t;

static xQueueHandle https_message_send_queue = NULL;

static TaskHandle_t s_network_service_taskhd = NULL;

static device_net_status_t device_net_status = NET_CONFNET_NOT_CON;		//设备配网状态，默认为离线状态

void app_handle_production_upgrade_response_ack(char *trace_id);
void app_handle_production_cid_response(void);

static uint8_t upgrade_souce = UPGRADE_NULL;

/**
 * @brief 设置升级来源
 * @param source 
 */
void app_set_upgrade_source(uint8_t source)
{
    upgrade_souce = source;
    
    app_sale_wakeup(false);        //激活点亮屏幕显示
    app_bt_wifi_suspend_stop();     //禁止称体休眠
    app_enter_scale_suspend_stop(); //禁止WIFI休眠
}
/**
 * @brief 获取升级来源
 * @return uint8_t 
 */
uint8_t app_get_upgrade_source(void)
{
    return upgrade_souce;
}
/**
 * @brief 返回trace_id
 * @param time 
 */
char *vesync_get_time(void)
{
    return &trace_time[0];
}
/**
 * @brief 接收json数据回调函数
 * @param data [原始格式的json数据]
 */
void vesync_recv_json_data(char *data)
{
    LOG_I(TAG, "User recv : %s", data);
    cJSON *root = cJSON_Parse(data);
    if(NULL == root){
        LOG_E(TAG, "Parse cjson error !");
        return;
    }
    vesync_printf_cjson(root);

    cJSON *trace_Id = cJSON_GetObjectItemCaseSensitive(root, "traceId");
    if(true == cJSON_IsString(trace_Id)){
        strcpy(trace_time, trace_Id->valuestring);
        LOG_I(TAG, "trase_Id %s", trace_time);
    }else{
        LOG_E(TAG, "prase trase_Id error");
    }
    
    cJSON *jsonCmd = cJSON_GetObjectItemCaseSensitive(root, "jsonCmd");
    if(true == cJSON_IsObject(jsonCmd))
    {
        cJSON *switch_item = cJSON_GetObjectItemCaseSensitive(jsonCmd, "switch");
        if(true == cJSON_IsString(switch_item)){
            if(!strcmp(switch_item->valuestring, "on")){
                LOG_I(TAG, "project test start!");
                app_handle_production_response_ack(vesync_get_time(),PRODUCT_TEST_START);
            }
        }
        cJSON *button_item = cJSON_GetObjectItemCaseSensitive(jsonCmd, "button");
        if(true == cJSON_IsString(button_item)){
            if(!strcmp(button_item->valuestring, "on")){
                LOG_I(TAG, "button test start");
                factory_test_bit |= FACTORY_TEST_SYNC_BUTTON_BIT;
                //app_handle_production_report_button(trace_time,3);
            }
        }
        cJSON *charge_item = cJSON_GetObjectItemCaseSensitive(jsonCmd, "charge");
        if(true == cJSON_IsString(charge_item)){
            if(!strcmp(charge_item->valuestring, "on")){
                LOG_I(TAG, "charge test start!");
                factory_test_bit |= FACTORY_TEST_SYNC_CHARGE_BIT;
                resend_cmd_bit |= RESEND_CMD_FACTORY_CHARGE_BIT;
                app_uart_encode_send(MASTER_SET,CMD_FACTORY_CHARGING,0,0,true);
                //app_handle_production_report_charge(trace_time,1,80);
            }
        }
        cJSON *weight_item = cJSON_GetObjectItemCaseSensitive(jsonCmd, "weight");
        if(true == cJSON_IsString(weight_item)){
            if(!strcmp(weight_item->valuestring, "on")){
                LOG_I(TAG, "weight test start!");
                vesync_set_production_status(RPODUCTION_RUNNING);   //状态调整为产测模式已开始;
                resend_cmd_bit |= RESEND_CMD_FACTORY_WEIGHT_BIT;
                factory_test_bit |= FACTORY_TEST_SYNC_WEIGHT_BIT;
                app_uart_encode_send(MASTER_SET,CMD_FACTORY_WEIGHT,0,0,true);
                //app_handle_production_report_weight(trace_time,10000,500);
            }
        }
        cJSON *bt_item = cJSON_GetObjectItemCaseSensitive(jsonCmd, "bt");
        if(true == cJSON_IsString(bt_item)){
            if(!strcmp(bt_item->valuestring, "on")){
                LOG_I(TAG, "bt test start!");
                factory_test_bit |= FACTORY_TEST_SYNC_BT_BIT;
                //app_handle_production_response_bt_rssi(trace_time,-10);
                app_product_ble_start();
            }
        }
        cJSON *firmware = cJSON_GetObjectItemCaseSensitive(jsonCmd, "firmware");
        if(firmware != NULL){
            LOG_I(TAG, "upgrade test start!");
            cJSON* newVersion = cJSON_GetObjectItemCaseSensitive(firmware, "newVersion");
            if(cJSON_IsString(newVersion)){
				strcpy(new_version, newVersion->valuestring);	//记录升级的新版本
                LOG_I(TAG, "upgrade new_version %s",new_version);
			}
            cJSON* url = cJSON_GetObjectItemCaseSensitive(firmware, "url");
            if(cJSON_IsString(url)){
				if(NULL != production_upgrade_url){
                    uint8_t url_len;
                    app_set_upgrade_source(UPGRADE_PRODUCTION);
					strcpy(production_upgrade_url, url->valuestring);
                    url_len = strlen(url->valuestring);
                    sprintf(&production_upgrade_url[url_len],"%s.V%s%s",PRODUCT_WIFI_NAME,new_version,".bin");
                    LOG_I(TAG, "upgrade url %s",production_upgrade_url);
                    app_handle_production_upgrade_response_ack(trace_time);
                    vesync_hal_bt_client_deinit();
                    vesync_ota_init(production_upgrade_url,ota_event_handler);
				}
			}
        }
        cJSON *cid = cJSON_GetObjectItemCaseSensitive(jsonCmd, "cid");
        if(true == cJSON_IsString(cid)){
            int32_t ret;
            LOG_I(TAG, "Get cid success !");
            strcpy((char *)product_config.cid, cid->valuestring);
            // ret = nvs_flash_erase();    //删除"nvs"分区对应的所有内容;
            // if(ret != 0){
            //     LOG_E(TAG, "erase nvs\r\n");
            // }
            // ret = nvs_flash_init();
            // if(ret != 0){
            //     LOG_E(TAG, "nvs_flash_init\r\n");
            // }
            if(vesync_flash_write_product_config(&product_config) == 0){
                app_handle_production_cid_response();
                vesync_set_production_status(PRODUCTION_EXIT);   //状态调整为产测模式已结束;

                app_scale_suspend_start();

                resend_cmd_bit |= RESEND_CMD_FACTORY_STOP_BIT;
                app_uart_encode_send(MASTER_SET,CMD_FACTORY_SYNC_STOP,0,0,true);
                vesync_mqtt_client_disconnet_from_cloud();
            }
        }else
            LOG_E(TAG, "Get cid error !");
        
        cJSON_Delete(root);									//务必记得释放资源！
    }
    else
        LOG_E(TAG, "Get jsonCmd error !");
}

/**
 * @brief 主动发起升级结果
 * @param result 
 * @param trace_id 
 */
void app_handle_production_upgrade_response_result(char *trace_id,uint8_t result)
{
    cJSON *root = cJSON_CreateObject();
    if(NULL != root){
        cJSON* firmware = NULL;
        cJSON_AddItemToObject(root, "firmware", firmware = cJSON_CreateObject());
        if(NULL != firmware){
            cJSON_AddStringToObject(firmware, "newVersion",new_version);
            cJSON_AddNumberToObject(firmware, "status", result);
            cJSON_AddStringToObject(firmware, "url", production_upgrade_url);
        }
        cJSON *report = vesync_json_add_method_head(trace_id,"reportFirmUp",firmware);
        vesync_printf_cjson(report);
        char* out = cJSON_PrintUnformatted(report);
        vesync_publish_production_data(out, MQTT_QOS1, 0);
        free(out);
        cJSON_Delete(report);
    }
    cJSON_Delete(root);
}
/**
 * @brief 应答服务器升级开始
 * @param json 
 */
void app_handle_production_upgrade_response_ack(char *trace_id)
{
    cJSON *report = cJSON_CreateObject();
    LOG_I(TAG, "response trace_id : %s",trace_id);
    if(NULL != report){
        cJSON_AddStringToObject(report, "traceId", trace_id);		//==TODO==，需要修改成毫秒级
        cJSON_AddNumberToObject(report, "code", 0);
        cJSON_AddStringToObject(report, "msg", "firmware set ok");
    }
    vesync_printf_cjson(report);
    char* out = cJSON_PrintUnformatted(report);
    vesync_response_production_command(out, MQTT_QOS1, 0);
    free(out);

    cJSON_Delete(report);
}

/**
 * @brief 应答cid
 */
void app_handle_production_cid_response(void)
{
    cJSON *report = cJSON_CreateObject();
    if(NULL != report){
        cJSON_AddStringToObject(report, "traceId", trace_time);
        cJSON_AddNumberToObject(report, "code", 0);
        cJSON_AddStringToObject(report, "msg", "cid set ok");
    }
    char* out = cJSON_PrintUnformatted(report);
    LOG_I(TAG, "Response server : %s",out);
    vesync_response_production_command(out, MQTT_QOS1, 0);
    free(out);
    cJSON_Delete(report);
}

/**
 * @brief 应答服务器下发
 * @param test_item 
 */
void app_handle_production_response_ack(char *trace_id,uint8_t test_item)
{
    cJSON *root = cJSON_CreateObject();
    if(NULL != root)
    {
        cJSON* info = NULL;
        cJSON_AddItemToObject(root, "info", info = cJSON_CreateObject());
        if(NULL != info)
        {
            switch(test_item){
                case PRODUCT_TEST_START:
                    cJSON_AddStringToObject(info, "switch", "start");
                    break;
                default:
                    break;
            }
            cJSON *report = vesync_json_add_method_head(trace_id,"updateDevInfo",info);
            vesync_printf_cjson(report);
            char* out = cJSON_PrintUnformatted(report);
            vesync_response_production_command(out, MQTT_QOS1, 0);
            free(out);
            cJSON_Delete(report);
        }
    }
    cJSON_Delete(root);
}

/**
 * @brief 上报产测时的按键测试次数
 * @param times [按键测试的次数]
 */
void app_handle_production_report_button(char *trace_id,int times)
{
    cJSON *root = cJSON_CreateObject();
    if(NULL != root)
    {
        cJSON* info = NULL;
        cJSON_AddItemToObject(root, "info", info = cJSON_CreateObject());
        if(NULL != info)
        {
            cJSON_AddNumberToObject(info, "button", times);
            cJSON *report = vesync_json_add_method_head(trace_id,"updateDevInfo",info);
            vesync_printf_cjson(report);
            char* out = cJSON_PrintUnformatted(report);
            vesync_response_production_command(out, MQTT_QOS1, 0);
            free(out);
            cJSON_Delete(report);
        }
    }
    cJSON_Delete(root);
}

/**
 * @brief 上报产测时的充电状态
 * @param charge_status [充电状态，1 - 表示正在充电；0 - 表示未充电]
 * @param power_percent [当前电量百分比数值]
 */
void app_handle_production_report_charge(char *trace_id,int charge_status, int power_percent)
{
    cJSON *root = cJSON_CreateObject();
    if(NULL != root)
    {
        cJSON* info = NULL;
        cJSON_AddItemToObject(root, "info", info = cJSON_CreateObject());
        if(NULL != info)
        {
            cJSON_AddNumberToObject(info, "charge", charge_status);
            cJSON_AddNumberToObject(info, "percent", power_percent);
            cJSON *report = vesync_json_add_method_head(trace_id,"updateDevInfo",info);
            vesync_printf_cjson(report);
            char* out = cJSON_PrintUnformatted(report);
            vesync_response_production_command(out, MQTT_QOS1, 0);
            free(out);
            cJSON_Delete(report);
        }
    }
    cJSON_Delete(root);
}

/**
 * @brief 上报产测时的称重结果
 * @param weight [实时称重的结果，单位kg]
 * @param imped  [实时称重的阻抗，单位欧姆]
 */
void app_handle_production_report_weight(char *trace_id,int weight, int imped)
{
    cJSON *root = cJSON_CreateObject();
    if(NULL != root)
    {
        cJSON* info = NULL;
        cJSON_AddItemToObject(root, "info", info = cJSON_CreateObject());
        if(NULL != info)
        {
            cJSON_AddNumberToObject(info, "weight", weight);
            cJSON_AddNumberToObject(info, "imped", imped);
            cJSON *report = vesync_json_add_method_head(trace_id,"updateDevInfo",info);
            vesync_printf_cjson(report);
            char* out = cJSON_PrintUnformatted(report);
            vesync_response_production_command(out, MQTT_QOS1, 0);
            free(out);
            cJSON_Delete(report);
        }
    }
    cJSON_Delete(root);
}

/**
 * @brief 响应回复服务器蓝牙的rssi值
 * @param rssi [蓝牙信号的rssi值]
 */
void app_handle_production_response_bt_rssi(char *trace_id,int rssi)
{
    cJSON *root = cJSON_CreateObject();
    if(NULL != root)
    {
        cJSON* info = NULL;
        cJSON_AddItemToObject(root, "info", info = cJSON_CreateObject());
        if(NULL != info)
        {
            cJSON_AddNumberToObject(info, "bt-rssi", rssi);
            cJSON *report = vesync_json_add_method_head(trace_id,"updateDevInfo",info);
            vesync_printf_cjson(report);
            char* out = cJSON_PrintUnformatted(report);
            vesync_response_production_command(out, MQTT_QOS1, 0);
            free(out);
            cJSON_Delete(report);
        }
    }
    cJSON_Delete(root);
}

/**
 * @brief 获取当前设备配网状态
 * @return device_net_status_t 
 */
device_net_status_t app_handle_get_net_status(void)
{
    LOG_I(TAG, "app_handle_get_net_status is %d", device_net_status);
    return device_net_status;
}

/**
 * @brief 设置当前配网状态
 * @param new_status 
 */
void app_handle_set_net_status(device_net_status_t new_status)
{
    if(new_status != device_net_status){
        device_net_status = new_status;
    }
}

/**
 * @brief 应用层网络服务通知
 */
void app_handle_net_service_task_notify_bit(uint32_t bit ,void *data,uint16_t len)
{
    // https_send_frame_t https_send_frame ={0};

    // //vesync_client_connect_wifi((char *)net_info.station_config.wifiSSID, (char *)net_info.station_config.wifiPassword);
    // if(len !=0){
    //     https_send_frame.len = len;
    //     memcpy((char *)https_send_frame.buff,(char *)data,len);
    //     xQueueSend(https_message_send_queue,&https_send_frame,portTICK_PERIOD_MS);
    // }
    //LOG_I(TAG, "app_handle_server_task_handler send bit[0x%08x]", bit);
    xTaskNotify(s_network_service_taskhd, bit, eSetBits);			//通知事件处理中心任务
}

static uint8_t app_json_https_service_parse(uint32_t mask,char *read_buf)
{
	char *index = strchr(read_buf,'{');	//截取有效json
	char trace_id[14] ={"\0"};

	cJSON *root = cJSON_Parse(index);
	uint8_t ret = 1;
	if(NULL == root){
		LOG_I(TAG,"Parse cjson error !\r\n");
		return ret;
	}

	cJSON *traceId = cJSON_GetObjectItemCaseSensitive(root, "traceId");
	if(true == cJSON_IsString(traceId)){
		LOG_I(TAG,"trace_id : %s", traceId->valuestring);
		strcpy(trace_id,traceId->valuestring);
	}

	cJSON *code = cJSON_GetObjectItemCaseSensitive(root, "code");
	if(true == cJSON_IsNumber(code)){
		LOG_I(TAG,"code : %d", code->valueint);
        if(code->valueint == 0){
            ret = 0;
            cJSON *result = cJSON_GetObjectItemCaseSensitive(root, "result");
            if(true == cJSON_IsObject(result)){
                cJSON *token = cJSON_GetObjectItemCaseSensitive(result, "token");
                if(true == cJSON_IsString(token)){
                    vesync_flash_write_token_config(token->valuestring);
                    LOG_I(TAG,"token : %s", token->valuestring);
                }
            }
        }else if(code->valueint == -11001005){  // token 过期
            ret = 3;
        }else{
            ret = 4;
        }
	}else{
        ret = 2;
        LOG_E(TAG,"app https upload error \r\n");
    }
	cJSON_Delete(root);	

	return ret;
}


static void vesync_store_match_user_req(void)
{
    static uint8_t gUserUploadStep =0;
    static uint8_t user_read_data_buff[4096] ={0};
    static uint16_t total_size =0;
    static uint8_t  array_len = 0;
    
    switch(gUserUploadStep){
        case 0: //先查询当前用户是否存在沉淀数据未上传;
                app_handle_get_flash_data(USER_HISTORY_DATA_NAMESPACE,mask_user_store_key,user_read_data_buff,&total_size);
                array_len = total_size/sizeof(user_history_t);
                
                gUserUploadStep = 1;
                app_handle_net_service_task_notify_bit(STORE_WEIGHT_DATA_REQ,NULL,0);  //返回继续检索下一个用户
            break;
        case 1:
            gUserUploadStep = 0;
            if(array_len < USER_STORE_LEN){    //判断沉淀数据是否超出范围
                if(!vesync_flash_write(USER_HISTORY_DATA_NAMESPACE,mask_user_store_key,(user_history_t *)&info_str.user_history_data ,sizeof(user_history_t))){
                    ESP_LOGE(TAG, "write time data store history data error!!"); 
                }
            }else{
                ESP_LOGE(TAG, "history data max 100!!!!!");
            }
            break;
        default:
            break;
    }
}


#define HTTPS_SEND_MAX  30

/**
 * @brief 上电遍历所有用户模型信息及同步数据至云端;
 */
static void vesync_send_all_data_https_req(void)
{
    cJSON *root = cJSON_CreateObject();
    char *req_method = NULL;
    cJSON* info = NULL;
    static char token[128];
    static uint8_t gUserUploadStep =0;
    int ret =0;
    static uint8_t rst_len =0;
    static uint8_t send_len =0;
    static uint16_t total_size =0;
    static uint8_t  array_len = 0;
    static uint8_t user_read_data_buff[4096] ={0};
    static uint8_t  calc_len = 0;
    static uint8_t  usermode_len =0;
    static uint16_t length =0;
    static uint8_t  send_usermode_key_cnt=0;
    static char current_send_user_key[12]={"\0"};
    static uint8_t o_mask = 0 ;

    if(o_mask == 0){    //上传数据
        req_method = "/fatScale/uploadWeighData";
        ESP_LOGI(TAG, "gUserUploadStep %d",gUserUploadStep);

        switch(gUserUploadStep){
            case 0:{ //先读取flash数据和总长度
                    vesync_flash_read(USER_MODEL_NAMESPACE,USER_MODEL_KEY,(char *)info_str.user_config_list,&length);   //取出所有用户模型信息
                    info_str.user_config_list_mode_len = length;                                                             //取出所有用户模型信息总长度
                    usermode_len = length/sizeof(user_config_data_t);
                    ESP_LOGI(TAG, "usermode_len[%d]",usermode_len);

                    if(usermode_len >= 0 && usermode_len <= MAX_CONUT){
                        gUserUploadStep = 1;    
                        send_usermode_key_cnt =0;   //轮巡用户模型key的计数;
                        app_handle_net_service_task_notify_bit(UPLOAD_ALL_USER_DATA_REQ,NULL,0);    //根据用户键值对轮寻沉淀数据
                    }else{
                        gUserUploadStep =0;
                    }
                    return;
                }
                break;
            case 1:{//遍历所有用户模型键值并取出键值对应的单个用户沉淀数据
                    if(send_usermode_key_cnt < usermode_len){
                        uint8_t i =0;
                        for(;i< usermode_len;i++){  //先遍历所有用户模型键值
                            ESP_LOGI(TAG, "user mode key[%d][%s] ",i,info_str.user_config_list[i].user_store_key);
                        }
                        strcpy(current_send_user_key,info_str.user_config_list[send_usermode_key_cnt].user_store_key);
                        ESP_LOGI(TAG, "======================================");
                        ESP_LOGI(TAG, "current send user mode cnt:%d,key[%s]",send_usermode_key_cnt,current_send_user_key);
                        ESP_LOGI(TAG, "======================================");

                        if(vesync_flash_read(USER_HISTORY_DATA_NAMESPACE,current_send_user_key,user_read_data_buff,&total_size) == 0){//遍历每个用户的沉淀数据
                            array_len = total_size/sizeof(user_history_t);  //获取单个用户的所有沉淀数据总条数;
                            rst_len = array_len;
                            calc_len =0;                            
                            if(rst_len <= HTTPS_SEND_MAX){  //最大发送30条数据，30*12 =360byte
                                send_len = rst_len;
                            }else{
                                send_len = HTTPS_SEND_MAX;
                            }
                        }else{  //当前只有用户模型，没有用户沉淀数据,继续轮寻下一个用户;
                            send_usermode_key_cnt++;
                            if(send_usermode_key_cnt >= usermode_len){  //所有用户检索完毕，退出
                                gUserUploadStep =0;
                                send_usermode_key_cnt = 0;
                                ESP_LOGI(TAG, "all use mode check success!!!!!");
                                //vesync_client_wifi_module_deinit();
                            }else{
                                app_handle_net_service_task_notify_bit(UPLOAD_ALL_USER_DATA_REQ,NULL,0);
                            }
                            return;
                        }
                    }else{
                        gUserUploadStep =0;
                        send_usermode_key_cnt = 0;
                        ESP_LOGE(TAG, "user history data NULL and close ");
                        return;
                    }
                    ESP_LOGI(TAG, "history data %d , array_len %d ,send_len %d",total_size , array_len,send_len);
                }
                break;
            case 2:
                if(rst_len <= HTTPS_SEND_MAX){
                    send_len = rst_len;
                }else{
                    send_len = HTTPS_SEND_MAX;
                }
                break;
            default:
                break;
        }
        /*******************************************************************************/
        /**************************单个用户沉淀数据封包开始*********************************/
        cJSON* history_weight[send_len];
        cJSON* data = NULL;
        char account[10]={"\0"};

        sprintf(account,"%d",info_str.user_config_list[send_usermode_key_cnt].account);
        cJSON_AddItemToObject(root, "info", info = cJSON_CreateObject());
        cJSON_AddStringToObject(info, "accountID", account);
        cJSON_AddItemToObject(info,"data",data = cJSON_CreateArray());  //创建数组

        if(NULL != info){
            user_history_t *user_history = (user_history_t *)malloc(total_size);
            memcpy((uint8_t *)&user_history->imped_value,(uint8_t *)&user_read_data_buff[sizeof(user_history_t)*calc_len],sizeof(user_history_t)*send_len);
            
            if(NULL != data){
                for(uint16_t i=0;i<send_len;i++){
                    cJSON_AddItemToArray(data, history_weight[i] = cJSON_CreateObject());/* 给创建的数组增加对象*/
                    if(NULL != history_weight[i]){
                        cJSON_AddNumberToObject(history_weight[i],"weigh_kg",user_history[i].weight_kg);
                        cJSON_AddNumberToObject(history_weight[i],"weigh_lb",user_history[i].weight_lb);
                        cJSON_AddNumberToObject(history_weight[i],"impedence",user_history[i].imped_value);
                        cJSON_AddNumberToObject(history_weight[i],"weighTime",user_history[i].utc_time);
                        cJSON_AddNumberToObject(history_weight[i],"unit",user_history[i].measu_unit);
                        cJSON_AddNumberToObject(history_weight[i],"timezone15m",user_history[i].time_zone);
                    }
                }
            }
            free(user_history);
        }
        /**************************单个用户沉淀数据封包结束*********************************/
        /*******************************************************************************/
    }else if(o_mask == 1){  //刷新token
        req_method = "refreshDeviceToken";
        info = NULL;
        LOG_E("https", "REFRESH_TOKEN_REQ");
    }

    /*******************************************************************************/
    /**************************单包用户沉淀数据准备发送*********************************/
    if(vesync_get_device_status() >= DEV_CONFIG_NET_RECORDS){   //有配网记录才开启沉淀数据上传
        if((vesync_get_router_link() == false)){                //如果未连接路由器 开启连接
            vesync_client_connect_wifi((char *)net_info.station_config.wifiSSID, (char *)net_info.station_config.wifiPassword);
        }
        time_t seconds;
        seconds = time((time_t *)NULL);
        char traceId_buf[64];
        itoa(seconds, traceId_buf, 10);

        cJSON *report = vesync_json_add_method_head(traceId_buf,req_method,info);
        if(o_mask == 0){
            vesync_get_https_token(token);
            cJSON_AddStringToObject(report, "token", token);
        }
        char* out = cJSON_PrintUnformatted(report);
        LOG_I("JSON", "https send packet len[%d],   %s", strlen(out),out);

        char recv_buff[300];    //服务器应答最大数据包
        int buff_len = sizeof(recv_buff);

        ret = vesync_https_client_request(req_method, out, recv_buff, &buff_len, 10000);
        if(buff_len > 0 && ret == 0){
            uint8_t if_resend;
            LOG_I(TAG, "Https recv %d byte data : \n%s", buff_len, recv_buff);
            if_resend = app_json_https_service_parse(UPLOAD_ALL_USER_DATA_REQ,recv_buff);   //解析https沉淀数据上传结果;

            if(if_resend == 0){ //数据上报成功
                uint32_t ret = 0;
                o_mask =0;

                rst_len -= send_len;
                calc_len += send_len;

                if(rst_len == 0){//同一用户沉淀数据上传完毕
                    calc_len = 0;
                    LOG_I(TAG, "current user data has send and next user mode %d ,delete user key[%s]" ,send_usermode_key_cnt,current_send_user_key);
                    if(send_usermode_key_cnt >= usermode_len){//所有用户沉淀数据上传完毕
                        gUserUploadStep = 0;
                        ret = vesync_flash_erase_partiton(USER_HISTORY_DATA_NAMESPACE);//擦除所有用户沉淀数据区key
                        if(ret != 0){
                            LOG_E(TAG, "erase USER_HISTORY_DATA_NAMESPACE\r\n");
                        }
                        //vesync_client_wifi_module_deinit();
                        LOG_I(TAG, "all user data has send over!!!!!!!!!!!");
                    }else{  //继续轮训下一个用户的数据
                        gUserUploadStep = 1;
                        send_usermode_key_cnt++;
                        vesync_flash_erase_key(USER_HISTORY_DATA_NAMESPACE,current_send_user_key);//擦除单一用户沉淀数据key
                        app_handle_net_service_task_notify_bit(UPLOAD_ALL_USER_DATA_REQ,NULL,0);  //返回继续检索下一个用户
                        LOG_I(TAG, "net user %d",send_usermode_key_cnt);
                    }
                }else{
                    gUserUploadStep = 2;    //继续轮训同一用户的剩余数据;
                    app_handle_net_service_task_notify_bit(UPLOAD_ALL_USER_DATA_REQ,NULL,0);
                    LOG_I(TAG, "resend weight data to server");
                }
                ESP_LOGI(TAG, "history data %d , array_len %d ,send_len %d rst_len %d",total_size , array_len,send_len,rst_len);
            }else if(if_resend == 3){ //token过期
                LOG_E(TAG, "server report token error!!!!");
                gUserUploadStep = 1;
                send_usermode_key_cnt =0;
                o_mask = 1;
                //vesync_refresh_https_token();
                app_handle_net_service_task_notify_bit(UPLOAD_ALL_USER_DATA_REQ,NULL,0);
            }
        }
        ESP_LOGI(TAG, "gUserUploadStep end %d",gUserUploadStep);
        free(out);
        cJSON_Delete(report);
        cJSON_Delete(root);
    }else{
        o_mask = 0;
        gUserUploadStep = 0;
    }
}

/**
 * @brief 
 * @param databuf 
 * @param len 
 * @param mask 
 */
static void vesync_send_match_user_data_https_req(void)
{
    static uint8_t gUserUploadStep =0;
    int ret =0;
    static uint8_t rst_len =0;
    static uint8_t send_len =0;
    static uint32_t total_size =0;
    static uint8_t  array_len = 0;
    static uint8_t user_read_data[4096] ={0};
    static uint8_t  calc_len = 0;
    static uint8_t mask = 0;
    char *req_method = NULL;

    if(mask == 0){
        req_method = "/fatScale/uploadWeighData";
        ESP_LOGI(TAG, "read user key_value[%s]",mask_user_store_key);
        ESP_LOGI(TAG, "gUserUploadStep %d",gUserUploadStep);

        switch(gUserUploadStep){
            case 0: //先查询当前用户是否存在沉淀数据未上传;
                    if((app_handle_get_flash_data(USER_HISTORY_DATA_NAMESPACE,mask_user_store_key,user_read_data,&total_size) == 0)){
                        array_len = total_size/sizeof(user_history_t);
                        rst_len = array_len;
                        calc_len =0;   

                        if(rst_len <= HTTPS_SEND_MAX){  //最大发送30条数据，30*12 =360byte
                            send_len = rst_len;
                        }else{
                            send_len = HTTPS_SEND_MAX;
                        }
                        gUserUploadStep = 2;
                    }else{
                        gUserUploadStep = 1;
                        app_handle_net_service_task_notify_bit(UPLOAD_WEIGHT_DATA_REQ,0,0);
                        return;
                    }
                    ESP_LOGI(TAG, "history data %d , array_len %d ,send_len %d",total_size , array_len,send_len);
                break;
            case 1:{//发送实时数据
                    memcpy((user_history_t *)&user_read_data,(user_history_t *)&info_str.user_history_data,sizeof(user_history_t));
                    total_size = sizeof(user_history_t);
                    array_len = total_size/sizeof(user_history_t);
                    rst_len = array_len;
                    calc_len =0;
                    send_len = rst_len;

                    ESP_LOGI(TAG, "real data %d , array_len %d ,send_len %d",total_size , array_len,send_len);
                }
                break;
            case 2://数据发送区
                if(rst_len <= HTTPS_SEND_MAX){
                    send_len = rst_len;
                }else{
                    send_len = HTTPS_SEND_MAX;
                }
                break;
            default:
                break;
        }
    }else if(mask == 1){
        req_method = "refreshDeviceToken";
        LOG_E("https", "REFRESH_TOKEN_REQ");
    }

    /****************************数据封包开始**********************************/
    cJSON *root = cJSON_CreateObject();
    cJSON *info = NULL;
    static char token[128];

    if(mask == 0){
        char account_id_str[10] ={"\0"};
        cJSON* history_weight[send_len];
        cJSON* data = NULL;
        sprintf(account_id_str,"%d",match_account_id);

        cJSON_AddItemToObject(root, "info", info = cJSON_CreateObject());
        cJSON_AddStringToObject(info, "accountID", account_id_str);
        cJSON_AddItemToObject(info,"data",data = cJSON_CreateArray());  //创建数组

        if(NULL != info){
            user_history_t *user_history = (user_history_t *)malloc(total_size);
            memcpy((uint8_t *)&user_history->imped_value,(uint8_t *)&user_read_data[sizeof(user_history_t)*calc_len],sizeof(user_history_t)*send_len);
            
            if(NULL != data){
                for(uint16_t i=0;i<send_len;i++){
                    cJSON_AddItemToArray(data, history_weight[i] = cJSON_CreateObject());/* 给创建的数组增加对象*/
                    if(NULL != history_weight[i]){
                        cJSON_AddNumberToObject(history_weight[i],"weigh_kg",user_history[i].weight_kg);
                        cJSON_AddNumberToObject(history_weight[i],"weigh_lb",user_history[i].weight_lb);
                        cJSON_AddNumberToObject(history_weight[i],"impedence",user_history[i].imped_value);
                        cJSON_AddNumberToObject(history_weight[i],"weighTime",user_history[i].utc_time);
                        cJSON_AddNumberToObject(history_weight[i],"unit",user_history[i].measu_unit);
                        cJSON_AddNumberToObject(history_weight[i],"timezone15m",user_history[i].time_zone);
                    }
                }
            }
            free(user_history);
        }
    }

    if((vesync_get_router_link() == false)){
        vesync_client_connect_wifi((char *)net_info.station_config.wifiSSID, (char *)net_info.station_config.wifiPassword);
    }
    /****************************数据封包结束**********************************/

    time_t seconds;
    seconds = time((time_t *)NULL);
    char traceId_buf[64];
    itoa(seconds, traceId_buf, 10);
    char recv_buff[300];
    int buff_len = sizeof(recv_buff);

    cJSON *report = vesync_json_add_method_head(traceId_buf,req_method,info);
    if(mask == 0){ //上传数据需要携带token信息
        vesync_get_https_token(token);
        cJSON_AddStringToObject(report, "token", token);
    }
    char* out = cJSON_PrintUnformatted(report);
    LOG_I("JSON", "%s", out);

    ret = vesync_https_client_request(req_method, out, recv_buff, &buff_len, 20000);
    if(buff_len > 0 && ret == 0){
        uint8_t if_resend;
        LOG_I(TAG, "Https recv %d byte data : %s", buff_len, recv_buff);
        if_resend = app_json_https_service_parse(mask,recv_buff);
        if(if_resend == 0){
            if(mask == 0){  //上传数据
                switch(gUserUploadStep){
                    case 1: //实时数据
                            LOG_I(TAG, "real time data has all send");
                            gUserUploadStep = 0;
                        break;
                    default:
                            rst_len -= send_len;
                            calc_len += send_len;

                            if(rst_len == 0){
                                gUserUploadStep = 1;    //继续上传用户实时数据
                                calc_len = 0;
                                vesync_flash_erase_key(USER_HISTORY_DATA_NAMESPACE,mask_user_store_key);//擦除匹配用户沉淀数据key
                            }else{
                                gUserUploadStep = 2;   //继续上传当前用户沉淀剩余数据
                                LOG_I(TAG, "resend weight data to server");
                            }
                            ESP_LOGI(TAG, "history data %d , array_len %d ,send_len %d rst_len %d",total_size , array_len,send_len,rst_len);
                            app_handle_net_service_task_notify_bit(UPLOAD_WEIGHT_DATA_REQ,0,0);
                        break;
                }
            }else if(mask == 1){    //刷新token回复;
                gUserUploadStep = 0;
                mask = 0;
                app_handle_net_service_task_notify_bit(UPLOAD_WEIGHT_DATA_REQ,0,0);
            }
        }else if(if_resend == 3){  //token失效
            LOG_E(TAG, "server report token error!!!!");
            mask = 1;
            app_handle_net_service_task_notify_bit(UPLOAD_WEIGHT_DATA_REQ,0,0);
        }else if(if_resend == 4){//数据上传失败
            mask = 0;
            gUserUploadStep = 0;
            if(array_len < USER_STORE_LEN){    //判断沉淀数据是否超出范围
                if(!vesync_flash_write(USER_HISTORY_DATA_NAMESPACE,mask_user_store_key,(user_history_t *)&info_str.user_history_data ,sizeof(user_history_t))){
                    ESP_LOGE(TAG, "real time data store history data error!!"); 
                }
            }else{
                ESP_LOGE(TAG, "history data max 100!!!!!");
            }
        }
    }else{
        mask = 0;
        gUserUploadStep = 0;
        if(array_len < USER_STORE_LEN){    //判断沉淀数据是否超出范围
            if(!vesync_flash_write(USER_HISTORY_DATA_NAMESPACE,mask_user_store_key,(user_history_t *)&info_str.user_history_data ,sizeof(user_history_t))){
                ESP_LOGE(TAG, "real time data store history data error!!"); 
            }
        }else{
            ESP_LOGE(TAG, "history data max 100!!!!!");
        }
    }
    ESP_LOGI(TAG, "gUserUploadStep end %d",gUserUploadStep);
    free(out);
    cJSON_Delete(report);
    cJSON_Delete(root);
}

/**
 * @brief
 * @param pvParameters
 */
static void app_handle_server_task_handler(void *pvParameters){
    BaseType_t notified_ret;
	uint32_t notified_value;
    https_send_frame_t send_frame ={0};

    while(1){
        notified_ret = xTaskNotifyWait(0x00000000, 0xFFFFFFFF, &notified_value, 10000 / portTICK_RATE_MS);
        if(1 == notified_ret){
            if(notified_value & UPLOAD_WEIGHT_DATA_REQ){
                LOG_I(TAG, "UPLOAD_WEIGHT_DATA_REQ");
                app_bt_wifi_suspend_start(BT_WIFI_ENTER_SUSPEND_TIME);		//开启WIFI 2min休眠计时
                vesync_send_match_user_data_https_req();
            }
            if(notified_value & UPLOAD_ALL_USER_DATA_REQ){
                LOG_I(TAG, "UPLOAD_ALL_USER_DATA_REQ");
                app_bt_wifi_suspend_start(BT_WIFI_ENTER_SUSPEND_TIME);		//开启WIFI 2min休眠计时
                vesync_send_all_data_https_req();
            }
            if(notified_value & STORE_WEIGHT_DATA_REQ){
                LOG_I(TAG, "STORE_WEIGHT_DATA_REQ");
                vesync_store_match_user_req();
            }
        }
    }
    vTaskDelete(NULL);
}
/**
 * @brief 创建https数据发送队列
 */
static void app_handle_https_message_queue_create(void)
{
    https_message_send_queue = xQueueCreate(5, sizeof(https_send_frame_t));
    if(https_message_send_queue == 0){
        ESP_LOGE(TAG, "create https message fail!");
    }
}

/**
 * @brief 路由器状态
 * @param status 
 */
void router_status(vesync_router_link_status_t status)
{
    uint8_t router_conn =0;
    uint8_t last_link_status = vesync_get_router_link_status();
    LOG_E(TAG, "router status %d ,last link status:%d \n",status,last_link_status);
    
    switch(status){
        case DEV_ROUTER_LINK_DISCONNTED:
                router_conn = 1;
                resend_cmd_bit |= RESEND_CMD_ROUTER_LINK;
                app_uart_encode_send(MASTER_SET,CMD_WIFI_STATUS,(unsigned char *)&router_conn,sizeof(uint8_t),true);
            break;
        case DEV_ROUTER_LINK_CONNTECD:
                router_conn = 2;
                app_uart_encode_send(MASTER_SET,CMD_WIFI_STATUS,(unsigned char *)&router_conn,sizeof(uint8_t),true);
                resend_cmd_bit |= RESEND_CMD_ROUTER_LINK;
            break;
        default:
            break;
    }
}

/**
 * @brief 配网状态
 * @param status 
 */
void device_status(device_status_e status)
{
    LOG_I(TAG, "device status %d\n",status);
    uint8_t wifi_conn =0;
	switch(status){
        case DEV_CONFIG_NET_FAIL:
            wifi_conn = 1;
            resend_cmd_bit |= RESEND_CMD_WIFI_STATUS_BIT;
            app_uart_encode_send(MASTER_SET,CMD_WIFI_STATUS,(unsigned char *)&wifi_conn,sizeof(uint8_t),true);
            break;
		case DEV_CONFIG_NET_NULL:				    //没有配网记录
            wifi_conn = 0;
            app_uart_encode_send(MASTER_SET,CMD_WIFI_STATUS,(unsigned char *)&wifi_conn,sizeof(uint8_t),true);
            resend_cmd_bit |= RESEND_CMD_WIFI_STATUS_BIT;
			break;
        case DEV_CONFIG_NET_SUCCESS:
		case DEV_CONFIG_NET_RECORDS:				//已有配网记录
            wifi_conn = 2;
            app_uart_encode_send(MASTER_SET,CMD_WIFI_STATUS,(unsigned char *)&wifi_conn,sizeof(uint8_t),true);
            resend_cmd_bit |= RESEND_CMD_WIFI_STATUS_BIT;
			break;
        case DEV_CONFIG_NET_READY:                  //配网中 
            wifi_conn = 1;
            app_scale_suspend_start();
            app_bt_wifi_suspend_stop();	            //连接蓝牙禁止WIFI休眠();	
            
            app_uart_encode_send(MASTER_SET,CMD_WIFI_STATUS,(unsigned char *)&wifi_conn,sizeof(uint8_t),true);
            resend_cmd_bit |= RESEND_CMD_WIFI_STATUS_BIT;
			break;
        default:
            break;				
	}
}
/**
 * @brief app端创建通知任务
 */
void app_hadle_server_create(void)
{
    vesync_flash_config(true, USER_HISTORY_DATA_NAMESPACE);//初始化用户沉淀数据flash区域
    xTaskCreate(app_handle_server_task_handler, "app_handle_server_task_handler", 8*1024, NULL, 2, &s_network_service_taskhd);

    app_handle_net_service_task_notify_bit(UPLOAD_ALL_USER_DATA_REQ,NULL,0);
}
