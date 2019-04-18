/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#pragma once /****************************************************************/

#include <stdint.h>
#include "driver/i2c.h"

uint16_t smbus_read_word(i2c_port_t i2c_port, uint8_t address, uint8_t command);
void smbus_write_word(i2c_port_t i2c_port, uint8_t address, uint8_t command, uint16_t data);

/*****************************************************************************/
