#ifndef __ADDR_H__
#define __ADDR_H__


#include <math.h>
#include <stdio.h>


#include "low.h"
#include "debug_log.h"


struct address_t {
    unsigned int bitsize;
    unsigned int offset;
    unsigned int set;
    unsigned int tag;
    unsigned long long int addr;
};


int calculateNumOffsetBits(struct cache_t cache);
int calculateNumIndexBits(struct cache_t cache);
int get_AddressSize(int *var);
unsigned int get_Offset(long long addr, int numOffsetBits);
unsigned int get_Index(long long addr, int numOffsetBits, int numIndexBits);
unsigned int get_Tag(long long addr, int numOffsetBits, int numIndexBits);
struct address_t get_Address(struct cache_t cache, int *var);
int * generate_Evictor(struct cache_t cache, struct address_t victim);


#endif