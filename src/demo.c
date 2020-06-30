#include "demo.h"


extern char g_DEBUG = 0;


/**
 * @brief      Demo routine.
 */
void main() {
    char buildtime[] = "20200624 2212";
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
                    size is %d.\n",
                    j, cpu[i].cache[j].ways,
                    cpu[i].cache[j].level,
                    cpu[i].cache[j].type,
                    cpu[i].cache[j].size,
                    cpu[i].cache[j].sets,
                    cpu[i].cache[j].blocksize);                
        }
    }
    printf("\n***\tThis process is running on cpu%d\n\n", currCPU);

    /*
     * Address analysis and generation
     */

    printf("=========================================\n");
    printf("Cache address manipulation demonstration.\n");
    printf("=========================================\n\n");

    int victim = 0;


    sprintf(msg, "Address of victim: 0x%llx", &victim);
    debug_msg(msg);

    struct address_t addr_victim = get_Address(cpu[currCPU].cache[0], &victim);

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

    printf("\t********************************\n");
    printf("\t* generate an evicting address *\n");
    printf("\t********************************\n");

    int *evictor = generate_Evictor(cpu[currCPU].cache[0], addr_victim);
    sprintf(msg, "Evictor: 0x%llx", evictor);
    debug_msg(msg);

    struct address_t addr_evictor = get_Address(cpu[currCPU].cache[0], evictor);
    
    printf("\tAddress:\t\t0x%llx\n", addr_evictor.addr);
    printf("\tAddress size:\t\t%d-bit\n", addr_evictor.bitsize);
    printf("\tAddress tag:\t\t0x%llx\n", addr_evictor.tag);
    printf("\tAddress index:\t\t0x%llx\n", addr_evictor.set);
    printf("\tAddress offset:\t\t0x%llx\n\n", addr_evictor.offset);

    printf("\tThe eviction address contains (char): [%c], (int): [%d]\n", evictor, evictor);

    printf("\n\tWe now have a victim and an address that can perform eviction.\n");
    printf("\t\tVictim:\t\t%llx\n", &victim);
    printf("\t\tEvictor:\t%llx\n\n", evictor);


    printf("=======================\n");
    printf("Prime and Probe Attack.\n");
    printf("=======================\n\n");

    // enable debug messages beyond this point
    g_DEBUG = 1;

    l1pp_demo(&victim, evictor);

}