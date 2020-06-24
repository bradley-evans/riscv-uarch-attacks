#include "demo.h"


int main() {

    struct cpu_t *cpu = malloc(sizeof(struct cpu_t));
    int numCPU = get_numCPUOnline();
    int currCPU = get_hartid();


    printf("RISC-V UARCH ATTACK DEMONSTRATION\n");

    printf("=================================\n");

    /*
     * Initialization Phase
     */

    printf("CPU Initialization: \n");
    cpu = initialize_cpu();


    exit(0);

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
    printf("=================================\n");
    printf("Cache address manipulation demonstration.\n\n");
    int myvar = 0;

    struct address_t addr_myvar = get_Address(cpu[currCPU].cache[0], myvar);

    printf("\tAddress of myvar:\t%llx\n", &myvar);
    printf("\tmyvar Address Size:\t%d bytes, %d bits.\n\n", sizeof(&myvar), sizeof(&myvar)*8);
    printf("\t*******************************\n");
    printf("\t* address_t structure created *\n");
    printf("\t*******************************\n");
    

    printf("\tAddress:\t\t0x%llx\n", addr_myvar.addr);
    printf("\tAddress size:\t\t%d-bit\n", addr_myvar.bitsize);
    printf("\tAddress tag:\t\t0x%llx\n", addr_myvar.tag);
    printf("\tAddress index:\t\t0x%llx\n", addr_myvar.set);
    printf("\tAddress offset:\t\t0x%llx\n\n", addr_myvar.offset);

    printf("\t*******************************\n");
    printf("\t* address_t structure created *\n");
    printf("\t*******************************\n");

    int *evicting_var = generate_Evictor(myvar);


}