#include "unity.h"

#include "esp32_adapter.h"

#define TEST_VAD_API
#define test_main() test_main_vad_unittest()
#include "../tests/vad_unittest.c"

TEST_CASE("vad_unittest", "[libfvad]")
{
  test_main_vad_unittest();
}
