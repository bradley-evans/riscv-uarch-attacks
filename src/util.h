/* 
 * Copyright 2020 Bradley Evans
 *
 * This program was developed to satisfy requirements
 * for the Masters Degree in Computer Engineering at
 * the University of California, Riverside. Refer to
 * the associated thesis for details.
 *
 * This program is based on Mastik by Yuval Yarom.
 */

/**
 * @file util.h
 * @author Bradley Evans
 * @date 16 June 2020
 * @brief Useful helper functions.
 */

#ifndef __UTIL_H__
#define __UTIL_H__


#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>
#include <time.h>


char* concat(char *s1, char *s2);
char* get_StringFromSysFile(char * filename);
int get_hartid();
char* int_to_binary_string(long long num, int numbits);
void delay(int ms);

#endif
