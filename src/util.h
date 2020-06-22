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

#ifndef __LOW_H__
#define __LOW_H__


#include <math.h>
#include <string.h>
#include <stdlib.h>


#include "low.h"

struct address_t {
    unsigned int offset;
    unsigned int set;
    unsigned int tag;
    unsigned int addr;
};

char* concat(char *s1, char *s2);


#endif
