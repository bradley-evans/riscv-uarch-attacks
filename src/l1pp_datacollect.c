#include "l1pp_datacollect.h"


char g_DEBUG;


char g_VICTIM_RUNNING = 0;  /*!< Global flag that indicates if threaded victim loop is running. */
char g_ATTACK_RUNNING = 0;  /*!< Global flag that indicates if threaded attacker loop is running. */
char g_VICTIM_START_READ = 0;
char g_VICTIM_READ_DONE = 0;


uint64_t g_VICTIM_RUN_USEC_FREQ = 500;


uint8_t *** victimAddrs;    /*!< a 2D array of pointers, made global to allow passing to signaled processes. */
uint64_t setWidth = 3;      /*!< How many sets to take from the middle cache lines. */
struct cache_t cache;


/**
 * @brief      Simple function that reads from memory to effect cache eviction.
 *
 * @param      addr  A target memory address to read.
 */
void victimEvict(uint8_t *addr) {
    uint8_t dummyVar;
    dummyVar = *addr;
}


/**
 * @brief      The victim process will terminate upon recieving SIGQUIT signal
 * from parent via kill().
 */
void victim_sigquit() {
    printf("Victim recieved SIGQUIT signal.\n");
    exit(0);
}


/**
 * @brief      The victim will read from victimAddrs when it recieves a SIG_VICREAD
 * signal from the parent via kill().
 */
void victim_read() {
    signal(SIG_VICREAD, victim_read); // Clear SIG_VICREAD
    // Perform the reads five times, hopefully this makes sure eviction occurs.
    for(uint64_t k=0; k<5; k++) {
        for(uint64_t i=0; i<setWidth; i++) {
            for(uint64_t j=0; j<cache.ways; j++) {
                victimEvict(victimAddrs[i][j]);
            }
        }
    }
}


/**
 * @brief      The victim process. Populates a victimAddrs global array and then
 * enters a loop. Attacking process controls this process via kill() signals.
 *
 * @return     No return.
 */
void * l1pp_dc_victim_process() {
    // Lets make the set selectable.
    // Set up as below, default set will be the middle sets.
    uint8_t victimMem[5 * cache.size];
    uint64_t alignedMem = ((uint64_t)&victimMem + cache.size) & cache.mask_Tag;
    uint64_t victimSet = cache.sets / 2;
    uint8_t dummyVar;

    victimAddrs = malloc(setWidth * cache.ways * sizeof(uint8_t*));

    for (uint64_t i=0; i<setWidth; i++) {
        // start at the victimSet, we'll iterate and evict out a group of three
        // cache lines.
        victimAddrs[i] = malloc(cache.ways * sizeof(uint8_t*));
        uint64_t setOffset = (i+victimSet) << cache.numbits_Offset;

        for (uint64_t j=0; j<cache.ways; j++) {  
            // Each set is going to have a certain number of ways, too, that we
            // need to evict with our probe set. We'll advance our tag bits by
            // the value of the iterator to generate cache.ways different
            // addresses that would map to the same index. 
            uint64_t wayOffset = j << (cache.numbits_Offset + cache.numbits_Set);
            // Then we add all these together to our aligned memory location to
            // get a valid memory address in our process' valid virtual address
            // space that corresponds to a set defined by i. We will generate
            // one of these for every cache way.
            // 
            // Store these all into an array so we don't have to generate them
            // over and over.
            victimAddrs[i][j] = (uint8_t*)(alignedMem + setOffset + wayOffset);
        }
    }

    // The attacker will signal the victim to perform its memory reads off of
    // the victimAddrs array.
    while(1) { }

}


/**
 * @brief      Victim function for an L1PP attack
 */
void * l1pp_dc_victim_thread() {

    uint8_t *victimMem1 = malloc(1);
    uint8_t *victimMem2 = malloc(1);
    uint8_t sum = 0;
    
    FILE *f = fopen("l1pp_datacollection_victim.csv", "w");
    fprintf(f, "virt,phys\n");
    fprintf(f, "0x%x,0x%x\n", &victimMem1, virt_to_phys((uint64_t)victimMem1,getpid()));
    fprintf(f, "0x%x,0x%x\n", &victimMem2, virt_to_phys((uint64_t)victimMem2,getpid()));
    fclose(f);

    printf("\tStarted victim process. Victim is reading 0x%x and 0x%x.\n", victimMem1, victimMem2);
    printf("\t...Additionally, global var g_VICTIM_RUNNING is located at 0x%x\n", &g_VICTIM_RUNNING);
    printf("\t...and global var g_ATTACK_RUNNING is located at 0x%x.\n", &g_ATTACK_RUNNING);

    g_VICTIM_RUNNING = 1;

    while (!g_ATTACK_RUNNING) { } // wait for attack to start

    printf("\tAttack process showing as initialized from victim.\n");

    while (g_ATTACK_RUNNING) {
        switch(g_VICTIM_START_READ) {
            case 0:
                // do nothing
                break;
            case 1:
                victimEvict(victimMem1);
                victimEvict(victimMem2);
                g_VICTIM_READ_DONE = 1;
                g_VICTIM_START_READ = 0;
            default:
                // do nothing
                break;
        }
    }
}


