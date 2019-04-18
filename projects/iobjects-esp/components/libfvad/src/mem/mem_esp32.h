#define malloc(size)        libfvad_mem_esp32_malloc(size)
#define calloc(n, size)     libfvad_mem_esp32_calloc(n, size)
#define realloc(ptr, size)  libfvad_mem_esp32_realloc(ptr, size)