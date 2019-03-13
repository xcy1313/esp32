/**
 * @file app_body_fat_calc.c
 * @brief 体脂参数本地计算
 * @author Jiven 
 * @date 2018-12-18
 */
#include "app_body_fat_calc.h"
#include "app_handle_server.h"
#include "vesync_net_service.h"

#include "vesync_uart.h"
#include "vesync_flash.h"
#include "esp_log.h"
#include <time.h>
#include "vesync_crc8.h"
#include "vesync_wifi.h"

static const char *TAG = "body_FAT";

char    mask_user_store_key[12];
uint32_t match_account_id;

#define SPORT_MODE      0   //运动员模式;
#define NORMAL_MODE     1   //普通人模式；

#define MAX_WEIGHT      300     //+-三公斤
#define MAX_IMPED       30      //+-30ohm

#define MIN_FAT         40
#define MAX_FAT         600

#define ff(x)	        (((int)(x+0.5))>((int)x)?((int)x+1):((int)x))
#define kg_to_lb(x)     ff((x)*2.20462262)
#define lb_to_kg(x)     ff((x)*0.45359237)
#define lb_to_g(x)      ff(((x)*453.59237)
#define st_to_lb(x)     ff(((x)*14)
#define st_to_kg(x)     ff((x)*6.3502932)
//BMI计算
#define BMI(W,H)        (((float)(W)*1000)/((H)*(H)))

//FAT计算
#define FAT_HIGH_16_MAN(F,A,B)          ((120090/(F)+0.676*(A)+376-50679100/((B)*(F))))
#define FAT_LOW_16_MAN(F,A,B)           ((120090/(F)+566-(11.21*(A))-50679100/((B)*(F))))
#define FAT_HIGH_16_WOMAN(F,A,B)        ((173870/(F)+0.467*(A)+430-55642100/((B)*(F))))
#define FAT_LOW_16_WOMAN(F,A,B)         ((173870.0/(F)+442.0-55642100.0/((B)*(F))))
#define FAT_HIGH_16_SPORT_MAN(F,W,H)    (314700/(F)+240.4-1222000000/((W)*(F))-(H)*(H)*6.5038/(F))
#define FAT_HIGH_16_SPORT_WOMAN(F,W,H)  (424500/(F)+535.0-1593166000/((W)*(F))-(H)*(H)*11.38/(F))

//MUSCLE计算
#define MUSCLE(F)                       ((1000-(F))*0.6875)   

//WATER计算
#define WATER_HIGH_16_MAN(F,W,H)        ((696819000/(F)+299430)/(W)+37053*(H)*(H)/10000/(F)+296-107700/(F))
#define WATER_HIGH_16_WOMAN(F,W,H)      ((556707000/(F)+444930)/(W)+37524*(H)*(H)/10000/(F)+243-102760/(F))
#define WATER_HIGH_16_SPORT_MAN(F,W,H)  ((702740000/(F)+280000)/(W)+37074*(H)*(H)/10000/(F)+298-108220/(F))
#define WATER_HIGH_16_SPORT_WOMAN(F,W,H) ((527426000/(F)+502010)/(W)+36977*(H)*(H)/10000/(F)+233-95828/(F))
#define WATER_LOW_16(F)                 ((1000-(F))*0.6875) 

//BONE计算
#define BONE_MAN(F)                     (1000 -(F))*0.052
#define BONE_WOMAN(F)                   (1000 -(F))*0.061

//BMR计算
#define BMR_MAN(F,A,W,H)                ((0.1764-26.475/(F))*(W)+(137877/(F)-9.3522)*(H)*(H)/10000+4095/(A)-45378/(F)-133)
#define BMR_WOMAN(F,A,W,H)              ((0.1739-36.424/(F))*(W)+(132178/(F)-19.79)*(H)*(H)/10000+3758/(A)-32682/(F)-49)
#define BMR_SPORT_MAM(F,A,W,H)          ((0.22576-27.047/(F))*(W)+(118380/(F)-8.561)*(H)*(H)/10000+3086/(A)-64132/(F)-146)
#define BMR_SPORT_WOMAM(F,A,W,H)        ((0.15016-25.81/(F))*(W)+(147997/(F)-15.777)*(H)*(H)/10000+3112/(A)-42482/(F)-21)


