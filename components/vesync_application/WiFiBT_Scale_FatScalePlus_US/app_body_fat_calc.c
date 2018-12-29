/**
 * @file app_body_fat_calc.c
 * @brief 体脂参数本地计算
 * @author Jiven 
 * @date 2018-12-18
 */
#include "app_body_fat_calc.h"
#include "vesync_uart.h"
#include "vesync_flash.h"
#include "esp_log.h"
#include <time.h>
#include "vesync_crc8.h"

static const char *TAG = "body_FAT";

#define SPORT_MODE      0   //运动员模式;
#define NORMAL_MODE     1   //普通人模式；

#define MAX_WEIGHT      3       //+-三公斤
#define MAX_IMPED       30      //+-30ohm

#define ff(x)	        (((int)(x+0.5))>((int)x)?((int)x+1):((int)x))
#define kg_to_lb(x)     ff((x)*2.20462262)
#define lb_to_kg(x)     ff((x)*0.45359237)
#define lb_to_g(x)      ff(((x)*453.59237)
#define st_to_lb(x)     ff(((x)*14)
#define st_to_kg(x)     ff((x)*6.3502932)
//BMI计算
#define BMI(W,H)                        ((W)*10000/((H)*(H)))

//FAT计算
#define FAT_HIGH_16_MAN(F,A,B)          ff((120090/(F)+0.676*(A)+376-50679100/((B)*(F))))
#define FAT_LOW_16_MAN(F,A,B)           ff((120090/(F)+566-11.21*(A)-50679100/((B)*(F))))
#define FAT_HIGH_16_WOMAN(F,A,B)        ff((173870/(F)+0.467*(A)+430-55642100/((B)*(F))))
#define FAT_LOW_16_WOMAN(F,A,B)         ff((173870/(F)+442-55642100/((B)*(F))))
#define FAT_HIGH_16_SPORT_MAN(F,W,H)    ff(314700/(F)+240.4-1222000000/((W)*(F))-(H)*(H)*6.5038/(F))
#define FAT_HIGH_16_SPORT_WOMAN(F,W,H)  ff(424500/(F)+535.0-1593166000/((W)*(F))-(H)*(H)*11.38/(F))

//MUSCLE计算
#define MUSCLE(F)                       ff((1000-(F))*0.5)   

//WATER计算
#define WATER_HIGH_16_MAN(F,W,H)        ff((696819000/(F)+299430)/(W)+37053*(H)*(H)/10000/(F)+296-107700/(F))
#define WATER_HIGH_16_WOMAN(F,W,H)      ff((556707000/(F)+444930)/(W)+37524*(H)*(H)/10000/(F)+243-102760/(F))
#define WATER_HIGH_16_SPORT_MAN(F,W,H)  ff((702740000/(F)+280000)/(W)+37074*(H)*(H)/10000/(F)+298-108220/(F))
#define WATER_HIGH_16_SPORT_WOMAN(F,W,H) ff((527426000/(F)+502010)/(W)+36977*(H)*(H)/10000/(F)+233-95828/(F))
#define WATER_LOW_16(F)                 ((1000-(F)*0.6875))

//BONE计算
#define BONE_MAN(F)                     (1000 -(F))*0.052
#define BONE_WOMAN(F)                   (1000 -(F))*0.061

