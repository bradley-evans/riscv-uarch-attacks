#include "test_util.h"


static void test_concat(void **state) {
    char *s1 = malloc(sizeof(char));
    strcpy(s1, "abcdefg");
    char *s2 = malloc(sizeof(char));
    strcpy(s2, "hijklmn");

    assert_true(strcmp(concat(s1,s2),"abcdefghijklmn") == 0);
}


static void test_get_StringFromSysFile(void **state) {
    char testfile[] = "testcase_get_StringFromSysFile";
    char teststring[] = "teststring";

    FILE *fp = fopen(testfile, "w");
    fputs(teststring, fp);
    fclose(fp);

    char *stringfromfile = get_StringFromSysFile(testfile);
    
    remove(testfile);

    assert_true(strcmp(teststring, stringfromfile) == 0);
}


static void test_int_to_binary_string(void **state) {
    int num = 15;    // 0xF, 0b1111
    assert_true(strcmp(int_to_binary_string(num, 4),"1111") == 0);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_concat),
        cmocka_unit_test(test_get_StringFromSysFile),
        cmocka_unit_test(test_int_to_binary_string),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
