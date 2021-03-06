/**
 * @defgroup   UTIL utility
 *
 * @file       util.h
 * @brief      Implements basic utility functions.
 *
 * @author     Bradley Evans
 * @date       June 2020
 */

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


#ifndef __UTIL_H__
#define __UTIL_H__


#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>


char* concat(char *s1, char *s2);
char* get_StringFromSysFile(char * filename);
int get_hartid();
char* int_to_binary_string(long long num, int numbits);
// char* printbin(uint64_t num);
// char* printzeros(uint64_t num);


#endif
