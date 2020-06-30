#include "test_addr.h"


char g_DISABLE_MOCK = 0;    // global variable to check if we don't want to use
                            // a mocked function


static void test_get_Address(void **state) {

}


static void test_initalize_cpu(void **state) {
    g_DISABLE_MOCK = 1;
    struct cpu_t *cpu = initialize_cpu();
    g_DISABLE_MOCK = 0;
}


int main(void) {
    const struct CMUnitTest tests[] = {

    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