//BMR计算
#define BMR_MAN(F,A,W,H)                ff((0.1764-26.475/(F))*(W)+(137877/(F)-9.3522)*(H)*(H)/10000+4095/(A)-45378/(F)-133)
#define BMR_WOMAN(F,A,W,H)              ff((0.1739-36.424/(F))*(W)+(132178/(F)-19.79)*(H)*(H)/10000+3758/(A)-32682/(F)-49)
#define BMR_SPORT_MAM(F,A,W,H)          ff((0.22576-27.047/(F))*(W)+(118380/(F)-8.561)*(H)*(H)/10000+3086/(A)-64132/(F)-146)
#define BMR_SPORT_WOMAM(F,A,W,H)        ff((0.15016-25.81/(F))*(W)+(147997/(F)-15.777)*(H)*(H)/10000+3112/(A)-42482/(F)-21)

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
bool body_fat_calc(bool bt_status,uint16_t mask ,hw_info *res,user_config_data_t *config,response_weight_data_t *p_weitht)
{    
    ESP_LOGI(TAG, "body_fat_calc bt:%d,weight[%d] gender[%d] height[%d] account[0x%04x]",bt_status,p_weitht->weight,config->gender,config->height,config->account);
    user_fat_data_t  resp_fat_data ={0};
    uint8_t crc8;
    uint8_t o_crc8;
    if(false == body_fat_para_if_null(p_weitht)) return false;
    
    resp_fat_data.bmi = BMI(p_weitht->weight,config->height);

    switch(config->gender){
        case 0: //男人
            if(mask & 0x0001){    //仅仅处理部分
                if(config->user_mode == NORMAL_MODE){   //普通人模式
                    if(config->age >= 16){
                        resp_fat_data.fat = FAT_HIGH_16_MAN(p_weitht->imped_value,config->age,resp_fat_data.bmi);
                    }else{
                        resp_fat_data.fat = FAT_LOW_16_MAN(p_weitht->imped_value,config->age,resp_fat_data.bmi);
                    }
                }else{
                    if(config->age >= 16){
                        resp_fat_data.fat = FAT_HIGH_16_SPORT_MAN(p_weitht->imped_value,p_weitht->weight,config->height);
                    }else{
                        resp_fat_data.fat = FAT_LOW_16_MAN(p_weitht->imped_value,config->age,resp_fat_data.bmi);    //同普通人模式
                    }
                }
            }
            if(mask & 0x0002){
                resp_fat_data.muscle = MUSCLE(resp_fat_data.fat);
            }
            if(mask & 0x0004){
                if(config->user_mode == NORMAL_MODE){   //普通人模式
                    if(config->age >= 16){
                        resp_fat_data.water = WATER_HIGH_16_MAN(p_weitht->imped_value,p_weitht->weight,config->height);
                    }else{
                        resp_fat_data.water = WATER_LOW_16(resp_fat_data.fat);
                    }
                }else{
                    if(config->age >= 16){
                        resp_fat_data.water = WATER_HIGH_16_SPORT_MAN(p_weitht->imped_value,p_weitht->weight,config->height);
                    }else{
                        resp_fat_data.water = WATER_LOW_16(resp_fat_data.fat);
                    }
                }
            }
            if(mask & 0x0008){
                    resp_fat_data.bone = BONE_MAN(resp_fat_data.fat);
            }
            if(mask & 0x0010){
                if(config->user_mode == NORMAL_MODE){   //普通人模式
                    resp_fat_data.bmr = BMR_MAN(p_weitht->imped_value,config->age,p_weitht->weight,config->height);
                }else{
                    resp_fat_data.bmr = BMR_SPORT_MAM(p_weitht->imped_value,config->age,p_weitht->weight,config->height);
                }
            }
            break;
        case 1: //女人
            if(mask & 0x0001){
                if(config->user_mode == NORMAL_MODE){   //普通人模式
                    if(config->age >= 16){
                        resp_fat_data.fat = FAT_HIGH_16_WOMAN(p_weitht->imped_value,config->age,resp_fat_data.bmi);
                    }else{
                        resp_fat_data.fat = FAT_LOW_16_WOMAN(p_weitht->imped_value,config->age,resp_fat_data.bmi);
                    }
                }else{
                    if(config->age >= 16){
                        resp_fat_data.fat = FAT_HIGH_16_SPORT_WOMAN(p_weitht->imped_value,p_weitht->weight,config->height);
                    }else{
                        resp_fat_data.fat = FAT_LOW_16_WOMAN(p_weitht->imped_value,config->age,resp_fat_data.bmi);    //同普通人模式
                    }
                }
            }
            if(mask & 0x0002){
                resp_fat_data.muscle = MUSCLE(resp_fat_data.fat);
            }
            if(mask & 0x0004){
                if(config->user_mode == NORMAL_MODE){   //普通人模式
                    if(config->age >= 16){
                        resp_fat_data.water = WATER_HIGH_16_WOMAN(p_weitht->imped_value,p_weitht->weight,config->height);
                    }else{
                        resp_fat_data.water = WATER_LOW_16(resp_fat_data.fat);
                    }
                }else{
                    if(config->age >= 16){
                        resp_fat_data.water = WATER_HIGH_16_SPORT_WOMAN(p_weitht->imped_value,p_weitht->weight,config->height);
                    }else{
                        resp_fat_data.water = WATER_LOW_16(resp_fat_data.fat);
                    }
                }
            }
            if(mask & 0x0008){
                resp_fat_data.bone = BONE_WOMAN(resp_fat_data.fat);
            }
            if(mask & 0x0010){
                if(config->user_mode == NORMAL_MODE){
                    resp_fat_data.bmr = BMR_WOMAN(p_weitht->imped_value,config->age,p_weitht->weight,config->height);
                }else{
                    resp_fat_data.bmr = BMR_SPORT_WOMAM(p_weitht->imped_value,config->age,p_weitht->weight,config->height); 
                }
            }
            break;
    }

    if(resp_fat_data.fat < 40) 
        resp_fat_data.fat =40;
    else if(resp_fat_data.fat > 750) 
        resp_fat_data.fat =750;

    if(resp_fat_data.muscle < 10)
         resp_fat_data.muscle =10;
    else if(resp_fat_data.muscle > 900) 
        resp_fat_data.muscle =900;

    if(resp_fat_data.water < 10)
         resp_fat_data.water = 10;
    else if(resp_fat_data.water > 300) 
        resp_fat_data.water =300;

    if(resp_fat_data.bone< 10)
         resp_fat_data.bone =10;
    else if(resp_fat_data.bone > 300) 
        resp_fat_data.bone =300;

    if(resp_fat_data.bmr< 200)
         resp_fat_data.bmr =200;
    else if(resp_fat_data.bmr > 9000) 
        resp_fat_data.bmr =9000;

    if(resp_fat_data.bmi< 50)
         resp_fat_data.bmi =50;
    else if(resp_fat_data.bmi > 800) 
        resp_fat_data.bmi =800;

    if(false == bt_status){ //蓝牙断开 由固件下发体脂计算参数至称体;
        uint8_t rmask =1;
        uint8_t len =2;
        for(uint8_t i=0;i<5;i++,len +=2){
            rmask <<=1;
            if(rmask == mask){
                break;
            }            
        }
        ESP_LOGI(TAG, "fat:%d,muscle:%d,water:%d,bone=%d,bmr=%d,bmi=%d\n" ,resp_fat_data.fat,resp_fat_data.muscle,resp_fat_data.water,
                       resp_fat_data.bone,resp_fat_data.bmr,resp_fat_data.bmi);
        memcpy((user_fat_data_t *)&res->user_fat_data,(user_fat_data_t *)&resp_fat_data,sizeof(user_fat_data_t));

        resend_cmd_bit |= RESEND_CMD_BODY_FAT_BIT;
        app_uart_encode_send(MASTER_SET,CMD_BODY_FAT,(unsigned char *)&resp_fat_data,len,true); 
    }
    return true;
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
    static uint16_t oweight = 0;
    static uint16_t nweight = 0;
    static uint16_t o_imped_value = 0;
    static uint16_t n_imped_value = 0;


    if((1 == p_weitht->if_stabil) && (p_weitht->imped_value !=0)){    //判断是否稳定体重;
        user_config_data_t user_list[MAX_CONUT] ={0};        
        uint16_t len =0;

        if(vesync_flash_read(USER_MODEL_NAMESPACE,USER_MODEL_KEY,(char *)user_list,&len) != 0){
            ESP_LOGE(TAG, "user module NULL");
            return false;
        }
        ESP_LOGI(TAG, "flash user count:%d" ,len/sizeof(user_config_data_t));
        if(len == 0){                          //第一次使用
            oweight = nweight;
            nweight = (uint16_t)(p_weitht->weight);

            o_imped_value = n_imped_value;
            n_imped_value = p_weitht->imped_value;

            ESP_LOGI(TAG, "unit is %d",p_weitht->measu_unit);
            ESP_LOGI(TAG, "oweight=%d kg,nweight=%d kg ,o_imped_value=%d ohm,n_imped_value=%d ohm",oweight,nweight,o_imped_value,n_imped_value);

            if(((abs(o_imped_value - n_imped_value) <= MAX_IMPED) &&
                (abs(oweight - nweight) <= MAX_WEIGHT))||
                ((oweight == 0) && (nweight !=0))){               
                ret = true;
            }
        }else{
            uint16_t new_imped = 32;//p_weitht->imped_value; 调试屏蔽注释
            uint16_t new_kg = 6165;//(uint16_t)(p_weitht->weight);调试屏蔽注释
            uint8_t user_cnt =0;
            uint8_t i=0;

            for(;i<(len/sizeof(user_config_data_t));i++){
                ESP_LOGI(TAG, "store flash user config account:[0x%04x],kg:[%d],imped:[%d]",user_list[i].account,user_list[i].weight_kg,user_list[i].imped_value);
                if((abs(user_list[i].imped_value - new_imped) <= MAX_IMPED) &&          //前后两次阻抗小于30 ohm
                    (abs(user_list[i].weight_kg - new_kg) <= MAX_WEIGHT)){              //前后两次体重小于3kg
                    user_cnt++;
                    break;
                }
            }
            ESP_LOGI(TAG, "user_cnt =%d" ,user_cnt);
            if(user_cnt !=0){
                for(uint8_t k=0;k<user_cnt;k++){
                    ESP_LOGI(TAG, "match user is [0x%04x]",user_list[i].account);         //定位当前账户 并推送至云端
                }
                if(bt_status == false){
                    user_history_t history = {0};
                    history.account = user_list[i].account;
                    history.imped_value = new_imped;
                    history.utc_time = time((time_t *)NULL);

                    history.measu_unit = p_weitht->measu_unit;
                    history.weight_kg = p_weitht->weight;
                    history.weight_lb = p_weitht->lb;
                    
                    ESP_LOGI(TAG, "account [0x%04x],imped_value[0x%02x],utc_time [0x%04x],unit [0x%x] ,kg [0x%02x],lb [0x02%x]",
                        history.account,history.imped_value,history.utc_time,history.measu_unit,history.weight_kg,history.weight_lb);         //体脂参数计算正确
                    
                    if(body_fat_calc(bt_status,ALL_CALC,res,&user_list[i],p_weitht)){
                        ret = true;
                        ESP_LOGI(TAG, "fat calc success");         //体脂参数计算正确
                    }
                    vesync_flash_write(USER_HISTORY_DATA_NAMESPACE,USER_HISTORY_KEY,(user_history_t *)&history ,sizeof(user_history_t));
                }
            }
        }
    }    
    return ret;
}