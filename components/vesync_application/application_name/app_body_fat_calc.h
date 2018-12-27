
#ifndef _APP_BODY_FAT_CALC_H
#define _APP_BODY_FAT_CALC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "app_handle_scales.h"

#define ALL_CALC    0X1F

enum{
    UNIT_KG,
    UNIT_LB,
    UNIT_ST,
}UNIT;





/**
 * @brief 根据flash存储用户模型 判断是否为绑定的用户数据
 * @param bt_status 
 * @param res 
 * @param p_weitht 
 * @return true 
 * @return false 
 */
bool body_fat_person(bool bt_status,hw_info *res ,response_weight_data_t *p_weitht);














#endif