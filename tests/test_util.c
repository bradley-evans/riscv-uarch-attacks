#include "test_util.h"


/**
 * @brief      Null test to verify cmocka is working
 *
 * @param      state  The state. Unused.
 */
static void util_null_test_success(void **state) {}


static void concat_test(void **state) {
    char *s1 = malloc(sizeof(char));
    strcpy(s1, "abcdefg");
    char *s2 = malloc(sizeof(char));
    strcpy(s2, "hijklmn");

    assert_true(strcmp(concat(s1,s2),"abcdefghijklmn") == 0);
}


int test_util(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(util_null_test_success),
        cmocka_unit_test(concat_test)
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
