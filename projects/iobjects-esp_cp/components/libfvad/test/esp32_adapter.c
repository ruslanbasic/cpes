#include "../tests/vad_unittest.h"
#include "esp32_adapter.h"
#include <stdio.h>
#include "unity.h"

void expect_fail(const char *s, const char *file, int line)
{
    printf("failed EXPECT: %s (%s:%d)\n", s, file, line);
    TEST_ASSERT(0);
}

void assert_fail(const char *s, const char *file, int line)
{
    printf("failed ASSERT: %s (%s:%d)\n", s, file, line);
    TEST_ASSERT(0);
}

