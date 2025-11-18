#include <zephyr/ztest.h>
#include "pir.h"

/* Test 1: pir_init() returns 0 when successful */
ZTEST(pir_test_suite, test_pir_init_success)
{
    int rc = pir_init();
    zassert_equal(rc, 0, "pir_init should return 0 on success");
}

ZTEST_SUITE(pir_test_suite, NULL, NULL, NULL, NULL, NULL);

/* Test 2: pir_is_motion_detected returns 1 when GPIO reads high */
ZTEST(pir_test_suite, test_pir_motion_detected_high)
{
    mock_pir_gpio_value = 1; // <-- we will add this global mock

    int detected = pir_is_motion_detected();

    zassert_equal(detected, 1, "Expected motion detected when GPIO = 1");
}
