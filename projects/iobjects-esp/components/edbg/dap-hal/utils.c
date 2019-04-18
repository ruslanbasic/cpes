#include "dap_config.h"
#include "edbg_flasher.h"

void edbg_hold_reset()
{
  HAL_GPIO_RESET_out();
  HAL_GPIO_RESET_clr();
}