/**
 * @brief 判断计算参数是否为空，防止分母为0出现panic
 * @param config 
 * @param p_weitht 
 * @return true 
 * @return false 
 */
bool body_fat_para_if_null(response_weight_data_t *p_weitht)
{
    if((p_weitht->weight == 0) || (p_weitht->lb == 0)){
        return false;
    }
    return true;
}
/**
 * @brief 
 * @param bt_status 蓝牙连接状态
 * @param mask      需要处理的体脂参数个数
 * @param res       用户模型信息
 * @param p_weitht  当前称重数据
 * @return true     
 * @return false 
 */
static bool body_fat_calc(user_fat_data_t *fat_data,uint16_t mask,user_config_data_t *config,response_weight_data_t *p_weitht)
{    
    double calc_fat =0;
    double calc_bmi =0;
    double calc_muscle =0;
    double calc_water =0;
    double calc_bone =0;
    double calc_bmr =0;

    ESP_LOGI(TAG, "==============================");
    ESP_LOGI(TAG, "account : 0x%04x", config->account);
    ESP_LOGI(TAG, "gender : 0x%x", config->gender);
    ESP_LOGI(TAG, "height_unit : %d", config->height_unit);
    ESP_LOGI(TAG, "height : %d", config->height);
    ESP_LOGI(TAG, "age : %d", config->age);
    ESP_LOGI(TAG, "measu_unit : %d", config->measu_unit);
    ESP_LOGI(TAG, "weight_kg : %d", config->weight_kg);
    ESP_LOGI(TAG, "weight_lb : %d", config->weight_lb);
    ESP_LOGI(TAG, "imped_value : %d", config->imped_value);
    ESP_LOGI(TAG, "user_store_key : %s", config->user_store_key);
    ESP_LOGI(TAG, "mersure weight : %d", p_weitht->weight);
    ESP_LOGI(TAG, "if normal mode: %d", config->user_mode);
    ESP_LOGI(TAG, "==============================");

    if(false == body_fat_para_if_null(p_weitht)) return false;

    calc_bmi = (((double)(p_weitht->weight))*1000)/(((double)config->height) * ((double)config->height));
    ESP_LOGI(TAG, "calc_bmi: %f",calc_bmi);
    ESP_LOGI(TAG, "current imped: %d",p_weitht->imped_value);
    if(0 != p_weitht->imped_value){
        switch(config->gender){
            case 0: //男人
                if(mask & 0x0001){    //仅仅处理部分
                    if(config->user_mode == NORMAL_MODE){   //普通人模式
                        if(config->age >= 16){
                            calc_fat = 120090.0/((double)(p_weitht->imped_value))+0.676*((double)(config->age))+376.0-50679100.0/(((double)calc_bmi)*((double)(p_weitht->imped_value)));
                        }else{
                            calc_fat = 120090/((double)(p_weitht->imped_value))+566-(11.21*((double)(config->age)))-50679100/(((double)calc_bmi)*((double)(p_weitht->imped_value)));
                        }
                    }else{
                        if(config->age >= 16){
                            calc_fat = FAT_HIGH_16_SPORT_MAN((double)p_weitht->imped_value,(double)p_weitht->weight,(double)config->height);
                        }else{
                            calc_fat = FAT_LOW_16_MAN(((double)p_weitht->imped_value),((double)config->age),((double)calc_bmi));    //同普通人模式
                        }
                    }
                    ESP_LOGI(TAG, "man calc_fat: %f",calc_fat);

                    if(calc_fat < MIN_FAT) 
                        calc_fat = MIN_FAT;
                    else if(calc_fat > MAX_FAT) 
                        calc_fat = MAX_FAT;
                }
                if(mask & 0x0002){
                    calc_muscle = MUSCLE(((double)calc_fat));
                    ESP_LOGI(TAG, "calc_muscle: %f",calc_muscle);
                }
                if(mask & 0x0004){
                    if(config->user_mode == NORMAL_MODE){   //普通人模式
                        if(config->age >= 16){
                            calc_water = WATER_HIGH_16_MAN(((double)p_weitht->imped_value),((double)p_weitht->weight),((double)config->height));
                        }else{
                            calc_water = WATER_LOW_16((double)calc_fat);
                        }
                    }else{
                        if(config->age >= 16){
                            calc_water = WATER_HIGH_16_SPORT_MAN(((double)p_weitht->imped_value),((double)p_weitht->weight),((double)config->height));
                        }else{
                            calc_water = WATER_LOW_16(((double)calc_fat));
                        }
                    }
                    ESP_LOGI(TAG, "calc_water: %f",calc_water);
                }
                if(mask & 0x0008){
                    calc_bone = BONE_MAN(((double)calc_fat));
                    ESP_LOGI(TAG, "calc_bone: %f",calc_bone);
                }
                if(mask & 0x0010){
                    if(config->user_mode == NORMAL_MODE){   //普通人模式
                        calc_bmr = (((0.1764-26.475/((double)p_weitht->imped_value))*((double)p_weitht->weight)+(137877/((double)p_weitht->imped_value)-9.3522)*((double)config->height)*((double)config->height)/10000+4095/((double)config->age)-45378/((double)p_weitht->imped_value)-133));
                    }else{
                        calc_bmr = BMR_SPORT_MAM(((double)p_weitht->imped_value),((double)config->age),((double)p_weitht->weight),((double)config->height));
                    }
                    ESP_LOGI(TAG, "calc_bmr: %f",calc_bmr);
                }
                break;
            case 1: //女人
                if(mask & 0x0001){
                    if(config->user_mode == NORMAL_MODE){   //普通人模式
                        if(config->age >= 16){
                            calc_fat = FAT_HIGH_16_WOMAN(((double)(p_weitht->imped_value)),((double)config->age),((double)calc_bmi));
                        }else{
                            calc_fat = FAT_LOW_16_WOMAN(((double)(p_weitht->imped_value)),((double)config->age),((double)calc_bmi));
                        }
                    }else{
                        if(config->age >= 16){
                            calc_fat = FAT_HIGH_16_SPORT_WOMAN(((double)p_weitht->imped_value),((double)p_weitht->weight),((double)config->height));
                        }else{
                            calc_fat = FAT_LOW_16_WOMAN(((double)p_weitht->imped_value),((double)config->age),((double)calc_bmi));    //同普通人模式
                        }
                    }
                    ESP_LOGI(TAG, "calc_fat: %f",calc_fat);
                    if(calc_fat < MIN_FAT) 
                        calc_fat = MIN_FAT;
                    else if(calc_fat > MAX_FAT) 
                        calc_fat = MAX_FAT;
                }
                if(mask & 0x0002){
                    calc_muscle = MUSCLE(((double)calc_fat));
                    ESP_LOGI(TAG, "calc_muscle: %f",calc_muscle);
                }
                if(mask & 0x0004){
                    if(config->user_mode == NORMAL_MODE){   //普通人模式
                        if(config->age >= 16){
                            calc_water = WATER_HIGH_16_WOMAN(((double)p_weitht->imped_value),((double)p_weitht->weight),((double)config->height));
                        }else{
                            calc_water = WATER_LOW_16((double)calc_fat);
                        }
                    }else{
                        if(config->age >= 16){
                            calc_water = WATER_HIGH_16_SPORT_WOMAN((double)p_weitht->imped_value,(double)p_weitht->weight,(double)config->height);
                        }else{
                            calc_water = WATER_LOW_16((double)calc_fat);
                        }
                    }
                    ESP_LOGI(TAG, "calc_water: %f",calc_water);
                }
                if(mask & 0x0008){
                    calc_bone = BONE_WOMAN((double)calc_fat);
                    ESP_LOGI(TAG, "calc_bone: %f",calc_bone);
                }
                if(mask & 0x0010){
                    if(config->user_mode == NORMAL_MODE){
                        calc_bmr = (((0.1739-36.424/((double)p_weitht->imped_value))*((double)p_weitht->weight)+(132178/((double)p_weitht->imped_value)-19.79)*((double)config->height)*((double)config->height)/10000+3758/((double)config->age)-32682/((double)p_weitht->imped_value)-49));
                    }else{
                        calc_bmr = BMR_SPORT_WOMAM((double)p_weitht->imped_value,(double)config->age,(double)p_weitht->weight,(double)config->height); 
                    }
                    ESP_LOGI(TAG, "calc_bmr: %f",calc_bmr);
                }
                break;
        }
    }
    
    fat_data->bmi = (uint16_t)(ff(calc_bmi));
    fat_data->fat = (uint16_t)(ff(calc_fat));
    fat_data->bmr = (uint16_t)(ff(calc_bmr));
    fat_data->water = (uint16_t)(ff(calc_water));
    fat_data->muscle = (uint16_t)(ff(calc_muscle));
    fat_data->bone = (uint16_t)(ff(calc_bone));

    ESP_LOGI(TAG, "===============================");
    ESP_LOGI(TAG, "fat_data->bmi %d",fat_data->bmi);
    ESP_LOGI(TAG, "fat_data->fat %d",fat_data->fat);
    ESP_LOGI(TAG, "fat_data->bmr %d",fat_data->bmr);
    ESP_LOGI(TAG, "fat_data->water %d",fat_data->water);
    ESP_LOGI(TAG, "fat_data->muscle %d",fat_data->muscle);
    ESP_LOGI(TAG, "fat_data->bone %d",fat_data->bone);
    ESP_LOGI(TAG, "===============================");

    return true;
}

