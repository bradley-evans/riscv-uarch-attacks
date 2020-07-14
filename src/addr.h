/**
 * @defgroup   ADDR address
 *
 * @file       addr.h
 * @brief      Functions that operate on memory addresses.
 *
 * @author     Bradley Evans
 * @date       June 2020
 */
#ifndef __ADDR_H__
#define __ADDR_H__


#include <math.h>
#include <stdio.h>


#include "low.h"
#include "debug_log.h"


/**
 * @brief      Stores information about a particular memory address.
 */
struct address_t {
    unsigned int bitsize; /*!< Bitwidth of the memory address, e.g. 64bit */
    uint64_t offset; /*!< Offset portion of the address. */
    uint64_t set; /*!< Set or index portion of the address. */
    uint64_t tag; /*!< Tag portion of the address. */
    uint64_t addr; /*!< The actual address. */
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