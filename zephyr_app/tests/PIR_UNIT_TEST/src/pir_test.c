#include <zephyr/ztest.h>
#include "pir.h"

/* Test 1: pir_init() returns 0 when successful */
ZTEST(pir_test_suite, test_pir_init_success)
{
    int rc = pir_init();
    zassert_equal(rc, 0, "pir_init should return 0 on success");
}

/* Test 2: pir_read() returns 0 by default (no motion) */
ZTEST(pir_test_suite, test_pir_read_default_no_motion)
{
    int val = pir_read();
    zassert_equal(val, 0, "pir_read should return 0 when no motion is detected");
}

/* Test 3: pir_set_state() updates pir_read() value */
ZTEST(pir_test_suite, test_pir_set_state)
{
    pir_set_state(1);
    zassert_equal(pir_read(), 1, "pir_read() should return 1 after motion is set");
}


ZTEST_SUITE(pir_test_suite, NULL, NULL, NULL, NULL, NULL);