/**
 * @brief 找出最接近x的值，并返回所在的数组位置;
 * @param a 
 * @param n 
 * @param x 
 * @return uint8_t 所在的位置;
 */
uint8_t find_near_data(user_config_data_t *a,uint8_t n,uint16_t x)
{
    uint8_t i;
    uint16_t min =abs(a->weight_kg-x);
    uint8_t r=0;
 
    for(i=0;i < n;++i){
        if(abs(a[i].weight_kg-x) < min){
            min=abs(a[i].weight_kg-x);
            r=i;
        }
    }
    return r;
}
/**
 * @brief 将采样的体重值保存 并计算体脂参数 单位不一需要统一内部换算成kg作比较
 * @param res 
 * @param p_weitht 
 * @return true 
 * @return false 
 */
bool body_fat_person(bool bt_status,hw_info *res ,response_weight_data_t *p_weitht)
{
    bool ret = false;
    static uint8_t o_if_stabil =0;
    static uint8_t n_if_stabil =0;

    o_if_stabil = n_if_stabil;
    n_if_stabil = p_weitht->if_stabil;

    if((o_if_stabil == 0) && (n_if_stabil == 1)){    //判断是否稳定体重;
        if(bt_status == false){
            user_config_data_t user_list[MAX_CONUT] ={0};        
            uint16_t len =0;

            if(vesync_flash_read(USER_MODEL_NAMESPACE,USER_MODEL_KEY,(char *)user_list,&len) != 0){
                ESP_LOGE(TAG, "user module NULL");
            }else{
#if 1
                p_weitht->imped_value = 0;
                p_weitht->weight = 55;  //6500  55
                uint16_t new_imped = p_weitht->imped_value; //调试屏蔽注释 88
                uint16_t new_kg = p_weitht->weight;//调试屏蔽注释 85
                uint8_t user_cnt =0;
                uint8_t i=0;
                user_config_data_t backup_user_list[MAX_CONUT] ={0};    

                for(;i<(len/sizeof(user_config_data_t));i++){   //找出符合+-3kg符合的所有用户
                    if((abs(user_list[i].weight_kg - new_kg) <= MAX_WEIGHT)){              //前后两次体重小于3kg
                        memcpy((user_config_data_t *)&backup_user_list[user_cnt],(user_config_data_t *)&user_list[i],sizeof(user_config_data_t));   //将满足条件范围的数据重新拷贝;
                        user_cnt++;
                    }
                    ESP_LOGI(TAG, "all user config account:[0x%04x],kg:[%d],imped:[%d]",user_list[i].account,user_list[i].weight_kg,user_list[i].imped_value);
                }
                ESP_LOGI(TAG, "====================================================");
                for(uint8_t j=0;j<user_cnt;j++){
                    ESP_LOGI(TAG, "near match user config account:[0x%04x],kg:[%d],imped:[%d],number:[%d]",backup_user_list[j].account,backup_user_list[j].weight_kg,backup_user_list[j].imped_value,j);
                }
                if(user_cnt !=0){
                    uint8_t list_number =0;
                    user_history_t history = {0};

                    list_number = find_near_data(&backup_user_list[0],user_cnt,new_kg);//找出符合用户最接近的用户
                    ESP_LOGI(TAG, "match user cnt is =%d,nearst user number =%d account[0x%08x]" ,user_cnt,list_number,backup_user_list[list_number].account);   //当前满足匹配条件的用户总个数

                    history.imped_value = new_imped;
                    history.utc_time = time((time_t *)NULL);
                    history.time_zone = 8;
                    history.measu_unit = p_weitht->measu_unit;
                    history.weight_kg = p_weitht->weight;
                    history.weight_lb = p_weitht->lb;

                    user_fat_data_t  resp_fat_data ={0};
                    backup_user_list[list_number].user_mode =1;  //默认配置为普通用户模式;
                    if(body_fat_calc(&resp_fat_data,ALL_CALC,&backup_user_list[list_number],p_weitht)){
                        memcpy((user_fat_data_t *)&res->user_fat_data,(user_fat_data_t *)&resp_fat_data,sizeof(user_fat_data_t));
                        resend_cmd_bit |= RESEND_CMD_BODY_FAT_BIT;
                        app_uart_encode_send(MASTER_SET,CMD_BODY_FAT,(unsigned char *)&resp_fat_data,sizeof(user_fat_data_t),true); 
                        ret = true;
                        ESP_LOGI(TAG, "flash store user success");         //体脂参数计算正确
                    }

                    ESP_LOGI(TAG, "[history:imped[0x%04x]]" ,history.imped_value); 
                    ESP_LOGI(TAG, "[history:utc_time[0x%04x]]" ,history.utc_time);
                    ESP_LOGI(TAG, "[history:utc_area[0x%02x]]" ,history.time_zone);
                    ESP_LOGI(TAG, "[history:measu_unit:[0x%02x]]" ,history.measu_unit);
                    ESP_LOGI(TAG, "[history:weight_kg:[0x%04x]]" ,history.weight_kg);
                    ESP_LOGI(TAG, "[history:weight_lb:[0x%04x]]" ,history.weight_lb);

                    match_account_id = backup_user_list[list_number].account;
                    strcpy(mask_user_store_key,backup_user_list[list_number].user_store_key);

                    // static uint8_t user_read_data_buff[4096] ={0};
                    // static uint16_t total_size =0;
                    // static uint8_t  array_len = 0;

                    

                    if((vesync_get_device_status() >= DEV_CONFIG_NET_RECORDS)){ //称重数据上报服务器 
                        if(vesync_get_router_link() == false){
                            vesync_client_connect_wifi((char *)net_info.station_config.wifiSSID, (char *)net_info.station_config.wifiPassword);
                        }
                        memcpy((user_history_t *)&res->user_history_data ,(user_history_t *)&history,sizeof(user_history_t));
                        app_handle_net_service_task_notify_bit(UPLOAD_WEIGHT_DATA_REQ,0,0);
                    }
                }else{
                    ESP_LOGE(TAG, "user mode para not match not same user!");        //蓝牙已经连接，参数已传给蓝牙，本地不做处理
                }

#else
                p_weitht->weight = 6845;
                p_weitht->imped_value = 585;
                user_fat_data_t  resp_fat_data ={0};
                user_history_t history = {0};
                user_config_data_t config;
                
                config.imped_value = 585;
                config.gender = 1;
                config.height_unit = 1;
                config.height = 178;
                config.age = 14;
                config.measu_unit = 0;
                config.user_mode = 1;
                if(body_fat_calc(&resp_fat_data,ALL_CALC,&config,p_weitht)){
                    ESP_LOGI(TAG, "fat:%d,muscle:%d,water:%d,bone=%d,bmr=%d,bmi=%d\n" ,resp_fat_data.fat,resp_fat_data.muscle,resp_fat_data.water,
                                resp_fat_data.bone,resp_fat_data.bmr,resp_fat_data.bmi);
                }
#endif                    
            }
        }
    }   
    return ret;
}
