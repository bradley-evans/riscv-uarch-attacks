#include "low.h"




/**
 * @brief      Gets the number caches.
 *
 * @param[in]  hart_id  The hart identifier
 *
 * @return     The number caches.
 */
int get_numCaches(int hart_id) {
    DIR *dirp;
    struct dirent *dir;
    int count = 0;


    char target[50];
    sprintf(target, "/sys/devices/system/cpu/cpu%d/cache/", hart_id);

    dirp = opendir(target);
    
    if (dirp == NULL) {
        printf("null\n");   
        return count;
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
    FILE *fp;
    char buff[10];
    int numCPU = 0;

    char *ptr;

    fp = fopen("/sys/devices/system/cpu/online", "r");
    fscanf(fp, "%s", buff);
    if (strcmp(buff, "0") == 0) {
        return 1;
    }    
    ptr = strtok(buff, "-");
    while (ptr != NULL) {
        ptr = strtok(NULL, "-");
        if (ptr != NULL) { numCPU = atoi(ptr) + 1; }
    }
    fclose(fp);
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
    char *target;

    sprintf(buff, "/sys/devices/system/cpu/cpu%d/cache/index%d/",
            hart_id, cache_index);
    workingdir = calloc(strlen(buff), sizeof(char));
    strcpy(workingdir, buff);

    target = concat(workingdir, "ways_of_associativity");

    // printf("\t[get_CacheParameters()] cpu%d cache index%d\n", 
    //         hart_id, cache_index);

    FILE *fp;

    fp = fopen(concat(workingdir, "ways_of_associativity"), "r");
    if (fp == NULL) {
        printf("Could not access ways of associativity.\n");
        exit(1);
    }
    fscanf(fp, "%d", &cache.ways);
    // printf("\t\t| Ways:\t\t%d\n", cache.ways);

    fp = fopen(concat(workingdir, "level"), "r");
    if (fp == NULL) {
        printf("Could not access cache level information.\n");
        exit(1);
    }
    fscanf(fp, "%d", &cache.level);
    // printf("\t\t| Level:\t%d", cache.level);

    fp = fopen(concat(workingdir,"type"), "r");
    if (fp == NULL) {
        printf("Could not access cache type.\n");
        exit(1);
    }
    fscanf(fp, "%s", buff);
    cache.type = malloc(strlen(buff));
    strcpy(cache.type, buff);
    // printf(" (%s)\n", cache.type); 

    fp = fopen(concat(workingdir, "size"), "r");
    if (fp == NULL) {
        printf("Could not access cache size.\n");
        exit(1);
    }
    fscanf(fp, "%d%s", &cache.size, buff);
    if (strcmp(buff, "K") == 0) { 
        cache.size = 1024 * cache.size; 
        buff[0] = '\0';
    }
    // printf("\t\t| Size:\t\t%d\n", cache.size); 

    fp = fopen(concat(workingdir,"number_of_sets"), "r");
    if (fp == NULL) {
        printf("Could not access cache number of sets.\n");
        exit(1);
    }
    fscanf(fp, "%d", &cache.sets);
    // printf("\t\t| Sets:\t\t%d\n", cache.sets);

    fp = fopen("/sys/devices/system/memory/block_size_bytes", "r");
    if (fp == NULL) {
        cache.blocksize = 4096;
    } else {
        fscanf(fp, "%d", &cache.blocksize);
        // printf("\t\t| Block size:\t%d\n", cache.blocksize);
    }

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
 * @return     An array of CPUs and their parameters.
 */
struct cpu_t * initialize_cpu() {
    struct cpu_t *cpu = malloc(sizeof(struct cpu_t));
    for(int i=0; i<get_numCPUOnline(); i++) {
        cpu[i] = get_CPUParameters(i);
    }
    return cpu;
}