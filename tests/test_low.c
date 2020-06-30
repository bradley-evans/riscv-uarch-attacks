#include "test_low.h"


char g_DISABLE_MOCK = 0;    // global variable to check if we don't want to use
                            // a mocked function

char* __wrap_get_StringFromSysFile(char filename[]) {
    printf("in mock sysstring reader\n");
    if (g_DISABLE_MOCK) {
        printf("mocking disabled, calling real function\n");
        char *real = __real_get_StringFromSysFile(filename);
        printf("got the real value, %s\n", real);
        return real;
    }
    return (mock());
}


struct dirent *__wrap_readdir(DIR *dirp) {
    if (g_DISABLE_MOCK) {
        return __real_readdir(dirp);
    }
    return (mock());
}


static void test_0_get_numCPUOnline(void **state) {
    char *returnval = malloc(sizeof(char));
    strcpy(returnval, "0");
    will_return(__wrap_get_StringFromSysFile, returnval);
    int fromfunction = get_numCPUOnline();
    assert_int_equal(1,fromfunction);
}


static void test_16_get_numCPUOnline(void **state) {
    char *returnval = malloc(sizeof(char));
    strcpy(returnval, "0-15");
    will_return(__wrap_get_StringFromSysFile, returnval);
    int fromfunction = get_numCPUOnline();
    assert_int_equal(16,fromfunction);
}


static void test_get_CacheParameters(void **state) {
    char *ways = malloc(sizeof(char));
    strcpy(ways,"4"); // Ways
    will_return(__wrap_get_StringFromSysFile, ways);

    char *level = malloc(sizeof(char));
    strcpy(level,"1"); // Level
    will_return(__wrap_get_StringFromSysFile, level);

    char *type = malloc(sizeof(char));    
    strcpy(type,"Data"); // Type
    will_return(__wrap_get_StringFromSysFile, type);


    char *size = malloc(sizeof(char));     
    strcpy(size,"8K"); // Size
    will_return(__wrap_get_StringFromSysFile, size);

    char *sets = malloc(sizeof(char));     
    strcpy(sets,"64"); // Sets
    will_return(__wrap_get_StringFromSysFile, sets);


    char *blocksize = malloc(sizeof(char));     
    strcpy(blocksize,"4096"); // Blocksize
    will_return(__wrap_get_StringFromSysFile, blocksize);

    struct cache_t cache = get_CacheParameters(0,0);

    assert_int_equal(cache.ways, 4);
    assert_int_equal(cache.level, 1);
    assert_true(strcmp(cache.type,"Data") == 0);
    assert_int_equal(cache.size, 8192);
    assert_int_equal(cache.sets, 64);
    assert_int_equal(cache.blocksize, 4096);
}


static void test_get_numCaches(void **state) {
    struct dirent *de = malloc(sizeof(struct dirent));
    de->d_type = DT_DIR;
    if (de == NULL) {
        fail_msg("de was null");
    }
    // have to do this 6 times because of . and .. dirs
    will_return_count(__wrap_readdir, de, 6);
    will_return(__wrap_readdir, NULL);
    assert_int_equal(get_numCaches(0),4);
}


int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_0_get_numCPUOnline),
        cmocka_unit_test(test_16_get_numCPUOnline),
        cmocka_unit_test(test_get_CacheParameters),
        cmocka_unit_test(test_get_numCaches),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
