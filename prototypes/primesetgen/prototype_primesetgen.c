#include "prototype_primesetgen.h"


uint8_t *dummyMem = NULL;


char* uint64_t_to_binary_string(uint64_t num) {
    char *binary_string = malloc(65);

    binary_string[64] = '\0';
    for (int i=63; i>=0; i--) {
        binary_string[i] = (num % 2) ? '1' : '0';
        num = num / 2;
    }
    return binary_string;
}


char * makecolumns(struct cache_t cache) {

    char *labels = malloc(cache.numbits_Offset+cache.numbits_Set+cache.numbits_Tag+1);

    char *offset = malloc(cache.numbits_Offset+1);
    char *spaces_offset = malloc(cache.numbits_Offset-5);
    for (int i=0; i<cache.numbits_Offset-5; i++) {
        spaces_offset[i] = '-';
    }
    snprintf(offset, cache.numbits_Offset+1, "[oft%s]",spaces_offset);

    char *set = malloc(cache.numbits_Set+1);
    char *spaces_set = malloc(cache.numbits_Set-5);
    for (int i=0; i<cache.numbits_Set-5; i++) {
        spaces_set[i] = '-';
    }
    snprintf(set, cache.numbits_Set+1, "[set%s]",spaces_set);

    char *tag = malloc(cache.numbits_Tag+1);
    char *spaces_tag = malloc(cache.numbits_Tag-5);
    for (int i=0; i<cache.numbits_Tag-5; i++) {
        spaces_tag[i] = '-';
    }
    snprintf(tag, cache.numbits_Tag+1, "[tag%s]",spaces_tag);

    snprintf(labels, cache.numbits_Offset+cache.numbits_Set+cache.numbits_Tag+1, "%s%s%s", tag, set, offset);

    return labels;
}


uint8_t * generatePrimeset(struct cache_t cache) {


    printf("Prime phase.\n");

    // Create a large enough array that it will overwrite
    // the whole cache.
    // Current state: &dummyMem = [dummytag][dummyset][dummyoffset]
    dummyMem = malloc(5 * cache.size); //!< Creates an array of at least the size of the cache times some multiple.

    // Advance to the memory address &dummyMem + (the size of the cache).
    // Then `and` this address against the tag mask, which will generate a
    // memory address that has &dummyMem + cache.size's tag, but should zero
    // out the offset and set bits.
    // From here we can generate our "probe set" of addresses, neatly iterating
    // from set 0 to the highest set number in our cache.
    // Current state: alignedMem = [dummytag+x][000][000]
    uint64_t alignedMem = ((uint64_t)dummyMem + cache.size) & cache.mask_Tag;  //!< Memory space aligned on the first space in dummyMem with set and offset = 0 
    uint8_t dummyVar = 0; //!< Dummy var used to perform memory reads.

    // we want to clear all sets in the cache
    // and replace them with our dummy array
    // to prime the cache.
    // 
    // Our aligned memory location starts with a tag related to dummyMem[] and
    // zeroed out set, offset bits.
    uint64_t *addr;
    uint64_t probeset[cache.ways];
    for (uint64_t i=0; i<cache.sets; ++i) {
        // First, craft our set bits. Our set will be what we're looping through
        // in this loop. 
        uint64_t setOffset = i << cache.numbits_Offset;

        printf("\t\t\t%s\n", makecolumns(cache));
        printf("setOffset (i=%ld):\t%s\t(0x%x)\n", i, uint64_t_to_binary_string(setOffset), setOffset);
        for (uint64_t j=0; j<cache.ways; ++j) {  
            // Each set is going to have a certain number of ways, too, that we
            // need to evict with our probe set. We'll advance our tag bits by
            // the value of the iterator to generate 4*cache.ways different
            // addresses that would map to the same tag. 
            uint64_t wayOffset = j << (cache.numbits_Offset + cache.numbits_Set);
            printf(" wayOffset(j=%ld):\t%s\t(0x%x)\n", j, uint64_t_to_binary_string(wayOffset), wayOffset);
            // Then we add all these together to our aligned memory location to
            // get a valid memory address in our process' valid virtual address
            // space that corresponds to a set defined by i. we will generate
            // cache.ways*4 of these for every set to be very sure we have
            // primed the cache.
            // 
            // Then we perform a memory read into our dummyVar to get that
            // address into our cache.
            addr = (alignedMem + setOffset + wayOffset);
            *addr = 0xdeadbeef;
            printf(" final address:\t\t%s\t(0x%x)\n",uint64_t_to_binary_string(addr),addr);
            probeset[j] = addr;
            dummyVar = *((uint8_t*)(alignedMem + setOffset + wayOffset));
        }
        printf("-reading generated memory addresses: ");
        for (uint64_t j=0; j<cache.ways; j++) {
            printf("[%ld] 0x%x ", j, *((uint8_t*)probeset[j]));
        }
        printf("\n");
    }

    // pass back the address of our aligned memory location
    // so that we can probe it later.
    return (uint8_t *)alignedMem;
}


