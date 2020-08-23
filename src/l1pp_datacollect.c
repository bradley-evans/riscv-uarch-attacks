#include "l1pp_datacollect.h"


char g_VICTIM_RUNNING = 0;  /*!< Global flag that indicates if victim loop is running. */
char g_ATTACK_RUNNING = 0;  /*!< Global flag that indicates if attacker loop is running. */
char g_VICTIM_START_READ = 0;
extern char g_DEBUG = 0;


uint64_t l1pp_dc_victim() {

    uint8_t victimMem1 = 1;
    uint8_t victimMem2 = 2;
    
    g_VICTIM_RUNNING = 1;

    while (!g_ATTACK_RUNNING) { } // wait for attack to start

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

    FILE *f = fopen("l1pp_datacollection_attacker.csv", "w");
    
    // Spawn victim thread
    pthread_create(&vic_thread, NULL, &l1pp_dc_victim, NULL);

    g_ATTACK_RUNNING = 1;
    while (!g_VICTIM_RUNNING) { } // wait for victim to start

    for (int i=0; i<numruns; i++) {
        // prime
        primed_memory = l1pp_prime(cache);
        // run victim
        g_VICTIM_START_READ = 1;
        while (g_VICTIM_START_READ) { } // wait for victim to do its reads.
        // probe
        rundata = l1pp_probe(primed_memory, cache);
        // save run data
        for (int j=0; j<(cache.sets*4*cache.ways); j++){
            fprintf(f, "0x%x,%d\n",rundata[j].addr, rundata[j].time);
        }
    }
    fclose(f);
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

    l1pp_datacollection(numruns);

    printf("Done.\n");

    return 0;

}