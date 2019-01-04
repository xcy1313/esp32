/**
 * @file esp_i2c.h
 * @brief esp32的i2c读写接口
 * @author WatWu
 * @date 2019-01-03
 */

#ifndef ESP_I2C_H
#define ESP_I2C_H

#include "driver/i2c.h"

/**
 * @brief iic初始化为主机模式
 * @param i2c_num       [iic编号，0或1]
 * @param sda_io        [SDA数据线引脚]
 * @param scl_io        [SCL时钟线引脚]
 * @param speed         [通信速率]
 * @return esp_err_t    [初始化结果]
 */
esp_err_t i2c_master_init(i2c_port_t i2c_num, uint8_t sda_io, uint8_t scl_io, uint32_t speed);

/**
 * @brief iic读取客户端数据
 * @param i2c_num       [iic编号，0或1]
 * @param addr          [操作的地址]
 * @param data_rd       [读取的数据缓存buffer]
 * @param size          [读取的数据大小]
 * @return esp_err_t    [读取结果]
 *_______________________________________________________________________________________
 * | start | slave_addr + rd_bit +ack | read n-1 bytes + ack | read 1 byte + nack | stop |
 * --------|--------------------------|----------------------|--------------------|------|
 */
esp_err_t i2c_master_read_slave(i2c_port_t i2c_num, uint8_t addr, uint8_t *data_rd, size_t size);

/**
 * @brief iic写数据到客户端
 * @param i2c_num       [iic编号，0或1]
 * @param addr          [操作的地址]
 * @param data_wr       [待写入的数据]
 * @param size          [待写入的数据大小]
 * @return esp_err_t    [写入结果]
 * ___________________________________________________________________
 * | start | slave_addr + wr_bit + ack | write n bytes + ack  | stop |
 * --------|---------------------------|----------------------|------|
 */
esp_err_t i2c_master_write_slave(i2c_port_t i2c_num, uint8_t addr, uint8_t *data_wr, size_t size);

#endif
