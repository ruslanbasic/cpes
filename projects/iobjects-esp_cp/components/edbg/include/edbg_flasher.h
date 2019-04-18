#include <stdlib.h>
#include <stdint.h>

void edbg_initialize();
void edbg_hold_reset();
void edbg_erase_flash_verify(uint8_t* data, size_t data_sz);
bool edbg_verify(uint8_t* data, size_t data_sz);
void edbg_deinitialize();