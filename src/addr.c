/**
 * @defgroup   ADDR address
 *
 * @file       addr.c
 * @brief      Implementation of functions that operate on memory addresses.
 *
 * @author     Bradley Evans
 * @date       June 2020
 */

#include "addr.h"


int calculateNumOffsetBits(struct cache_t cache) {
    return log2(cache.blocksize);
}


int calculateNumIndexBits(struct cache_t cache) {
    return log2(cache.sets);
}


int get_AddressSize(int *var) {
    return sizeof(var)*8;
}


unsigned int get_Offset(long long addr, int numOffsetBits) {
    long long mask = pow(2,numOffsetBits)-1;
    return mask & addr;
}


unsigned int get_Index(long long addr, int numOffsetBits, int numIndexBits) {
    long long mask = (int)(pow(2,numIndexBits)-1) << numOffsetBits;
    return (mask & addr) >> numOffsetBits;
}


/**
 * @brief      Generates a tag from an address.
 *
 * @param[in]  addr           The address
 * @param[in]  numOffsetBits  The number of offset bits
 * @param[in]  numIndexBits   The number of index bits
 *
 * @return     The tag portion of the address.
 */
unsigned int get_Tag(long long addr, int numOffsetBits, int numIndexBits) {
    long long mask = pow(2,numIndexBits+numOffsetBits)-1;
    mask = ~mask;
    return (mask & addr) >> (numOffsetBits + numIndexBits);
}


/**
 * @brief      Generates an address_t struct from cache information
 * and a variable passed by reference.
 *
 * @param[in]  cache  A cache_t struct.
 * @param      var    A variable, passed by reference, to generate an address_t from.
 *
 * @return     A struct describing the different fields of the address.
 */
struct address_t get_Address(struct cache_t cache, int *var) {
    struct address_t address;
    char *msg = malloc(100);

    sprintf(msg, "Recieved address for processing: 0x%llx", var);
    debug_msg(msg);


    /* this seems to consistently offset by 0x74 (116_10) bits. why? */
    address.addr = var;
    address.bitsize = get_AddressSize(var);

    int numOffsetBits = calculateNumOffsetBits(cache);
    int numIndexBits = calculateNumIndexBits(cache);
    int numTagBits = address.bitsize - numOffsetBits - numIndexBits;

    address.offset = get_Offset(address.addr, numOffsetBits);
    address.set = get_Index(address.addr, numOffsetBits, numIndexBits);
    address.tag = get_Tag(address.addr, numOffsetBits, numIndexBits);

    return address;
}


/**
 * @brief      Creates a candidate address in the same set as a "victim" address.
 * In theory, this address could evict the victim address by occupying the same
 * cache line, but this doesn't work. It will instead segmentation fault, more
 * often than not.
 *
 * @param[in]  cache   Cache parameters.
 * @param[in]  victim  Victim address we would like to evict.
 *
 * @return     A candidate "evictor" address.
 */
int * generate_Evictor(struct cache_t cache, struct address_t victim) {
    char *msg = malloc(100);

    sprintf(msg, "Address of victim: \t\t0x%llx", victim.addr);
    debug_msg(msg);

    int numOffsetBits = calculateNumOffsetBits(cache);
    int numIndexBits = calculateNumIndexBits(cache);
    unsigned long long int add_to_tag = 0x1 << (numOffsetBits + numIndexBits + 1);
    unsigned long long int evictor_addr = victim.addr + add_to_tag;
    sprintf(msg, "Candidate evictor address: \t0x%llx", evictor_addr);
    debug_msg(msg);

    const void * evictor = (void *)evictor_addr;

    sprintf(msg, "Evictor address: \t\t0x%llx", evictor);
    debug_msg(msg);


    return (int *)evictor;
}