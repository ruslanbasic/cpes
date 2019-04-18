/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "smbus.h"
#include "crc8.h"
#include "sdkconfig.h"

#define ACK_CHECK_EN   0x1     /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS  0x0     /*!< I2C master will not check ack from slave */
#define ACK_VAL        0x0     /*!< I2C ack value */
#define NACK_VAL       0x1     /*!< I2C nack value */

uint16_t smbus_read_word(i2c_port_t i2c_port, uint8_t address, uint8_t command)
{
  uint16_t value = 0;
  uint8_t data_l = 0;
  uint8_t data_h = 0;
  uint8_t pec = 0;

  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, address << 1 | I2C_MASTER_WRITE, ACK_CHECK_EN);
  i2c_master_write_byte(cmd, command, NACK_VAL);
  i2c_master_cmd_begin(i2c_port, cmd, 1000 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);

  cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, address << 1 | I2C_MASTER_READ, ACK_CHECK_EN);
  i2c_master_read_byte(cmd, &data_l, ACK_VAL);
  i2c_master_read_byte(cmd, &data_h, ACK_VAL);
  i2c_master_read_byte(cmd, &pec, NACK_VAL);
  i2c_master_stop(cmd);
  i2c_master_cmd_begin(i2c_port, cmd, 1000 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);

  const uint16_t ERROR = 0xFFFF;

  uint8_t buf[5] = {address << 1, command,  address << 1 | I2C_MASTER_READ, data_l, data_h};
  uint8_t ok = crc8(buf, sizeof(buf));

  if(pec == ok) value = ((uint16_t) data_h << 8) | ((uint16_t) data_l << 0);
  else          value = ERROR;

//  printf("command = $%X \n", command);
//  printf("data_l = $%X \n", data_l);
//  printf("data_h = $%X \n", data_h);
//  printf("pec = $%X \n", pec);
//
//  printf("esp32 pec = $%X \n", ok);

  return value;

}

//void smbus_write_word(i2c_port_t i2c_port, uint8_t address, uint8_t command, uint16_t data)
//{
//  uint8_t data_l = data & 0xFF;
//  uint8_t data_h = data >> 8;
//  uint8_t buf[3] = {command, data_l, data_h};
//  uint8_t pec = crc8_block(0, buf, sizeof(buf));
//
//  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
//  i2c_master_start(cmd);
//  i2c_master_write_byte(cmd, address << 1 | I2C_MASTER_WRITE, ACK_CHECK_EN); //address
//  i2c_master_write_byte(cmd, command, NACK_VAL); //REG
//  i2c_master_write_byte(cmd, data_l, NACK_VAL); //LSB
//  i2c_master_write_byte(cmd, data_h, NACK_VAL); //MSB
//  i2c_master_write_byte(cmd, pec, NACK_VAL); //PEC 0x63 --> 2E5000
//  i2c_master_stop(cmd);
//  i2c_master_cmd_begin(i2c_port, cmd, 1000 / portTICK_RATE_MS);
//  i2c_cmd_link_delete(cmd);
//}

/*****************************************************************************/