void probePrimeset(struct cache_t cache, uint8_t *alignedMem) {

    printf("Probe phase.\n");
    uint8_t dummyVar = 0; //!< Dummy var used to perform memory reads.

    uint64_t *addr;
    uint64_t probeset[cache.ways];
    for (uint64_t i=0; i<cache.sets; ++i) {
        // First, craft our set bits. Our set will be what we're looping through
        // in this loop. 
        uint64_t setOffset = i << cache.numbits_Offset;

        printf("\t\t\t%s\n", makecolumns(cache));
        printf("setOffset (i=%ld):\t%s\t(0x%x)\n", i, uint64_t_to_binary_string(setOffset), setOffset);
        for (uint64_t j=0; j<cache.ways; ++j) {  
            // Each set is going to have a certain number of ways, too, that we
            // need to evict with our probe set. We'll advance our tag bits by
            // the value of the iterator to generate 4*cache.ways different
            // addresses that would map to the same tag. 
            uint64_t wayOffset = j << (cache.numbits_Offset + cache.numbits_Set);
            printf(" wayOffset(j=%ld):\t%s\t(0x%x)\n", j, uint64_t_to_binary_string(wayOffset), wayOffset);
            // Then we add all these together to our aligned memory location to
            // get a valid memory address in our process' valid virtual address
            // space that corresponds to a set defined by i. we will generate
            // cache.ways*4 of these for every set to be very sure we have
            // primed the cache.
            // 
            // Then we perform a memory read into our dummyVar to get that
            // address into our cache.
            addr = (alignedMem + setOffset + wayOffset);
            printf(" final address:\t\t%s\t(0x%x)\n",uint64_t_to_binary_string(addr),addr);
            probeset[j] = addr;
            dummyVar = *((uint8_t*)(alignedMem + setOffset + wayOffset));
        }
        printf("-reading probed memory addresses: ");
        for (uint64_t j=0; j<cache.ways; j++) {
            printf("[%ld] 0x%x ", j, *((uint32_t*)probeset[j]));
        }
        printf("\n");
    }
}


struct cache_t initializeCache() {
    struct cache_t cache;
    cache.ways = 4;
    cache.level = 1;
    cache.size = 16384;
    cache.sets = 64;
    cache.linesize = 64;
    cache.blocksize = 4096;
    cache.type = malloc(100);
    strcpy(cache.type,"Data");

    uint64_t fullmask = 0xFFFFFFFFFFFFFFFF;
    cache.numbits_Offset =  log2(cache.linesize);
    cache.numbits_Set =     log2(cache.sets);
    cache.numbits_Tag =     sizeof(void*)*8 - cache.numbits_Set - cache.numbits_Offset;
    cache.mask_Offset =     (~(fullmask << cache.numbits_Offset));
    cache.mask_Tag =        (fullmask << (cache.numbits_Set + cache.numbits_Offset));
    cache.mask_Set =        (~(cache.mask_Tag | cache.mask_Offset));

    return cache;
}


int main() {

    printf("Running cache priming set prototype.\n");

    struct cache_t cache = initializeCache();

    uint8_t *alignedMem = generatePrimeset(cache);

    probePrimeset(cache, alignedMem);

    return 0;

}