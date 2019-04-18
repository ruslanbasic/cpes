#include <stdio.h>
#include "esp_attr.h"
#include "esp_heap_caps.h"

IRAM_ATTR void *kws_guess_mem_esp32_malloc( size_t size )
{
    return heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
}

IRAM_ATTR void *kws_guess_mem_esp32_calloc( size_t n, size_t size )
{
    return heap_caps_calloc(n, size, MALLOC_CAP_SPIRAM);
}

IRAM_ATTR void *kws_guess_mem_esp32_realloc( void *ptr, size_t size )
{
    return heap_caps_realloc(ptr, size, MALLOC_CAP_SPIRAM);
}