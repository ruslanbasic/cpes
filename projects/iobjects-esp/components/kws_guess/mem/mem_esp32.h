#define malloc(size)        kws_guess_mem_esp32_malloc(size)
#define calloc(n, size)     kws_guess_mem_esp32_calloc(n, size)
#define realloc(ptr, size)  kws_guess_mem_esp32_realloc(ptr, size)