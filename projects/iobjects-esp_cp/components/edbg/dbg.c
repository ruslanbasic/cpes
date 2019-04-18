/*
 * Copyright (c) 2013-2015, Alex Taradov <alex@taradov.com>
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

/*- Includes ----------------------------------------------------------------*/
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "edbg.h"
#include "dbg.h"
#include <assert.h>
#include "dap-hal/dap.h"

//-----------------------------------------------------------------------------
int dbg_enumerate(debugger_t *debuggers, int size)
{
  return 1;
}

//-----------------------------------------------------------------------------
void dbg_open(debugger_t *debugger)
{
  dap_init();
}

//-----------------------------------------------------------------------------
void dbg_close(void)
{
}

//-----------------------------------------------------------------------------
int dbg_get_report_size(void)
{
  return DAP_CONFIG_PACKET_SIZE;
}

//-----------------------------------------------------------------------------
int dbg_dap_cmd(uint8_t *data, int size, int rsize)
{
  char cmd = data[0];
  int res = DAP_CONFIG_PACKET_SIZE;

  uint8_t request[DAP_CONFIG_PACKET_SIZE+1];
  assert(rsize <= sizeof request);
  uint8_t response[DAP_CONFIG_PACKET_SIZE+1];

  memset(request, 0xff, sizeof response);
  memcpy(&request[0], data, rsize);
  memset(response, 0xff, sizeof response);

  // for (int i = 0; i < sizeof request; i++)
  // {
  //     printf("%02X", request[i]);
  // }
  // printf(" => ");

  dap_process_request(request, response);

  // for (int i = 0; i < sizeof response; i++)
  // {
  //     printf("%02X", response[i]);
  // }
  // printf("\n");

  check(response[0] == cmd, "invalid response received");

  res--;
  memcpy(data, &response[1], (size < res) ? size : res);

  return res;
}

