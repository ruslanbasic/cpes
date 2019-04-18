#include "unity.h"

#include "esp32_adapter.h"

#define TEST_SPL_MACRO
#define test_main() test_main_vad_spl_macro()
#include "../tests/signal_processing_unittest.c"

TEST_CASE("test_main_vad_spl_macro", "[libfvad]")
{
  test_main();
}

#undef TEST_SPL_MACRO
#undef test_main
#define TEST_SPL_INLINE
#define test_main() test_main_vad_spl_inline()
#include "../tests/signal_processing_unittest.c"

TEST_CASE("test_main_vad_spl_inline", "[libfvad]")
{
  test_main_vad_spl_inline();
}

#undef TEST_SPL_INLINE
#undef test_main
#define TEST_SPL_LEADING_ZEROS
#define test_main() test_main_vad_spl_leading_zeroes()
#include "../tests/signal_processing_unittest.c"

TEST_CASE("test_main_vad_spl_leading_zeroes", "[libfvad]")
{
  test_main_vad_spl_leading_zeroes();
}

#undef TEST_SPL_LEADING_ZEROS
#undef test_main
#define TEST_SPL_MATH_OPERATIONS
#define test_main() test_main_vad_spl_math_operations()
#include "../tests/signal_processing_unittest.c"

TEST_CASE("test_main_vad_spl_math_operations", "[libfvad]")
{
  test_main_vad_spl_math_operations();
}

#undef TEST_SPL_MATH_OPERATIONS
#undef test_main
#define TEST_SPL_SIGNAL_PROCESSING
#define test_main() test_main_vad_signal_processing()
#include "../tests/signal_processing_unittest.c"

TEST_CASE("test_main_vad_signal_processing", "[libfvad]")
{
  test_main_vad_signal_processing();
}

#undef TEST_SPL_SIGNAL_PROCESSING
#undef test_main
#define TEST_SPL_RESAMPLE_48
#define test_main() test_main_vad_spl_resample_48()
#include "../tests/signal_processing_unittest.c"

TEST_CASE("test_main_vad_spl_resample_48", "[libfvad]")
{
  test_main_vad_spl_resample_48();
}
