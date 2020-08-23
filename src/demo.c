/**
 * @defgroup   DEMO demo
 *
 * @file       demo.c
 * @brief      Implementation of a demonstration function that steps
 * through what the library can do.
 *
 * @author     Bradley Evans
 * @date       June 2020
 */

#include "demo.h"


extern char g_DEBUG = 0; /*!< Disables debug messages by default. Global, shared with debug_log module. */


/**
 * @brief      Demo routine.
 */
void main() {
    char buildtime[] = "20200815 1500";
    char *msg = malloc(100);
    
    printf("BUILD: %s\n", buildtime);


    printf("=================================\n");
    printf("RISC-V UARCH ATTACK DEMONSTRATION\n");
    printf("=================================\n\n");

    /*
     * Initialization Phase
     */

    printf("===================\n");
    printf("CPU Initialization.\n");    
    printf("===================\n");

    int numCPU = get_numCPUOnline();
    struct cpu_t *cpu = initialize_cpu();
    int currCPU = get_hartid();

    for(int i=0; i<numCPU; i++) {
        printf("%d:\tcpu%d:\thas %d caches\n", i, cpu[i].hart, cpu[i].numCaches);
        for (int j=0; j<cpu[i].numCaches; j++) {
            printf("\t\tcache%d: is a %d way L%d %s cache \n\
                    with size %d and and %d sets. The block\n\
                    size is %d. The line size is %d.\n",
                    j, cpu[i].cache[j].ways,
                    cpu[i].cache[j].level,
                    cpu[i].cache[j].type,
                    cpu[i].cache[j].size,
                    cpu[i].cache[j].sets,
                    cpu[i].cache[j].blocksize,
                    cpu[i].cache[j].linesize);
            printf("\t\t\tDerived parameters: there are %d offset bits,\n\
                    %d set bits, and %d tag bits. Address length\n\
                    is %d.\n",
                    cpu[i].cache[j].numbits_Offset,
                    cpu[i].cache[j].numbits_Set,
                    cpu[i].cache[j].numbits_Tag,
                    sizeof(void*)*8);            
        }
    }

    printf("\n***\tThis process is running on cpu%d\n", currCPU);

    /*
     * Address analysis and generation
     */

    struct cache_t tgt_cache = cpu[currCPU].cache[0];
    if (strcmp(tgt_cache.type, "Instruction") == 0) {
        tgt_cache = cpu[currCPU].cache[1];

    }

    printf("\n***\tThe cache loaded as target is a %s cache.\n\n", tgt_cache.type);

    printf("=========================================\n");
    printf("Cache address manipulation demonstration.\n");
    printf("=========================================\n\n");

    int victim = 0;


    sprintf(msg, "Address of victim: 0x%llx", &victim);
    debug_msg(msg);

    struct address_t addr_victim = get_Address(tgt_cache, &victim);

    printf("\tAddress of victim:\t%llx\n", &victim);
    printf("\tvictim Address Size:\t%d bytes, %d bits.\n\n", sizeof(&victim), sizeof(&victim)*8);
    printf("\t*******************************\n");
    printf("\t* address_t structure created *\n");
    printf("\t*******************************\n");
    

    printf("\tAddress:\t\t0x%llx\n", addr_victim.addr);
    printf("\tAddress size:\t\t%d-bit\n", addr_victim.bitsize);
    printf("\tAddress tag:\t\t0x%llx\n", addr_victim.tag);
    printf("\tAddress index:\t\t0x%llx\n", addr_victim.set);
    printf("\tAddress offset:\t\t0x%llx\n\n", addr_victim.offset);
    

    printf("===============================================\n");
    printf("L1 Cache Contention Generation and Measurement.\n");
    printf("===============================================\n\n");

    l1_contention_demo(&victim, tgt_cache);

    printf("====================================================\n");
    printf("Prime and Probe, L1 Data Cache (L1D$) Demonstration.\n");
    printf("====================================================\n\n");

    l1pp_demo(tgt_cache);

    printf("=======================\n");
    printf("DEMONSTRATION CONCLUDED\n");
    printf("=======================\n");
    printf("       ٩(͡๏̯͡๏)۶       \n");

    exit(0);

}