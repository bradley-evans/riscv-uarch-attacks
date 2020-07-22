#ifndef __PROTOTYPE_FLUSHCACHE_H__
#define __PROTOTYPE_FLUSHCACHE_H__


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>


/**
 * @brief      Replicates the cache_t struct in low for testing.
 */
struct cache_t {
    int ways; /*!< Number of cache ways. */
    int level; /*!< Level of this cache, eg L1. */
    int size; /*!< Size of cache, bytes. */
    int sets; /*!< Number of sets in this cache. */
    int blocksize; /*!< The system block size, bytes. */
    char * type; /*!< String for instruction, data cache, etc. */
    // derived properties
    int numbits_Offset; /*!< Number of offset bits for an address in this system. */
    int numbits_Set; /*!< Number of bits used for indexing. */
    int numbits_Tag; /*!< Number of bits used for tag. */
    uint64_t mask_Offset; /*!< Masks to derive offset. */
    uint64_t mask_Set; /*!< Masks to derive set. */
    uint64_t mask_Tag; /*!< Masks to derive tag. */
};

char * printzeros(uint64_t num);
char * printbin(uint64_t num);
struct cache_t initialize_cache();
static void _riscv_flushCache_pt(uint64_t addr, uint64_t sz, struct cache_t cache);


#define flushcache_pt(addr, sz, cache) _riscv_flushCache_pt(addr, sz, cache)


#endif