/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#define malloc(size)        kws_recorder_mem_esp32_malloc(size)
#define calloc(n, size)     kws_recorder_mem_esp32_calloc(n, size)
#define realloc(ptr, size)  kws_recorder_mem_esp32_realloc(ptr, size)

/*****************************************************************************/
