/**
 * @defgroup   LOW low
 *
 * @file       low.c
 * @brief      This file implements low.
 *
 * @author     Bradley Evans
 * @date       June 2020
 */
#include "low.h"


/**
 * @brief      Gets the number of caches.
 *
 * @param[in]  hart_id  The hart identifier
 *
 * @return     The number of caches.
 */
int get_numCaches(int hart_id) {
    DIR *dirp;
    struct dirent *dir;
    int count = 0;

    char target[50];
    sprintf(target, "/sys/devices/system/cpu/cpu%d/cache/", hart_id);

    dirp = opendir(target);
    
    if (dirp == NULL) {
        return -1;
    }
    while((dir = readdir(dirp)) != NULL) {
        if(dir->d_type == DT_DIR) {
            count++;
        }
    }
    
    // -2, do not count "." and ".." directories
    return count-2;
}


/**
 * @brief      Determines the number of CPU cores in a system.
 *
 * @return     Number of CPUs online per Linux sysfs.
 */
int get_numCPUOnline() {
    char sysfile[] = "/sys/devices/system/cpu/online";
    char *online = get_StringFromSysFile(sysfile);
    int numCPU = 0;

    if (strcmp(online, "0") == 0) {
        return 1;
    }

    char *ptr;    
    ptr = strtok(online, "-");
    while (ptr != NULL) {
        ptr = strtok(NULL, "-");
        if (ptr != NULL) { numCPU = atoi(ptr) + 1; }
    }
    return numCPU;
}


/**
 * @brief      Gets the cache parameters.
 *
 * @param[in]  hart_id      The CPU or HART identifier (eg, 0 in CPU0)
 * @param[in]  cache_index  The cache index (eg, 0 in index0)
 *
 * @return     A cache_t object with cache parameters for the cpu/cache index given
 */
struct cache_t get_CacheParameters(int hart_id, int cache_index) {

    struct cache_t cache;
    char buff[200];
    char *workingdir;

    sprintf(buff, "/sys/devices/system/cpu/cpu%d/cache/index%d/",
            hart_id, cache_index);
    workingdir = calloc(strlen(buff), sizeof(char));
    strcpy(workingdir, buff);

    // get cache ways of associativity
    cache.ways = atoi(get_StringFromSysFile(concat(workingdir,"ways_of_associativity")));

    // get cache level (eg L1, L3, etc)
    cache.level = atoi(get_StringFromSysFile(concat(workingdir,"level")));

    // get cache type (eg data, instruction)
    cache.type = malloc(100); // TODO: allocate better
    strcpy(cache.type, get_StringFromSysFile(concat(workingdir,"type")));

    // get number of sets
    cache.sets = atoi(get_StringFromSysFile(concat(workingdir,"number_of_sets")));

    // get cache size as derivation from num sets, num ways
    cache.size = cache.sets * cache.ways * cache.blocksize;

    char *blocksize = get_StringFromSysFile("/sys/devices/system/memory/block_size_bytes");
    if (blocksize == NULL) {
        cache.blocksize = 4096;
    } else {
        cache.blocksize = atoi(blocksize);
    }

    // derived parameters
    uint64_t fullmask = 0xFFFFFFFFFFFFFFFF;
    cache.numbits_Offset =  log2(cache.blocksize);
    cache.numbits_Set =     log2(cache.sets);
    cache.numbits_Tag =     sizeof(void*);
    cache.mask_Offset =     (~(fullmask << cache.numbits_Offset));
    cache.mask_Tag =        (fullmask << (cache.numbits_Set + cache.numbits_Offset));
    cache.mask_Set =        (~(cache.mask_Tag | cache.mask_Offset));

    return cache;
}


/**
 * @brief      Gets the CPU parameters.
 *
 * @param[in]  hart_id  The CPU or HART identifier (eg, 0 in CPU0)
 *
 * @return     Parameters for the CPU core identified as a cpu_t object
 */
struct cpu_t get_CPUParameters(int hart_id) {
    struct cpu_t cpu;
    cpu.hart = hart_id;
    cpu.numCaches = get_numCaches(hart_id);
    // printf("[get_CPUParameters] Hart cpu%d has %d caches.\n", hart_id,
    //        cpu.numCaches);
    cpu.cache = calloc(cpu.numCaches, sizeof(struct cache_t));
    for(int cache_index=0; cache_index<cpu.numCaches; cache_index++) {
        cpu.cache[cache_index] = get_CacheParameters(hart_id, cache_index);       
    }
    return cpu;
}


/**
 * @brief      Creates a *cpu_t array of parameters for each core on a system.
 *
 * @return     An array of CPUs and their parameters. Returns NULL if error.
 */
struct cpu_t * initialize_cpu() {
    int numCPU = get_numCPUOnline();
    if (numCPU == -1) {
        return NULL;
    }
    struct cpu_t *cpu;
    cpu = malloc(numCPU * sizeof(struct cpu_t));
    for(int i=0; i<numCPU; i++) {
        cpu[i] = get_CPUParameters(i);
    }
    return cpu;
}


/**
 * @brief      Dummy function that raises a debug error when used.
 */
void notimplemented() {
    debug_msg("Not implemented.");
}