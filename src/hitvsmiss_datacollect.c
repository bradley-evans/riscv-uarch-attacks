#include "hitvsmiss_datacollect.h"


char g_DEBUG;
char DATAFILE[] = "hitvmiss_measurement.csv";


uint64_t hitvmiss_measurement(struct cache_t cache) {

    int8_t dummyMem[5 * cache.size];
    uint64_t alignedMem = ((uint64_t)&dummyMem + cache.size) & cache.mask_Tag;
    uint8_t dummyVar = 0;
    uint64_t start, end;
    uint64_t data_index = 0;

    FILE *f = fopen(DATAFILE, "w");
    struct l1pp_result_t *results = malloc(sizeof(struct l1pp_result_t) * cache.sets * cache.ways * 5 * 10);

    for(uint64_t n=0; n<10; n++) {
        flushcache(alignedMem, cache.size, cache);
        for (uint64_t k=0; k<5; k++) {
            for (uint64_t i=0; i<cache.sets; i++) {
                uint64_t setOffset = (((alignedMem & cache.mask_Set) >> cache.numbits_Offset) + i) << cache.numbits_Offset;
                for (uint64_t j=0; j<cache.ways; ++j) {
                    uint64_t wayOffset = j << (cache.numbits_Offset + cache.numbits_Set);
                    start = cycles();
                    dummyVar = *((uint8_t*)((uint64_t)alignedMem + setOffset + wayOffset));
                    end = cycles();

                    results[data_index].addr = ((uint64_t)alignedMem + setOffset + wayOffset);
                    results[data_index].time = end-start;
                    results[data_index].phys = 0;
                    results[data_index].run = k;
                    data_index++;
                }
            }
        }
        for (uint64_t i=0; i<data_index; i++) {
            fprintf(f, "0x%lx,0x%lx,%ld,%ld\n",
                results[i].addr,
                results[i].phys,
                results[i].time,
                results[i].run
                );
        }
    }
    fclose(f);
    return data_index;
}


int main() {

    struct cache_t cache = getL1DCache();
    uint64_t datasize = 0;

    // FILE *f = fopen(DATAFILE, 'w');  // Destroy the DATAFILE if it already exists. 
    // fclose(f);

    printf("\nChecking hit vs miss cache read times.\n");

    datasize = hitvmiss_measurement(cache);

    printf("\tCollected %d data points.\n", datasize);
    printf("\tCache sets: %ld. Cache ways: %ld. Expected datapoints: %ld\n", 
        cache.sets,
        cache.ways,
        cache.sets*cache.ways*5*10);

    printf("Done.\n");

    return 0;

}