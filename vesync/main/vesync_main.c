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


#include "vesync_ota.h"
#include "vesync_uart.h"
#include "vesync_bt.h"
#include "vesync_wifi.h"
#include "vesync_blufi.h"
#include "vesync_button.h"
#include "vesync_unixtime.h"
#include "vesync_flash.h"
#include "vesync_crc8.h"

#include "esp_log.h"

static const char *TAG = "main";

uni_frame_t bt_frame;
static TaskHandle_t user_task;
typedef enum{
	REC_HARD_VERSION,
	REC_PROJECT_ITEM,
}gUserAction;
gUserAction gUserConfig;

#define ENABLE_DEBUG    1
/**
 * @brief 
 * @param p_event_data 
 */
void ui_event_handler(void *p_event_data){

    ESP_LOGI(TAG, "key [%d]\r\n" ,*(uint8_t *)p_event_data);
    switch(*(uint8_t *)p_event_data){
        case Short_key:

				vesync_flash_write("userdata","store","112233445566778899",20);	
			return;
        case Double_key:
							
			return;
        case Reapet_key:
						
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
 * @brief app下发控制指令 称体指令作透传，用户数据固件本地处理，不做透传;
 * @param bt_buf 
 * @param length 
 */
void bt_event_handler(const void*bt_buf, int length){
    esp_log_buffer_hex(TAG, (char *)bt_buf, length);    
//    WriteCoreQueue((char *)bt_buf, length);

    for(int i=0;i<length;i++){
        if(Comm_frame_parse(*(unsigned char *)&bt_buf[i],1,&bt_frame) == 1){
            uint8_t res_data[20]={0};
            ESP_LOGI(TAG, "frame_data_len %d\r\n", bt_frame.frame_data_len);

            for(uint8_t i=0;i< bt_frame.frame_data_len;i++){
                printf("%02x ",bt_frame.frame_data[i]);
            }
            printf("\r\n ctl =0x%02x[%d] ,len =%d\r\n ",bt_frame.frame_ctrl,bt_frame.frame_ctrl,bt_frame.frame_data_len);

            for(uint8_t j=0;j<sizeof(command_type)/sizeof(command_type[0]);j++){
                if(bt_frame.frame_ctrl == command_type[j].command){    // 蓝牙协议控制码即为指令;
                    uint8_t sendlen =0;
                    uint8_t *opt = &bt_frame.frame_data[0];
                    switch(command_type[j].command){
                        case CMD_HW_VN:         //app主动获取版本号
                                ESP_LOGI(TAG, "app get CMD_HW_VN\r\n");
                                command_type[j].transfer_callback = uart_encode_send;
                                res_data[0] = MASTER_INQUIRY;
                                res_data[1] = command_type[j].command;
                                sendlen = 0;
                            break;
                        case CMD_ID:           //app主动获取产品编码
                                ESP_LOGI(TAG, "app get CMD_ID\r\n");
                                command_type[j].transfer_callback = uart_encode_send;
                                res_data[0] = MASTER_INQUIRY;
                                res_data[1] = command_type[j].command;
                                sendlen = 0;
                            break;
                        case CMD_MEASURE_UNIT: //app切换计量单位
                                ESP_LOGI(TAG, "app set CMD_MEASURE_UNIT\r\n");
                                command_type[j].transfer_callback = uart_encode_send;
                                res_data[0] = MASTER_SET;
                                res_data[1] = command_type[j].command;
                                *(uint8_t *)&res_data[2] = info_str.user_config_data.measu_unit;
                                sendlen = sizeof(uint8_t);
                            break;
                        case CMD_BACKLIGHT_TIME://app切换背光时间
                                ESP_LOGI(TAG, "app set CMD_BACKLIGHT_TIME\r\n");
                            break;
                        case CMD_POWER_BATTERY:
                                ESP_LOGI(TAG, "app get CMD_POWER_BATTERY\r\n");
                                command_type[j].transfer_callback = uart_encode_send;
                                res_data[0] = MASTER_INQUIRY;
                                res_data[1] = command_type[j].command;
                                sendlen = 0;    
                            break;
                        case CMD_CREATE_USER:{
                                uint8_t crc8;
                                ESP_LOGI(TAG, "app CMD_CREATE_USER\r\n");
                                crc8 = vesync_crc8(0,opt,bt_frame.frame_data_len); //过滤crc字段
                                ESP_LOGI(TAG, "crc8= %d config_crc8:%d\r\n", crc8,info_str.user_config_data.crc8);
                                
                                res_data[0] = command_type[j].command;
                                res_data[1] = 1;
                                res_data[2] = 8;
                                if(crc8 != info_str.user_config_data.crc8){ //crc8不同表示用户数据有改变
                                    info_str.user_config_data.crc8 = crc8;
                                    info_str.user_config_data.length = bt_frame.frame_data_len;
                                    memcpy((uint8_t *)&info_str.user_config_data.account,opt,bt_frame.frame_data_len);
                                    //if(vesync_flash_write("userconfig","config",(uint8_t *)&info_str.user_config_data.account,sizeof(user_config_data_t)-1)){  //保存用户配置信息
                                    if(vesync_flash_write("userdata","store","112233445566778899",20)){
                                        ESP_LOGI(TAG, "store user config ok! crc8 =%d len =%d\r\n",info_str.user_config_data.crc8 ,info_str.user_config_data.length);
                                    }else{
                                        res_data[1] = 0;
                                    }
                                }
                                *(uint32_t *)&res_data[3] = info_str.user_config_data.account;
                                res_data[7] = info_str.user_config_data.ueser_id;
                                sendlen = 7;    //不包含command id
                                ESP_LOGI(TAG, "CMD_CREATE_USER account:0x%04x ,ueser_id:%02x ,gender:%d,height:%d,age:%d ,weight:%d,measu_unit:%d,user_mode:%d\r\n",
                                                        info_str.user_config_data.account,info_str.user_config_data.ueser_id,info_str.user_config_data.gender,info_str.user_config_data.height,
                                                        info_str.user_config_data.age,info_str.user_config_data.weight,info_str.user_config_data.measu_unit,info_str.user_config_data.user_mode);
                            }
                            break;
                        case CMD_DELETE_USER:
                             ESP_LOGI(TAG, "app CMD_CREATE_USER \r\n");
                            break;
                        case CMD_DELETE_HIS_ITEM:
                              ESP_LOGI(TAG, "app CMD_DELETE_HIS_ITEM \r\n");
                            break;
                        case CMD_SYNC_UTC:
                              ESP_LOGI(TAG, "app CMD_SYNC_UTC \r\n");
                            break;
                        case CMD_MODIFY_USER:   //app修改账户信息
                              ESP_LOGI(TAG, "app CMD_MODIFY_USER \r\n");
                            break;
                        case CMD_HISTORY_TOTALLEN://app获取当前历史数据总量
                              ESP_LOGI(TAG, "app CMD_HISTORY_TOTALLEN \r\n");
                            break;
                        case CMD_USER_AMOUT:    //app获取当前用户数量
                              ESP_LOGI(TAG, "app CMD_USER_AMOUT \r\n");
                            break;
                        default:
                              ESP_LOGI(TAG, "error cmd! \r\n");
                            break; 
                    }
                    if(command_type[j].transfer_callback != NULL){
                        if(command_type[j].transfer_callback == uart_encode_send){
                            ESP_LOGI(TAG, "send len =%d \r\n" ,res_data[1]);
                            command_type[j].transfer_callback(res_data[0],res_data[1],&res_data ,sendlen);  //串口下发
                        }
                        else if(command_type[j].transfer_callback == vesync_bt_notify){
                            ESP_LOGI(TAG, "send len =%d \r\n" ,res_data[1]);
                            command_type[j].transfer_callback(0,0,res_data ,sendlen);  //应答app
                        }
                    }
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
void  uart_event_handler(const void*uart_buf, int length){


}

RET_MSG_ID_E DEVICE_PUBLIC_COMMUITCT(hw_info *res ,uint8_t cmd)
{
    RET_MSG_ID_E gRet=RET_MSG_ID_NONE;
    static uint8_t step=0;
    static int gWaitRespone =0;

    uint16_t len =0;
#if 0
	if(step==0){
        if(uart_encode_send())
		//if(packetdataSend(cmd ,encodecommand) == RET_MSG_ID_OK)
        {
			step++;
		}else{
			ESP_LOGI(TAG, "Public command Error! -->[0x%0x4x]:",cmd);
		}
	}else if(step==1){
		//step=0;
	}
#endif
    return gRet;
}
/**
 * @brief 
 * @param pvParameters 
 */
static void user_task_handler_loop(void *pvParameters)
{
    RET_MSG_ID_E gRet=RET_MSG_ID_NONE;
    while(1) { 
        switch(gUserConfig){
			case REC_HARD_VERSION:
                    if(DEVICE_PUBLIC_COMMUITCT(&info_str ,CMD_HW_VN) == RET_MSG_ID_OK)
						gUserConfig = REC_PROJECT_ITEM;
                break;
            case REC_PROJECT_ITEM:
                    if(DEVICE_PUBLIC_COMMUITCT(&info_str ,CMD_ID) == RET_MSG_ID_OK)
						gUserConfig = REC_HARD_VERSION;
                break;    
            default:
                break;
        }
        vTaskDelay(500 / portTICK_PERIOD_MS);	//正常使用500ms；
    }
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
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    esp_read_mac(&info_str.macaddr[0], ESP_MAC_BT);
    ESP_LOGI(TAG, "BT mac:%02x%02x%02x%02x%02x%02x", info_str.macaddr[0][0],info_str.macaddr[0][1],info_str.macaddr[0][2],info_str.macaddr[0][3],info_str.macaddr[0][4],info_str.macaddr[0][5]);

    esp_read_mac(&info_str.macaddr[1], ESP_MAC_WIFI_STA);
    ESP_LOGI(TAG, "STATION mac:%02x%02x%02x%02x%02x%02x", info_str.macaddr[1][0],info_str.macaddr[1][1],info_str.macaddr[1][2],info_str.macaddr[1][3],info_str.macaddr[1][4],info_str.macaddr[1][5]);
    
    vesync_ota_init(ota_event_handler);
    vesync_wifi_init();
    vesync_bt_init(bt_event_handler);
    vesync_blufi_init();
    vesync_button_init(ui_event_handler);
    vesync_uart_int(uart_event_handler);
    printf("silicon revision %d \n", chip_info.revision);

    vesync_flash_init("userdata");
    xTaskCreate(&user_task_handler_loop, "user_task_handler_loop", 8192, NULL, 10, &user_task);
#if 0
    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    for (int i = 10; i >= 0; i--) {
        printf("Restarting in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
#endif    
}
