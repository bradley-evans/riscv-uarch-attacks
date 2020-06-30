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


int get_Tag(long long addr, int numOffsetBits, int numIndexBits) {
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
    char *msg = malloc(100);

    sprintf(msg, "Address of var: 0x%llx", &var);
    debug_msg(msg);


    /* this seems to consistently offset by 0x74 (116_10) bits. why? */
    address.addr = &var;
    address.bitsize = get_AddressSize(&var);

    int numOffsetBits = calculateNumOffsetBits(cache);
    int numIndexBits = calculateNumIndexBits(cache);
    int numTagBits = address.bitsize - numOffsetBits - numIndexBits;

    address.offset = get_Offset(address.addr, numOffsetBits);
    address.set = get_Index(address.addr, numOffsetBits, numIndexBits);
    address.tag = get_Tag(address.addr, numOffsetBits, numIndexBits);

    return address;
}


struct address_t generate_Evictor(struct cache_t cache, struct address_t victim) {
    struct address_t evictor = victim;
    char *msg = malloc(100);

    sprintf(msg, "Victim address: 0x%llx", victim.addr);
    debug_msg(msg);
    sprintf(msg, "Victim tag: 0x%llx", victim.tag);
    debug_msg(msg);
    sprintf(msg, "Victim index / set: 0x%llx", victim.set);
    debug_msg(msg);

    evictor.tag = evictor.tag + 1; // this is just kind of aritrary
    unsigned long long tag = evictor.tag;
    evictor.offset = 0;

    sprintf(msg, "Evictor tag: 0x%llx", evictor.tag);
    debug_msg(msg);

    int numOffsetBits = calculateNumOffsetBits(cache);
    int numIndexBits = calculateNumIndexBits(cache);

    sprintf(msg, "Evictor address: 0x%llx", evictor.addr);
    debug_msg(msg);
    evictor.addr = evictor.addr >> numOffsetBits; // zero out the offset
    evictor.addr = evictor.addr << numOffsetBits;
    sprintf(msg, "Evictor address: 0x%llx", evictor.addr);
    debug_msg(msg);
    evictor.addr = evictor.addr << (numIndexBits + numOffsetBits); // zero out tag
    sprintf(msg, "Evictor address: 0x%llx", evictor.addr);
    debug_msg(msg);
    evictor.addr = evictor.addr >> (numIndexBits + numOffsetBits);
    sprintf(msg, "Evictor address: 0x%llx", evictor.addr);
    debug_msg(msg);
    sprintf(msg, "Evictor tag with bit offset: 0x%llx", (tag << (numOffsetBits + numIndexBits)));
    debug_msg(msg);
    evictor.addr = evictor.addr | (tag << (numOffsetBits + numIndexBits));

    sprintf(msg, "Evictor address: 0x%llx", evictor.addr);
    debug_msg(msg);

    return evictor;
}