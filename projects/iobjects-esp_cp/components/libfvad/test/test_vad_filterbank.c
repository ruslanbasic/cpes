#include "unity.h"

#include "esp32_adapter.h"

#define test_main() test_main_vad_filterbank_unittest()
#include "../tests/vad_filterbank_unittest.c"

TEST_CASE("vad_filterbank_unittest", "[libfvad]")
{
  test_main_vad_filterbank_unittest();
}
