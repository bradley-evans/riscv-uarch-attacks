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


int get_Offset(long long addr, int numOffsetBits) {
    long long mask = pow(2,numOffsetBits)-1;

    // printf("\n");
    // printf("address (bin): %s\n", int_to_binary_string(addr,64));
    // printf("mask(bin):     %s\n", int_to_binary_string(mask,64));
    // printf("offset (bin):  %s\n", int_to_binary_string(mask & addr,64));

    return mask & addr;
}


int get_Index(long long addr, int numOffsetBits, int numIndexBits) {
    long long mask = (int)(pow(2,numIndexBits)-1) << numOffsetBits;

    // printf("\n");
    // printf("address (bin): %s\n", int_to_binary_string(addr,64));
    // printf("mask(bin):     %s\n", int_to_binary_string(mask,64));
    // printf("index (bin):   %s\n", int_to_binary_string(mask & addr,64));
    // printf("adjusted:      %s\n", int_to_binary_string((mask & addr) >> numOffsetBits, 64));

    return (mask & addr) >> numOffsetBits;
}


int get_Tag(long long addr, int numOffsetBits, int numIndexBits, int numTagBits) {
    long long mask = pow(2,numIndexBits+numOffsetBits)-1;
    mask = ~mask;

    // printf("\n");
    // printf("tag bits: %d, offset bits: %d, index bits: %d\n", numTagBits, numOffsetBits, numIndexBits);
    // printf("address (bin): %s\n", int_to_binary_string(addr,64));
    // printf("mask(bin):     %s\n", int_to_binary_string(mask,64));
    // printf("tag (bin):     %s\n", int_to_binary_string(mask & addr,64));
    // printf("adjusted:      %s\n", int_to_binary_string((mask & addr) >> (numOffsetBits+numIndexBits), 64));

    return (mask & addr) >> (numOffsetBits + numIndexBits);
}

struct address_t get_Address(struct cache_t cache, int *var) {
    struct address_t address;

    /* this seems to consistently offset by 0x74 (116_10) bits. why? */
    address.addr = &var + 14;
    address.bitsize = get_AddressSize(var);

    int numOffsetBits = calculateNumOffsetBits(cache);
    int numIndexBits = calculateNumIndexBits(cache);
    int numTagBits = address.bitsize - numOffsetBits - numIndexBits;

    address.offset = get_Offset(address.addr, numOffsetBits);
    address.set = get_Index(address.addr, numOffsetBits, numIndexBits);
    address.tag = get_Tag(address.addr, numOffsetBits, numIndexBits, numTagBits);

    return address;
}


int* generate_Evictor(int victim) {
    int *evictor = &victim;

    return evictor;
}