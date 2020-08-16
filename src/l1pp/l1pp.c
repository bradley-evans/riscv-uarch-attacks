#include "l1pp.h"


/**
 * @brief      Creates an array a multiple size of the whole cache.
 * Writes to all this memory to "prime" the cache.
 * 
 * This function is based upon the flushcache() function provided
 * by Berkeley Architecture Research.
 *
 * @param[in]  cache  The cache parameters.
 *
 * @return     A pointer to an array that occupies a space some multiple
 * larger than the size of the cache.
 */
static uint8_t * l1pp_prime (struct cache_t cache) {

    // Create a large enough array that it will overwrite
    // the whole cache.
    uint8_t dummyMem[5 * cache.size]; //!< Creates an array of at least the size of the cache times some multiple.
    uint64_t alignedMem = ((uint64_t)&dummyMem + cache.size) & cache.mask_Tag;  //!< Memory space aligned on the first space in dummyMem with set and offset = 0 
    uint8_t dummyVar = 0; //!< Dummy var used to perform memory reads.

    /* note:
     * 64 / 8 = 8, therefore sizeof(uint64_t) = 8.
     * we have allocated 5*cache size to the pointer
     * but since each entry is 8 bytes, it has 
     * 5*cache.size / 8 elements.
     */
    char *msg = malloc(100);
    sprintf(msg, "Sizeof dummyMem: %d\n", sizeof(dummyMem));
    debug_msg(msg);


    sprintf(msg, "dummyMem is located at 0x%x. Aligned at 0x%x.\n", &dummyMem, alignedMem);
    debug_msg(msg);


    // we want to clear all sets in the cache
    // and replace them with our dummy array
    // to prime the cache.
    uint64_t addr = (uint64_t)&dummyMem;
    for (uint64_t i=0; i<cache.sets; ++i) {
        uint64_t setOffset = (((addr & cache.mask_Set) >> cache.numbits_Offset) + i) << cache.numbits_Offset;
        for (uint64_t j=0; j<4*cache.ways; ++j) {
            uint64_t wayOffset = j << (cache.numbits_Offset + cache.numbits_Set);
            dummyVar = *((uint8_t*)(alignedMem + setOffset + wayOffset));
        }
    }

    // pass back the address of our aligned memory location
    // so that we can probe it later.
    return alignedMem;
}


/**
 * @brief      Performs memory reads on a "primed" portion of memory
 * to determine which members of the primed memory were evicted.
 *
 * @param[in]  primed_memory  A pointer to the beginning of the primed memory.
 * @param[in]  sz             The size
 */
void l1pp_probe(uint8_t *primed_memory, struct cache_t cache) {
    printf("Probing beginning at aligned memory location 0x%x.\n", primed_memory);

    uint64_t start, end;

    uint8_t dummyVar = 0; //!< Dummy var used to perform memory reads.
    uint64_t addr = (uint64_t)primed_memory;
    for (uint64_t i=0; i<cache.sets; ++i) {
        uint64_t setOffset = (((addr & cache.mask_Set) >> cache.numbits_Offset) + i) << cache.numbits_Offset;
        for (uint64_t j=0; j<4*cache.ways; ++j) {
            uint64_t wayOffset = j << (cache.numbits_Offset + cache.numbits_Set);
            start = cycles();
            dummyVar = *((uint8_t*)((uint64_t)primed_memory + setOffset + wayOffset));
            end = cycles();
            printf("At 0x%x, memaccess time was %d.\n", ((uint64_t)primed_memory + setOffset + wayOffset), end-start);
        }
    }
}


/**
 * @brief      Victim function that will read some arbitrary
 * memory location
 */
void l1pp_victim () {


    uint8_t victim = 0;
    uint8_t dummy = 1;

    printf("The victim is active. It is reading 0x%x and 0x%x.\n", &victim, &dummy);
    for(int i=0; i<100; i++) {
        victim = victim + dummy;
        dummy = dummy + victim;
    }
}


/**
 * @brief      { function_description }
 *
 * @param[in]  cache  The cache
 */
void l1pp_demo(struct cache_t cache) {

    // step 1: prime cache
    printf("Priming cache...");
    uint8_t *primed_memory = l1pp_prime(cache);
    printf("A primed memory location has been returned at 0x%x.\n", primed_memory);

    // step 2: initiate victim thread
    printf("Spinning up victim.\n");
    l1pp_victim();

    // step 3: probe cache to see what was evicted
    printf("Victim process has run. Probing...\n");
    l1pp_probe(primed_memory, cache);
}