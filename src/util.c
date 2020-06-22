#include "util.h"


/**
 * @brief      Perform concatenation of strings
 *
 * @param      s1    The beginning string
 * @param      s2    The ending string
 *
 * @return     A string concatenation of s1, s2.
 */
char* concat(char *s1, char *s2) {
    char *result = malloc(strlen(s1) + strlen(s2) + 1);

    strcpy(result, s1);
    strcat(result, s2);
    return result;
}
