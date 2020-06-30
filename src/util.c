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


char* int_to_binary_string(long long num, int numbits) {
    char *binary_string = malloc((numbits+1) * sizeof(char));

    binary_string[numbits] = '\0';
    for (int i=numbits-1; i>=0; i--) {
        binary_string[i] = (num % 2) ? '1' : '0';
        num = num / 2;
    }
    return binary_string;
}


void delay(int ms) {
    long pause;
    clock_t now, then;
    pause = ms*(CLOCKS_PER_SEC/1000);
    while( (now-then) < pause ) {
        now = clock();
    } 
}