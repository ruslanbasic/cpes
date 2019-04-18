#include "unity.h"

#include "esp32_adapter.h"

#define test_main() test_main_vad_gmm_unittest()
#include "../tests/vad_gmm_unittest.c"

TEST_CASE("vad_gmm_unittest", "[libfvad]")
{
  test_main_vad_gmm_unittest();
}
