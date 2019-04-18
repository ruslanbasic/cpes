#include "unity.h"

#include "esp32_adapter.h"

#define TEST_VAD_CORE_SET_MODE
#define test_main() test_main_vad_core_set_mode()
#include "../tests/vad_core_unittest.c"

TEST_CASE("test_main_vad_core_set_mode", "[libfvad]")
{
  test_main();
}

#undef TEST_VAD_CORE_SET_MODE
#undef test_main
#define TEST_VAD_CORE_INIT
#define test_main() test_main_vad_core_init()
#include "../tests/vad_core_unittest.c"

TEST_CASE("vad_core_init", "[libfvad]")
{
  test_main_vad_core_init();
}

#undef TEST_VAD_CORE_INIT
#undef test_main
#define TEST_VAD_CORE_CALC_VAD
#define test_main() test_main_vad_core_calc_vad()
#include "../tests/vad_core_unittest.c"

TEST_CASE("vad_core_calc_vad", "[libfvad]")
{
  test_main_vad_core_calc_vad();
}

