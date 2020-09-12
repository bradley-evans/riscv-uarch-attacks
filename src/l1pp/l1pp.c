/**
 * @defgroup   L1PP l1pp
 * 
 * L1 cache prime and probe attack implementations.
 *
 * @file       l1pp.c
 * @ingroup    L1PP l1pp
 * @brief      Function implementations that perform L1PP tasks, e.g. priming 
 * operations, probing operations.
 *
 * @author     Bradley Evans
 * @date       July 2020
 */
#include "l1pp.h"


/**
 * @brief      Creates an array a multiple size of the whole cache.
 * Writes to all this memory to "prime" the cache.
 * @ingroup    L1PP l1pp
 * 
 * This function is based upon the flushcache() function provided
 * by Berkeley Architecture Research.
 *
 * @param[in]  cache  The cache parameters.
 *
 * @return     A pointer to an array that occupies a space some multiple
 * larger than the size of the cache.
 */
uint8_t * l1pp_prime(struct cache_t cache) {

    // Create a large enough array that it will overwrite
    // the whole cache.
    // Current state: &dummyMem = [dummytag][dummyset][dummyoffset]
    uint8_t dummyMem[5 * cache.size]; //!< Creates an array of at least the size of the cache times some multiple.

    // Advance to the memory address &dummyMem + (the size of the cache).
    // Then `and` this address against the tag mask, which will generate a
    // memory address that has &dummyMem + cache.size's tag, but should zero
    // out the offset and set bits.
    // From here we can generate our "probe set" of addresses, neatly iterating
    // from set 0 to the highest set number in our cache.
    // Current state: alignedMem = [dummytag+x][000][000]
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
    // 
    // Our aligned memory location starts with a tag related to dummyMem[] and
    // zeroed out set, offset bits.  
    // Current state: alignedMem = [dummytag][dummyset][dummyoffset]
    uint64_t addr = (uint64_t)&dummyMem;
    for (uint64_t i=0; i<cache.sets; ++i) {
        // First, craft our set bits. Our set will be what we're looping through
        // in this loop. 
        uint64_t setOffset = (((addr & cache.mask_Set) >> cache.numbits_Offset) + i) << cache.numbits_Offset;
        for (uint64_t j=0; j<cache.ways; ++j) {
            // Each set is going to have a certain number of ways, too, that we
            // need to evict with our probe set. We'll advance our tag bits by
            // the value of the iterator to generate cache.ways different
            // addresses that would map to the same tag. 
            uint64_t wayOffset = j << (cache.numbits_Offset + cache.numbits_Set);
            // Then we add all these together to our aligned memory location to
            // get a valid memory address in our process' valid virtual address
            // space that corresponds to a set defined by i. we will generate
            // cache.ways*4 of these for every set to be very sure we have
            // primed the cache.
            // 
            // Then we perform a memory read into our dummyVar to get that
            // address into our cache.
            dummyVar = *((uint8_t*)(alignedMem + setOffset + wayOffset));
        }
    }

    // pass back the address of our aligned memory location
    // so that we can probe it later.
    return (uint8_t *)alignedMem;
}


/**
 * @brief      Performs memory reads on a "primed" portion of memory
 * to determine which members of the primed memory were evicted.
 * @ingroup    L1PP l1pp
 *
 * @param[in]  primed_memory  A pointer to the beginning of the primed memory.
 *
 * @return     An array of l1pp_result_t structs that show the memory address
 * probed and the resulting memory access time.
 * 
 */
struct l1pp_result_t * l1pp_probe(uint8_t *primed_memory, struct cache_t cache) {

    struct l1pp_result_t *results = malloc(sizeof(struct l1pp_result_t) * cache.sets * 4 * cache.ways);
    uint64_t start, end;
    uint8_t dummyVar = 0; //!< Dummy var used to perform memory reads.
    int index = 0;


    // In l1pp_prime() we created an aligned memory address that corresponds to
    // dummyMem[]. We saved that address, and now we need to go back and check
    // the memory access times of our probe set.
    uint64_t addr = (uint64_t)primed_memory;
    for (uint64_t i=0; i<cache.sets; i++) {
        // As before 
        uint64_t setOffset = (((addr & cache.mask_Set) >> cache.numbits_Offset) + i) << cache.numbits_Offset;
        for (uint64_t j=0; j<4*cache.ways; ++j) {
            uint64_t wayOffset = j << (cache.numbits_Offset + cache.numbits_Set);
            start = cycles();
            dummyVar = *((uint8_t*)((uint64_t)primed_memory + setOffset + wayOffset));
            end = cycles();
            results[index].addr = ((uint64_t)primed_memory + setOffset + wayOffset);
            results[index].time = end-start;
            index++;
        }
    }

    for (uint64_t i=0; i<(cache.sets*4*cache.ways); i++) {
        results[i].phys = virt_to_phys(results[i].addr, getpid());
    }

    return results;
}


/**
 * @brief      Victim function that will read some arbitrary
 * memory location
 * @ingroup    L1PP l1pp
 */
void * l1pp_victim () {

    uint8_t victim = 0;
    uint8_t dummy = 1;

    printf("The victim is active. It is reading 0x%x and 0x%x.\n", &victim, &dummy);
    for(int i=0; i<100; i++) {
        victim = victim + dummy;
        dummy = dummy + victim;
    }

}


/**
 * @brief      A brief demonstration function that shows the L1PP capabilities
 * of this library.
 * @ingroup    L1PP l1pp
 *
 * @param[in]  cache  The cache parameters.
 */
void l1pp_demo(struct cache_t cache) {

    pthread_t vic_thread;

    // step 1: prime cache
    printf("Priming cache...");
    uint8_t *primed_memory = l1pp_prime(cache);
    printf("A primed memory location has been returned at 0x%x.\n", primed_memory);

    // step 2: initiate victim thread
    printf("Spinning up victim.\n");
    // victim goes into a different thread
    pthread_create(&vic_thread, NULL, &l1pp_victim, NULL);
    pthread_join(vic_thread, NULL);

    // step 3: probe cache to see what was evicted
    printf("Victim process has run. Probing...\n");
    printf("Probing beginning at aligned memory location 0x%x.\n", primed_memory);

    struct l1pp_result_t *proberesults = l1pp_probe(primed_memory, cache);
    for (int i=0; i<(cache.sets*(4 * cache.ways)); i++) {
        printf("\tProbe of 0x%x\tphys: 0x%x\tmemaccess time: %d\n",
            proberesults[i].addr,
            proberesults[i].phys,
            proberesults[i].time
        );
    }
}