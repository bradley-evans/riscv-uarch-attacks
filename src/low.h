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
 * @file low.h
 * @author Bradley Evans
 * @date 27 May 2020
 * @brief Low level common functions for RISC-V side-channel attacks.
 */

#ifndef __UTIL_H__
#define __UTIL_H__


#include <stdint.h>
#include <stdio.h>
#include <dirent.h>


#include "util.h"

/**
 * This helper function will perform a memory access. Returns an int of what 
 * was found at that address. Used to force memory accesses to specific 
 * locations. This method was suggested by Stef O'Rear. It would be good to 
 * examine if compiler optimizations actually affect this. 
 * Usage: loadword(target_addr)
 * @param address A memory address. Long for 32 bit systems.
 */
static inline int loadword(uint32_t address) {
    return *(volatile int *)address;
}


/**
 * This helper function will perform a memory write to a specific address.
 * Returns nothing. Usage storeword(target_addr).
 * @param address A memory address. Long for 32 bit systems.
 */
static inline void storeword(uint32_t address, int value) {
    *(volatile int *)address = value;
}


/**
 * Uses a CSR instruction to serialize the instruction stream. A serialization
 * instruction does not actually exist in RISC-V, however, the documentation
 * for the Berkeley Out-of-Order Machine (BOOM) indicates that Control Status
 * Register instructions may have a serializing effect. We use rdinstret to
 * leverage this property. This function will return the retired instruction
 * count on the processor. 
 */
static inline uint32_t rdinstret() {
    uint32_t rv;
    asm volatile("rdinstret x10"
            : "=r" (rv)
            : // no inputs
            : "x10"
            );
    return rv;
}


/**
 * Defines CPU cache parameters.
 */
struct cache_t {
    int ways;
    int level;
    int size;
    int sets;
    int blocksize;
    char * type;
};


/**
 * Defines CPU parameters.
 */
struct cpu_t {
    int hart;
    int numCaches;
    struct cache_t * cache;
};


/*
 * Function prototypes for device tree reader
 */

int get_numCaches(int hart_id);
int get_numCPUOnline();
struct cache_t get_CacheParameters(int hart_id, int cache_index);
struct cpu_t get_CPUParameters(int hart_id);
struct cpu_t* initialize_cpu();


#endif
