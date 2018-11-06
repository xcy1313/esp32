
#include "body_fat_calc.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG = "body";

#define MAX_WEIGHT      150     //+-三公斤
#define MAX_IMPED       30      //+-30ohm

#define ff(x)	        (((int)(x+0.5))>((int)x)?((int)x+1):((int)x))
#define kg_to_lb(x)     ff((x)*2.20462262)
#define lb_to_kg(x)     ff((x)*0.45359237)
#define lb_to_g(x)      ff(((x)*453.59237)

bool body_fat_person(response_weight_data_t *p_weitht)
{
    bool ret = false;
    static uint8_t backup_unit =0;
    static uint8_t new_unit =0;
    static uint16_t oweight = 0;
    static uint16_t nweight = 0;
    static uint16_t o_imped_value = 0;
    static uint16_t n_imped_value = 0;

    ESP_LOGE(TAG, "body_fat_person\n");

    if(1 == p_weitht->if_stabil){    //判断是否稳定体重;
        backup_unit = new_unit;
        new_unit = p_weitht->measu_unit;

        oweight = nweight;
        nweight = (uint16_t)(p_weitht->weight>>8);
        
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
        ESP_LOGE(TAG, "oweight=%d,nweight=%d\n" ,oweight ,nweight);

        if((abs(oweight - nweight) < MAX_WEIGHT) &&
            (abs(o_imped_value - n_imped_value) < MAX_IMPED)){
            ret = true;
        }
    }    
    ESP_LOGE(TAG, "ret is %d\n" ,ret);

    return ret;
}
