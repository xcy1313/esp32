/**
 * @file vesync_developer.h
 * @brief vesync设备SDK开发者模式
 * @author WatWu
 * @date 2018-08-03
 */

#ifndef VESYNC_DEVELOPER_H
#define VESYNC_DEVELOPER_H

/**
 * @brief 启动开发者模式
 * @return int [启动结果，0为成功]
 */
int vesync_developer_start(void);

/**
 * @brief 直接串口打印系统任务管理器
 */
void printf_os_task_manager(void);

int developer_tcp_send(uint8_t *data, uint32_t length);

#endif
