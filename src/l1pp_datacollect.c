#include "l1pp_datacollect.h"


char g_DEBUG;


char g_VICTIM_RUNNING = 0;  /*!< Global flag that indicates if victim loop is running. */
char g_ATTACK_RUNNING = 0;  /*!< Global flag that indicates if attacker loop is running. */
char g_VICTIM_START_READ = 0;
char g_VICTIM_READ_DONE = 0;


void * l1pp_dc_victim() {

    uint8_t victimMem1 = 1;
    uint8_t victimMem2 = 2;
    
    FILE *f = fopen("l1pp_datacollection_victim.csv", "w");
    fprintf(f, "virt,phys\n");
    fprintf(f, "0x%x,0x%x\n", &victimMem1, virt_to_phys((uint64_t)&victimMem1,getpid()));
    fprintf(f, "0x%x,0x%x\n", &victimMem2, virt_to_phys((uint64_t)&victimMem2,getpid()));
    fclose(f);

    printf("\tStarted victim process. Victim is reading 0x%x and 0x%x.\n", &victimMem1, &victimMem2);

    g_VICTIM_RUNNING = 1;

    while (!g_ATTACK_RUNNING) { } // wait for attack to start

    printf("\tAttack process showing as initialized from victim.\n");

    while (g_ATTACK_RUNNING) {
        switch(g_VICTIM_START_READ) {
            case 0:
                // do nothing
                break;
            case 1:
                victimMem1 = victimMem1 + victimMem2;
                victimMem2 = victimMem2 + victimMem1;
                victimMem1 = 1;
                victimMem2 = 2;
                g_VICTIM_READ_DONE = 1;
                g_VICTIM_START_READ = 0;
            default:
                // do nothing
                break;
        }
    }
}


void l1pp_datacollection(int numruns) {

    struct cache_t cache = getL1DCache(); /*!< Cache parameters. */
    pthread_t vic_thread; /*!< Victim pthread */
    uint8_t *primed_memory;
    struct l1pp_result_t *rundata;

    printf("\t[Collecting Data]\n");
    printf("\tData collection process has started.\n");

    FILE *f = fopen("l1pp_datacollection_attacker.csv", "w");
    
    // Spawn victim thread
    pthread_create(&vic_thread, NULL, &l1pp_dc_victim, NULL);

    g_ATTACK_RUNNING = 1;
    while (!g_VICTIM_RUNNING) { } // wait for victim to start


    printf("\tVictim has initialized. Starting attack.\n");
    for (int i=0; i<numruns; i++) {

        printf("\r\tRun %d of %d...     ",i+1,numruns);
        fflush(stdout);
        // prime
        primed_memory = l1pp_prime(cache);
        // run victim
        g_VICTIM_START_READ = 1;        // tell victim to do its memory reads
        while (!g_VICTIM_READ_DONE) { } // wait for victim to do its reads.
        g_VICTIM_READ_DONE = 0;         // clear read done flag afterward
        // probe
        rundata = l1pp_probe(primed_memory, cache);
        // save run data
        for (int j=0; j<(cache.sets*4*cache.ways); j++){
            fprintf(f, "0x%x,0x%x,%d\n",
                rundata[j].addr,
                rundata[j].phys,
                rundata[j].time);
        }
    }
    g_ATTACK_RUNNING = 0; // signal that the attack is complete

    pthread_join(vic_thread, NULL);

    fclose(f);
    printf("\n\tAttack run complete.\n");

}


int main(int argc, char* argv[]) {

    int numruns = 0;

    printf("Collecting data for L1D$ Prime and Probe.\n");

    if (argc < 2) {
        printf("No number of runs argument supplied, performing 100 runs. Try ./l1pp_datacollection <numruns>\n");
        numruns = 100;
    } else {
        numruns = atoi(argv[1]);
    }
    printf("Starting data collection process.");

    l1pp_datacollection(numruns);

    printf("Data collection process complete.\n");

    return 0;

}