/**
 * @brief      Threaded version of the attack. Will victim thread run like a 
 * state machine controlled by the attacker process via global variables to
 * ensure memory reads occur between prime and probe phases.
 *
 * @param[in]  numruns  The number of times to repeat the attack.
 */
void l1pp_datacollection_threaded(int numruns) {

    cache = getL1DCache(); /*!< Cache parameters. */
    pthread_t vic_thread; /*!< Victim pthread */
    uint8_t *primed_memory;
    struct l1pp_result_t *rundata;

    printf("\t[Collecting Data]\n");
    printf("\tData collection process has started.[THREADED]\n");

    FILE *f = fopen("l1pp_datacollection_attacker_fork.csv", "w");
    
    // Spawn victim thread
    pthread_create(&vic_thread, NULL, &l1pp_dc_victim_thread, NULL);

    g_ATTACK_RUNNING = 1;
    while (!g_VICTIM_RUNNING) { } // wait for victim to start


    printf("\tVictim has initialized. Starting attack.\n");
    for (int i=0; i<numruns; i++) {

        printf("\r\tRun %d of %d...\taligned on 0x%x",i+1,numruns, primed_memory);
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


/**
 * @brief      Forked process version of the attack.
 *
 * @param[in]  numruns  Number of times to repeat the attack.
 */
void l1pp_datacollection_forked(int numruns) {

    cache = getL1DCache(); /*!< Cache parameters. */
    uint8_t *primed_memory;
    struct l1pp_result_t *rundata;
    pid_t child;
    int cpuid = get_hartid();

    // Set CPU affinity to current CPU.
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(cpuid,&mask);
    sched_setaffinity(0, sizeof(cpu_set_t), &mask);

    printf("\t[Collecting Data]\n");
    printf("\tData collection process has started. [FORKED]\n");

    FILE *f = fopen("l1pp_datacollection_attacker_proc.csv", "w");
    
    // Spawn victim process
    child = fork();
    if (child == 0) {
        // child process
        printf("\tSpun up a child process.\n");
        if (get_hartid() != cpuid) {
            printf("Error: wrong CPUID!\n");
            exit(1);
        }
        signal(SIG_VICREAD, victim_read);
        signal(SIGQUIT, victim_sigquit);
        l1pp_dc_victim_process();
    } else if (child == -1) {
        printf("\tChild process failed to spawn. Terminating\n");
        exit(2);
    }
    sleep(3); // give the child some time to spin up.
    printf("\tParent process continuing. Child (victim) is pid %d.\n", child);


    // Do attack.
    for (int i=0; i<numruns; i++) {

        printf("\r\tRun %d of %d...\taligned on 0x%x",i+1,numruns, primed_memory);
        fflush(stdout);
        // prime
        primed_memory = l1pp_prime(cache);

        // Let victim run. Victim will run on a frequency of 
        // g_VICTIM_RUN_USEC_FREQ. multiplying the freq *4 should give our vic
        // plenty of time to evict the cache.
        kill(child, SIG_VICREAD);
        usleep(g_VICTIM_RUN_USEC_FREQ);

        // probe
        rundata = l1pp_probe(primed_memory, cache);
        // // save run data
        for (int j=0; j<(cache.sets*4*cache.ways); j++){
            fprintf(f, "0x%x,0x%x,%d\n",
                rundata[j].addr,
                rundata[j].phys,
                rundata[j].time);
        }
    }
    printf("\n == FORKED ATTACK DONE. ==\n");
    kill(child,SIGQUIT);
    sleep(1);

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
    printf("Starting data collection process.\n");

    // l1pp_datacollection_threaded(numruns);

    l1pp_datacollection_forked(numruns);

    printf("Data collection process complete.\n");

    return 0;

}