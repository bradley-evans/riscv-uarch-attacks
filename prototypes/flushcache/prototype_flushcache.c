#include "prototype_flushcache.h"


char * printbin(uint64_t num) {
    char *buf = malloc(65);
    itoa(num, buf, 2);
    return buf;
}

char * printzeros(uint64_t num) {
    char *buf = malloc(num+1);
    for (int i=0; i<num; i++) {
        buf[i] = '0';
    }
    buf[num] = '\0';
    //printf("num: %d, %s\n", num, buf);
    return buf;
}


/**
 * @brief      Helper function that creates a sample cache_t object for us to use here.
 *
 * @return     Example cache parameters.
 */
struct cache_t initialize_cache() {
    struct cache_t cache;

    cache.ways = 4;
    cache.level = 1;
    cache.sets = 64;
    // sprintf(cache.type, "Data");
    cache.blocksize = 4096;

    // derived parameters
    int numBlocks = cache.sets * cache.ways;
    cache.size = numBlocks * cache.blocksize;


    // derivation of other parameters
    // from low
    uint64_t fullmask = 0xFFFFFFFFFFFFFFFF;
    cache.numbits_Offset =  log2(cache.blocksize);
    cache.numbits_Set =     log2(cache.sets);
    cache.numbits_Tag =     (sizeof(void*)*4) - (cache.numbits_Set + cache.numbits_Offset);
    cache.mask_Offset =     (~(fullmask << cache.numbits_Offset));
    cache.mask_Tag =        (fullmask << (cache.numbits_Set + cache.numbits_Offset));
    cache.mask_Set =        (~(cache.mask_Tag | cache.mask_Offset));

    /*
        Result:     | 32 <--                 --> 0 |
        Tag mask:   11111111111111000000000000000000
        Set mask:                 111111000000000000
        Offset mask:                    111111111111
     */

    printf("-- Cache is set up.\n");
    printf("\tSize: %d\n\tWays: %d\n\tSets: %d\n\tBlocksize: %d\n",
                cache.size, cache.ways, cache.sets, cache.blocksize);
    printf("\tTag mask: \t%s, sz %d\n",
                    printbin(cache.mask_Tag),
                    cache.numbits_Tag);
    printf("\tSet mask: \t%s%s, sz %d\n", 
                    printzeros(cache.numbits_Tag), 
                    printbin(cache.mask_Set),
                    cache.numbits_Set);
    printf("\tOffset mask: \t%s%s, sz %d\n", 
                    printzeros(cache.numbits_Tag+cache.numbits_Set),
                    printbin(cache.mask_Offset),
                    cache.numbits_Offset);
    return cache;
}


/**
 * @brief      A prototype / experimental flushcache routine.
 * 
 * Meant for testing, actual implmenentation would be in src/low.
 *
 * @param[in]  addr   The address to be evicted
 * @param[in]  sz     The size of the data held at address
 * @param[in]  cache  The cache parameters.
 */
static void _riscv_flushCache_pt(uint64_t addr, uint64_t sz, struct cache_t cache) {
    printf("\taddr: 0x%llx, sz: %d bytes\n", addr, sz);

    // a uint8_t has size 1 (bytes)
    uint8_t dummyMem[5 * cache.size]; /*!< Creates an array at least the size of the cache. */

    printf("\tDummy memory created with size %d bytes.\n", sizeof(dummyMem));
    printf("\t\tThe start and end addresses of dummyMem are:\n\t\t\tStart:\t0x%x\n\t\t\tEnd:\t0x%x\n",
                    &dummyMem[0],
                    &dummyMem[5*cache.size-1]);


    // Determine the number of blocks we need to clear.
    uint64_t numSetsClear = sz >> cache.numbits_Offset;
    if ( (sz & cache.mask_Offset) != 0 ) {
        numSetsClear += 1;
    }
    if (numSetsClear > cache.sets) {
        // Flush the entire cache, no rollever.
        numSetsClear = cache.sets;
    }
    printf("\tnumSetsClear:\t%d\t\t(number of blocks to clear)\n", numSetsClear);

    uint8_t dummyVar = 0; //!< Unused dummy variable 

    uint64_t alignedMem = ((uint64_t)&dummyMem + cache.size) & cache.mask_Tag;
    printf("\talginedMem:\t0x%x\t(start of contiguous mem space that fits in cache)\n", alignedMem);

    printf("\t\tdummyMem binary:\t%s\n",printbin(dummyMem));
    printf("\t\talignedMem binary:\t%s\n", printbin(alignedMem));
    uint64_t alignedMem_idx = alignedMem & cache.mask_Set;
    printf("\t\talignedMem index:\t%s\n", printbin(alignedMem_idx));
    uint64_t alignedMem_offset = alignedMem & cache.mask_Offset;
    printf("\t\talignedMem offset:\t%s\n", printbin(alignedMem_offset));
    printf("\tFlush initialization done. Begin actual flush.\n");
    for (uint64_t i=0; i<numSetsClear; ++i) {
        uint64_t setOffset = (((addr & cache.mask_Set) >> cache.numbits_Offset) + i) << cache.numbits_Offset;
        printf("\t\tsetOffset: 0x%x\n", setOffset);
        for (uint64_t j=0; j<4*cache.ways; ++j) {
            uint64_t wayOffset = j << (cache.numbits_Offset + cache.numbits_Set);
            printf("\t\t\twayOffset: 0x%x, ", wayOffset);
            dummyVar = *((uint8_t*)(alignedMem + setOffset + wayOffset));
            printf("\t..reading 0x%x\n", alignedMem + setOffset + wayOffset);
        }
    }
    printf("\tFlush done.\n");
}


/**
 * @brief      Main routine that tests the flushcache prototype.
 */
void main() {
    printf("-- Testing RISCV flushcache prototype.\n");

    struct cache_t cache = initialize_cache();

    int *victim = malloc(sizeof(int));
    printf("-- Victim address generated at 0x%x, size %d.\n", victim, sizeof(victim));
    printf("-- Flushing the cache.\n");
    flushcache_pt((uint64_t)victim, sizeof(victim), cache);

}
