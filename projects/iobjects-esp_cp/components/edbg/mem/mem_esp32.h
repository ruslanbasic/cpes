#define malloc(size)        edbg_mem_esp32_malloc(size)
#define calloc(n, size)     edbg_mem_esp32_calloc(n, size)
#define realloc(ptr, size)  edbg_mem_esp32_realloc(ptr, size)