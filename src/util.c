/**
 * @defgroup   UTIL utility
 *
 * @file       util.c
 * @brief      This file implements utility.
 *
 * @author     Bradley Evans
 * @date       June 2020
 */

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


/**
 * @brief      Gets the string from a system file.
 *
 * @param      filename Location of sysfile.
 *
 * @return     The string from the system file.
 */
char* get_StringFromSysFile(char filename[]){
    FILE *fp;
    char *buff = malloc(100); // TODO: allocate this better
    fp = fopen(filename, "r");
    if (fp == NULL) {
        return NULL;
    }
    fscanf(fp, "%s", buff);
    fclose(fp);
    // printf("about to return from sysstring reader, buff is %s\n", buff);
    return buff;
}

/**
 * @brief      Determines which CPU this process is running on.
 * Adapted from: 
 * https://hpcf.umbc.edu/general-productivity/checking-which-cpus-are-used-by-your-program/
 *
 * @return     The hartid.
 */
int get_hartid() {

    FILE *fp = fopen("/proc/self/stat", "r");
    long to_read = 8192;
    char buff[to_read];
    int read = fread(buff, sizeof(char), to_read, fp);
    fclose(fp);

    char* line = strtok(buff, " ");
    for (int i=1; i<38; i++) {
        line = strtok(NULL, " ");
    }
    line = strtok(NULL, " ");

    return atoi(line);
}


/**
 * @brief      Converts an integer to a binary cstring representation.
 *
 * @param[in]  num      The integer
 * @param[in]  numbits  The bitwidth of the resulting cstring.
 *
 * @return     A cstring binary representation of num.
 */
char* int_to_binary_string(long long num, int numbits) {
    char *binary_string = malloc((numbits+1) * sizeof(char));

    binary_string[numbits] = '\0';
    for (int i=numbits-1; i>=0; i--) {
        binary_string[i] = (num % 2) ? '1' : '0';
        num = num / 2;
    }
    return binary_string;
}

// /**
//  * @brief      Different implementation using itoa to convert numbers
//  * to binary strings.
//  * 
//  * I made this when I made the flushcache prototype, it seems to work
//  * better than the original so I incorporated it with the intention of
//  * replacing the original,
//  *
//  * @param[in]  num   The number
//  *
//  * @return     cstring of number in binary
//  */
// char * printbin(uint64_t num) {
//     char *buf = malloc(65);
//     itoa(num, buf, 2);
//     return buf;
// }


// /**
//  * @brief      Prints zeros to pad printbin() strings.
//  *
//  * @param[in]  num   The number of zeros.
//  *
//  * @return     A cstring of length num filled only with zeros.
//  */
// char * printzeros(uint64_t num) {
//     char *buf = malloc(num+1);
//     for (int i=0; i<num; i++) {
//         buf[i] = '0';
//     }
//     buf[num] = '\0';
//     //printf("num: %d, %s\n", num, buf);
//     return buf;
// }