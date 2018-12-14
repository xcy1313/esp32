/**
 * @file vesync_bt_driver.c
 * @brief 蓝牙业务逻辑驱动
 * @author Jiven 
 * @date 2018-12-14
 */
#include "vesync_bt_driver.h"
#include "vesync_bt_hal.h"
#include "vesync_wifi.h"
#include "vesync_log.h"

static const char *TAG = "vesync_bt_driver";

bt_frame_t  bt_prase ={0};

/**
 * @brief 队列解析蓝牙接收数据
 * @param hw_data 硬件设备信息
 * @param data_buf app下发数据
 * @param length 
 */
static void bt_event_handler(const void *reg,const void*data_buf, unsigned short length)
{
    for(int i=0;i<length;i++){
        if(bt_data_frame_decode(*(unsigned char *)&data_buf[i],0,&bt_prase) == 1){
            frame_ctrl_t res_ctl ={     //应答包res状态  
                .data =0,
            };
            struct{                     //应答数据包
                uint8_t buf[20];
                uint8_t len;
            }resp_strl ={{0},0};         
            uint8_t *cnt = NULL;
            uint8_t *opt = &bt_prase.frame_data[0]; //指针指向data_buf数据域；
            uint8_t len = bt_prase.frame_data_len-sizeof(uint16_t); //长度减去包含2个字节的命令包
            cnt = &bt_prase.frame_cnt;
            LOG_I(TAG, "payload len[%d]",len);
        }
    }
}
/**
 * @brief 
 * @param ctl 
 * @param cnt 
 * @param cmd 
 * @param notify_data 
 * @param len 
 * @return int 
 */
int vesync_notify_send(frame_ctrl_t ctl,uint8_t *cnt,uint16_t cmd,const void *notify_data ,unsigned short len)
{
    return vesync_bt_notify(ctl,cnt, cmd,notify_data ,len);
}

/**
 * @brief true使能蓝牙配网功能
 * @param version bit[7 : 4]:保留 ; bit[3 : 0]:数据格式版本号，当前版本为1；即当前该字节为：0x01
 * @param product_type 产品类型
 * @param product_num  产品编码
 * @param custom       自定义数据流
 * @param enable_blufi 是否使能蓝牙配网
 */
void vesync_init_bt_module(char *adver_name,uint8_t version,uint8_t product_type,uint8_t product_num,uint8_t *custom,bool enable_blufi)
{
    vesync_hal_bt_client_init(adver_name,version,product_type,product_num,custom,enable_blufi, bt_event_handler);
}