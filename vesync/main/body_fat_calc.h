
#ifndef _BODY_FAT_CALC_H
#define _BODY_FAT_CALC_H

#include "vesync_uart.h"


enum{
    UNIT_KG,
    UNIT_LB,
    UNIT_ST,
}UNIT;






bool body_fat_person(hw_info *res,response_weight_data_t *p_weitht);














#endif