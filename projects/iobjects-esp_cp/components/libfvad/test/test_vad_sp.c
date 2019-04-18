#include "unity.h"

#include "esp32_adapter.h"

#define test_main() test_main_vad_sp_unittest()
#include "../tests/vad_sp_unittest.c"

TEST_CASE("vad_sp_unittest", "[libfvad]")
{
  test_main_vad_sp_unittest();
}
