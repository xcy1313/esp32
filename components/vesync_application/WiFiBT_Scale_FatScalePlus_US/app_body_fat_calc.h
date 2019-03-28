/**
 * @file app_body_fat_calc.h
 * @brief 称体体脂参数计算
 * @author Jiven 
 * @date 2018-12-28
 */
#ifndef _APP_BODY_FAT_CALC_H
#define _APP_BODY_FAT_CALC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "app_handle_scales.h"

#define ALL_CALC    0x1F

#define MAX_IMPED_VALUE   630
#define MIN_IMPED_VALUE   430

extern char    mask_user_store_key[12];
extern uint32_t match_account_id;

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