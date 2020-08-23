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

    DIR* d = opendir(target);
    if(d==NULL) {
        printf("Number of caches could not be determined from device tree. Are you in an emulator?\n");
        printf("Generating a default number of caches.\n");
        return 2;
    }

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
    char *buff_ptr;
    char *workingdir;

    sprintf(buff, "/sys/devices/system/cpu/cpu%d/cache/index%d/",
            hart_id, cache_index);
    workingdir = calloc(strlen(buff), sizeof(char));
    strcpy(workingdir, buff);

    DIR* dir = opendir(workingdir);
    if(dir==NULL) {
        if (cache_index==0) {
            // generate a emulator default instr cache
            printf("Generating a default instruction cache object.\n");
            cache.ways=4;
            cache.level=1;
            cache.type=malloc(100);
            strcpy(cache.type,"Instruction");
            cache.sets=64;
            cache.size=32768;
            cache.linesize = cache.size / (cache.sets * cache.ways);
            cache.blocksize=4096;
        } else if (cache_index==1) {
            // generate a emulator default data cache
            printf("Generating a default data cache object.\n");
            cache.ways=4;
            cache.level=1;
            cache.type=malloc(100);
            strcpy(cache.type,"Data");
            cache.sets=64;
            cache.size=32768;
            cache.linesize = cache.size / (cache.sets * cache.ways);
            cache.blocksize=4096;
        }
        // derived parameters
        uint64_t fullmask = 0xFFFFFFFFFFFFFFFF;
        cache.numbits_Offset =  log2(cache.linesize);
        cache.numbits_Set =     log2(cache.sets);
        cache.numbits_Tag =     sizeof(void*)*8 - cache.numbits_Set - cache.numbits_Offset;
        cache.mask_Offset =     (~(fullmask << cache.numbits_Offset));
        cache.mask_Tag =        (fullmask << (cache.numbits_Set + cache.numbits_Offset));
        cache.mask_Set =        (~(cache.mask_Tag | cache.mask_Offset));
        return cache;
    }

    // get cache ways of associativity
    cache.ways = atoi(get_StringFromSysFile(concat(workingdir,"ways_of_associativity")));

    // get cache level (eg L1, L3, etc)
    cache.level = atoi(get_StringFromSysFile(concat(workingdir,"level")));

    // get cache type (eg data, instruction)
    cache.type = malloc(100); // TODO: allocate better
    strcpy(cache.type, get_StringFromSysFile(concat(workingdir,"type")));

    // get number of sets
    cache.sets = atoi(get_StringFromSysFile(concat(workingdir,"number_of_sets")));

    // get cache size. Usually this will return as some string like "32K".
    // Remember, here K is base2 so 32K means 32*1024 = 
    buff_ptr = get_StringFromSysFile(concat(workingdir,"size"));
    cache.size = atoi(buff_ptr);
    if (strchr(buff_ptr,'K') != NULL) {
        cache.size = cache.size * 1024;
    }

    // derive line size
    cache.linesize = cache.size / ( cache.sets * cache.ways );


    char *blocksize = get_StringFromSysFile("/sys/devices/system/memory/block_size_bytes");
    if (blocksize == NULL) {
        cache.blocksize = 4096;
    } else {
        cache.blocksize = atoi(blocksize);
    }

    // derived parameters
    uint64_t fullmask = 0xFFFFFFFFFFFFFFFF;
    cache.numbits_Offset =  log2(cache.linesize);
    cache.numbits_Set =     log2(cache.sets);
    cache.numbits_Tag =     sizeof(void*) * 8 - cache.numbits_Set - cache.numbits_Offset;
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
 * @brief      Gets the valid L1 data cache parameters for the current CPU
 * that this process is running on.
 *
 * @return     The L1 Data Cache parameters for the current CPU core.
 */
struct cache_t getL1DCache() {
    int numCPU = get_numCPUOnline();
    struct cpu_t *cpu = initialize_cpu();
    int currCPU = get_hartid();
    struct cache_t cache = cpu[currCPU].cache[0];
    if (strcmp(cache.type, "Instruction") == 0) {
        cache = cpu[currCPU].cache[1];
    }
    return cache;
}


/**
 * @brief      Dummy function that raises a debug error when used.
 */
void notimplemented() {
    debug_msg("Not implemented.");
}