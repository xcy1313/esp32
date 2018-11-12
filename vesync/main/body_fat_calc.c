
#include "body_fat_calc.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG = "body_FAT";

#define SPORT_MODE      0   //运动员模式;
#define NORMAL_MODE     1   //普通人模式；

#define MAX_WEIGHT      3       //+-三公斤
#define MAX_IMPED       30      //+-30ohm

#define ff(x)	        (((int)(x+0.5))>((int)x)?((int)x+1):((int)x))
#define kg_to_lb(x)     ff((x)*2.20462262)
#define lb_to_kg(x)     ff((x)*0.45359237)
#define lb_to_g(x)      ff(((x)*453.59237)

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
 * @brief 
 * @param mask 
 * @param res 
 * @param p_weitht 
 * @return true 
 * @return false 
 */
bool body_fat_calc(bool bt_status,uint16_t mask ,hw_info *res,response_weight_data_t *p_weitht)
{    
    res->user_fat_data.bmi = BMI((uint16_t)(p_weitht->weight>>16),res->user_config_data.height);

    switch(res->user_config_data.gender){
        case 0: //男人
            if(mask & 0x0001){    //仅仅处理部分
                if(res->user_config_data.user_mode == NORMAL_MODE){   //普通人模式
                    if(res->user_config_data.age >= 16){
                        res->user_fat_data.fat = FAT_HIGH_16_MAN(p_weitht->imped_value,res->user_config_data.age,res->user_fat_data.bmi);
                    }else{
                        res->user_fat_data.fat = FAT_LOW_16_MAN(p_weitht->imped_value,res->user_config_data.age,res->user_fat_data.bmi);
                    }
                }else{
                    if(res->user_config_data.age >= 16){
                        res->user_fat_data.fat = FAT_HIGH_16_SPORT_MAN(p_weitht->imped_value,((uint16_t)(p_weitht->weight>>16)),res->user_config_data.height);
                    }else{
                        res->user_fat_data.fat = FAT_LOW_16_MAN(p_weitht->imped_value,res->user_config_data.age,res->user_fat_data.bmi);    //同普通人模式
                    }
                }
            }
            if(mask & 0x0002){
                res->user_fat_data.muscle = MUSCLE(res->user_fat_data.fat);
            }
            if(mask & 0x0004){
                if(res->user_config_data.user_mode == NORMAL_MODE){   //普通人模式
                    if(res->user_config_data.age >= 16){
                        res->user_fat_data.water = WATER_HIGH_16_MAN(p_weitht->imped_value,((uint16_t)(p_weitht->weight>>16)),res->user_config_data.height);
                    }else{
                        res->user_fat_data.water = WATER_LOW_16(res->user_fat_data.fat);
                    }
                }else{
                    if(res->user_config_data.age >= 16){
                        res->user_fat_data.water = WATER_HIGH_16_SPORT_MAN(p_weitht->imped_value,((uint16_t)(p_weitht->weight>>16)),res->user_config_data.height);
                    }else{
                        res->user_fat_data.water = WATER_LOW_16(res->user_fat_data.fat);
                    }
                }
            }
            if(mask & 0x0008){
                    res->user_fat_data.bone = BONE_MAN(res->user_fat_data.fat);
            }
            if(mask & 0x0010){
                if(res->user_config_data.user_mode == NORMAL_MODE){   //普通人模式
                    res->user_fat_data.bmr = BMR_MAN(p_weitht->imped_value,res->user_config_data.age,((uint16_t)(p_weitht->weight>>16)),res->user_config_data.height);
                }else{
                    res->user_fat_data.bmr = BMR_SPORT_MAM(p_weitht->imped_value,res->user_config_data.age,((uint16_t)(p_weitht->weight>>16)),res->user_config_data.height);
                }
            }
            break;
        case 1: //女人
            if(mask & 0x0001){
                if(res->user_config_data.user_mode == NORMAL_MODE){   //普通人模式
                    if(res->user_config_data.age >= 16){
                        res->user_fat_data.fat = FAT_HIGH_16_WOMAN(p_weitht->imped_value,res->user_config_data.age,res->user_fat_data.bmi);
                    }else{
                        res->user_fat_data.fat = FAT_LOW_16_WOMAN(p_weitht->imped_value,res->user_config_data.age,res->user_fat_data.bmi);
                    }
                }else{
                    if(res->user_config_data.age >= 16){
                        res->user_fat_data.fat = FAT_HIGH_16_SPORT_WOMAN(p_weitht->imped_value,((uint16_t)(p_weitht->weight>>16)),res->user_config_data.height);
                    }else{
                        res->user_fat_data.fat = FAT_LOW_16_WOMAN(p_weitht->imped_value,res->user_config_data.age,res->user_fat_data.bmi);    //同普通人模式
                    }
                }
            }
            if(mask & 0x0002){
                res->user_fat_data.muscle = MUSCLE(res->user_fat_data.fat);
            }
            if(mask & 0x0004){
                if(res->user_config_data.user_mode == NORMAL_MODE){   //普通人模式
                    if(res->user_config_data.age >= 16){
                        res->user_fat_data.water = WATER_HIGH_16_WOMAN(p_weitht->imped_value,((uint16_t)(p_weitht->weight>>16)),res->user_config_data.height);
                    }else{
                        res->user_fat_data.water = WATER_LOW_16(res->user_fat_data.fat);
                    }
                }else{
                    if(res->user_config_data.age >= 16){
                        res->user_fat_data.water = WATER_HIGH_16_SPORT_WOMAN(p_weitht->imped_value,((uint16_t)(p_weitht->weight>>16)),res->user_config_data.height);
                    }else{
                        res->user_fat_data.water = WATER_LOW_16(res->user_fat_data.fat);
                    }
                }
            }
            if(mask & 0x0008){
                res->user_fat_data.bone = BONE_WOMAN(res->user_fat_data.fat);
            }
            if(mask & 0x0010){
                if(res->user_config_data.user_mode == NORMAL_MODE){
                    res->user_fat_data.bmr = BMR_WOMAN(p_weitht->imped_value,res->user_config_data.age,((uint16_t)(p_weitht->weight>>16)),res->user_config_data.height);
                }else{
                    res->user_fat_data.bmr = BMR_SPORT_WOMAM(p_weitht->imped_value,res->user_config_data.age,((uint16_t)(p_weitht->weight>>16)),res->user_config_data.height); 
                }
            }
            break;
    }

    if(res->user_fat_data.fat < 40) 
        res->user_fat_data.fat =40;
    else if(res->user_fat_data.fat > 600) 
        res->user_fat_data.fat =600;

    ESP_LOGI(TAG, "fat:%d,muscle:%d,water:%d,bone=%d,bmr=%d,bmi=%d\n" ,res->user_fat_data.fat,res->user_fat_data.muscle,res->user_fat_data.water,
                                        res->user_fat_data.bone,res->user_fat_data.bmr,res->user_fat_data.bmi);
    if(0 == bt_status){ //蓝牙断开
        uart_encode_send(MASTER_SET,CMD_BODY_FAT,(char *)&res->user_fat_data,12); 
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
bool body_fat_person(hw_info *res ,response_weight_data_t *p_weitht)
{
    bool ret = false;
    static uint8_t backup_unit =0;
    static uint8_t new_unit =0;
    static uint16_t oweight = 0;
    static uint16_t nweight = 0;
    static uint16_t o_imped_value = 0;
    static uint16_t n_imped_value = 0;

    ESP_LOGI(TAG, "body_fat_person\n");

    if(0 == res->user_config_data.length){  //称体未设置用户数据 不做体脂计算处理 直接返回
        ESP_LOGI(TAG, " do not set user config data crc8:[%d]\n",res->user_config_data.crc8);
        return false;
    }

    if(1 == p_weitht->if_stabil){    //判断是否稳定体重;
        backup_unit = new_unit;
        new_unit = p_weitht->measu_unit;

        oweight = nweight;
        nweight = (uint16_t)(p_weitht->weight>>16);
        
        o_imped_value = n_imped_value;
        n_imped_value = p_weitht->imped_value;

        switch(p_weitht->measu_unit){
            case UNIT_KG:
                if(backup_unit == UNIT_LB){
                    oweight = lb_to_kg(oweight);
                }else if(backup_unit == UNIT_ST){

                }
                break;
            case UNIT_LB:
                if(backup_unit == UNIT_LB){   //前后两次计算的单位不一致 需要统一成kg计算
                    oweight = lb_to_kg(oweight);
                }
                nweight = lb_to_kg(nweight);
                break;
            case UNIT_ST:

                break;
            default:
                return false;
        }
        ESP_LOGI(TAG, "oweight=%d,nweight=%d,userweight =%d\n",oweight,nweight,res->user_config_data.weight);

        //此处需要遍历固件端保存用户数据总数作比较来决定是否为绑定数据
        if((abs(o_imped_value - n_imped_value) <= MAX_IMPED) &&              //前后两次阻抗小于30 ohm
            (abs(nweight - res->user_config_data.weight) <= MAX_WEIGHT)){    //当前实际值与设置值误差小于三公斤
            ret = true;
        }else if((oweight == 0) && (res->user_config_data.length !=0)){  //上电第一次使用
            ret = true;
        }   
    }    
    ESP_LOGI(TAG, "ret is %d\n" ,ret);
    
    if(ret){    //判断是同一用户 进行体脂计算处理 并下发称体参数
        body_fat_calc(false,0x1f,res,p_weitht);
    }
    return ret;
}
