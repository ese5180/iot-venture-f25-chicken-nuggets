#include <zephyr/ztest.h>
#include "pir.h"

/* Test 1: pir_init() returns 0 when successful */
ZTEST(pir_test_suite, test_pir_init_success)
{
    int rc = pir_init();
    zassert_equal(rc, 0, "pir_init should return 0 on success");
}

ZTEST_SUITE(pir_test_suite, NULL, NULL, NULL, NULL, NULL);
