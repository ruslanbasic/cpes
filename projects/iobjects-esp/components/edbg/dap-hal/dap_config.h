/*
 * Copyright (c) 2016, Alex Taradov <alex@taradov.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _DAP_CONFIG_H_
#define _DAP_CONFIG_H_

/*- Includes ----------------------------------------------------------------*/
#include "hal_gpio.h"
#include "sdkconfig.h"

HAL_GPIO_PIN(SWCLK, CONFIG_EDBG_SWCLK_GPIO)
HAL_GPIO_PIN(SWDIO, CONFIG_EDBG_SWDIO_GPIO)
HAL_GPIO_PIN(RESET, CONFIG_EDBG_RESET_GPIO)

#define DAP_CONFIG_CLOCK_DELAY         100
#define DAP_CONFIG_PACKET_COUNT        1

// Set the value to NULL if you want to disable a string
// DAP_CONFIG_PRODUCT_STR must contain "CMSIS-DAP" to be compatible with the standard
#define DAP_CONFIG_VENDOR_STR          "LinkLine 2018"
#define DAP_CONFIG_PRODUCT_STR         "Generic CMSIS-DAP Adapter"
#define DAP_CONFIG_SER_NUM_STR         "123456"
#define DAP_CONFIG_FW_VER_STR          "v0.1"
#define DAP_CONFIG_DEVICE_VENDOR_STR   NULL
#define DAP_CONFIG_DEVICE_NAME_STR     NULL

/*- Implementations ---------------------------------------------------------*/

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SWCLK_write(int value)
{
  HAL_GPIO_SWCLK_write(value);
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SWDIO_write(int value)
{
  HAL_GPIO_SWDIO_write(value);
}

//-----------------------------------------------------------------------------
static inline int DAP_CONFIG_SWCLK_read(void)
{
  return HAL_GPIO_SWCLK_read();
}

//-----------------------------------------------------------------------------
static inline int DAP_CONFIG_SWDIO_read(void)
{
  return HAL_GPIO_SWDIO_read();
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SWCLK_set(void)
{
  HAL_GPIO_SWCLK_set();
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SWCLK_clr(void)
{
  HAL_GPIO_SWCLK_clr();
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SWDIO_in(void)
{
  HAL_GPIO_SWDIO_in();
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SWDIO_out(void)
{
  HAL_GPIO_SWDIO_out();
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SETUP(void)
{
  HAL_GPIO_SWCLK_in();

  HAL_GPIO_SWDIO_in();
  HAL_GPIO_SWDIO_pullup();
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_DISCONNECT(void)
{
  HAL_GPIO_SWCLK_in();
  HAL_GPIO_SWDIO_in();
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_CONNECT_SWD(void)
{
  HAL_GPIO_SWDIO_out();
  HAL_GPIO_SWDIO_set();

  HAL_GPIO_SWCLK_out();
  HAL_GPIO_SWCLK_set();
}

#endif // _DAP_CONFIG_H_
