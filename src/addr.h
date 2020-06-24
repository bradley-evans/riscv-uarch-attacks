
#ifndef __ADDR_H__
#define __ADDR_H__


#include <math.h>
#include <stdio.h>


#include "low.h"


struct address_t {
    unsigned int bitsize;
    unsigned int offset;
    unsigned int set;
    unsigned int tag;
    unsigned long long int addr;
};


struct address_t get_Address(struct cache_t cache, int *var);


#endif