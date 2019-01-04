/**
 * @file esp_i2c.c
 * @brief esp32的i2c读写接口
 * @author WatWu
 * @date 2019-01-03
 */

#include <stdio.h>

#include "vesync_log.h"

#include "esp_i2c.h"

// static const char *TAG = "esp_i2c";

#define I2C_MASTER_TX_BUF_DISABLE   0                           /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0                           /*!< I2C master doesn't need buffer */

#define WRITE_BIT                   I2C_MASTER_WRITE            /*!< I2C master write */
#define READ_BIT                    I2C_MASTER_READ             /*!< I2C master read */
#define ACK_CHECK_EN                0x01                        /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS               0x00                        /*!< I2C master will not check ack from slave */
#define ACK_VAL                     0x00                        /*!< I2C ack value */
#define NACK_VAL                    0x01                        /*!< I2C nack value */

/**
 * @brief iic初始化为主机模式
 * @param i2c_num       [iic编号，0或1]
 * @param sda_io        [SDA数据线引脚]
 * @param scl_io        [SCL时钟线引脚]
 * @param speed         [通信速率]
 * @return esp_err_t    [初始化结果]
 */
esp_err_t i2c_master_init(i2c_port_t i2c_num, uint8_t sda_io, uint8_t scl_io, uint32_t speed)
{
    int i2c_master_port = i2c_num;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = sda_io;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = scl_io;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = speed;
    i2c_param_config(i2c_master_port, &conf);
    return i2c_driver_install(i2c_master_port, conf.mode,
                              I2C_MASTER_RX_BUF_DISABLE,
                              I2C_MASTER_TX_BUF_DISABLE, 0);
}

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
esp_err_t i2c_master_read_slave(i2c_port_t i2c_num, uint8_t addr, uint8_t *data_rd, size_t size)
{
    if(size == 0)
    {
        return ESP_OK;
    }
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | READ_BIT, ACK_CHECK_EN);
    if(size > 1)
    {
        i2c_master_read(cmd, data_rd, size - 1, ACK_VAL);
    }
    i2c_master_read_byte(cmd, data_rd + size - 1, NACK_VAL);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

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
esp_err_t i2c_master_write_slave(i2c_port_t i2c_num, uint8_t addr, uint8_t *data_wr, size_t size)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write(cmd, data_wr, size, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}
