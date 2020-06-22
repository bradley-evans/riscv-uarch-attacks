#include "test_low.h"


/**
 * @brief      Null test to verify cmocka is working
 *
 * @param      state  The state. Unused.
 */
static void low_null_test_success(void **state) {}
}


int test_low(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(low_null_test_success),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
