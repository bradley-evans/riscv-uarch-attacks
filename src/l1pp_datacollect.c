/**
 * @defgroup   L1PP l1pp
 * 
 * @file       l1pp_datacollect.c
 * @ingroup    L1PP l1pp
 * @brief      Implementations of L1PP data collection functions.
 *
 * @author     Bradley Evans
 * @date       August 2020
 */
#include "l1pp_datacollect.h"


char g_DEBUG;


char g_VICTIM_RUNNING = 0;  /*!< Global flag that indicates if threaded victim loop is running. */
char g_ATTACK_RUNNING = 0;  /*!< Global flag that indicates if threaded attacker loop is running. */
char g_VICTIM_START_READ = 0; /*!< Control variable, tells victim thread to start a read operation. */
char g_VICTIM_READ_DONE = 0; /*!< Control variable, tells attacker that the victim read is done. */


uint64_t g_VICTIM_RUN_USEC_FREQ = 500; /*!< How long we should wait after a victim memory read is initated before proceeding. */


uint8_t *** victimAddrs;    /*!< a 2D array of pointers, made global to allow passing to signaled processes. */
uint64_t setWidth = 3;      /*!< How many sets to take from the middle cache lines. */
struct cache_t cache;       /*!< The cache parameters. Global. */


/**
 * @brief      Simple function that reads from memory to effect cache eviction.
 *
 * @param      addr  A target memory address to read.
 */
void victimEvict(uint8_t *addr) {
    uint8_t dummyVar; /*!< Dummy variable to store the contents of addr. */
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
    uint8_t victimMem[5 * cache.size]; /*!< Allocates a block of memory five times the cache size to avoid segfault. */
    uint64_t alignedMem = ((uint64_t)&victimMem + cache.size) & cache.mask_Tag; /*!< First sequential memory location in victimMem with set=0 offset=0 */
    uint64_t victimSet = cache.sets / 2; /*!< The set that the victim will try to evict. */
    uint8_t dummyVar; /*!< Dummy variable to load stuff into to effect eviction. */

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

    uint8_t *victimMem1 = malloc(1); /*!< A victim memory location. */
    uint8_t *victimMem2 = malloc(1); /*!< A victim memory location. */
    uint8_t sum = 0; /*!< Somewhere to store dummy arithmentic operations. */
    
    FILE *f = fopen("l1pp_datacollection_victim.csv", "w");
    fprintf(f, "virt,phys\n");
    fprintf(f, "0x%lx,0x%lx\n", (uint64_t)&victimMem1, virt_to_phys((uint64_t)victimMem1,getpid()));
    fprintf(f, "0x%lx,0x%lx\n", (uint64_t)&victimMem2, virt_to_phys((uint64_t)victimMem2,getpid()));
    fclose(f);

    printf("\tStarted victim process. Victim is reading 0x%lx and 0x%lx.\n", (uint64_t)victimMem1, (uint64_t)victimMem2);
    printf("\t...Additionally, global var g_VICTIM_RUNNING is located at 0x%lx\n", (uint64_t)&g_VICTIM_RUNNING);
    printf("\t...and global var g_ATTACK_RUNNING is located at 0x%lx.\n", (uint64_t)&g_ATTACK_RUNNING);

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
 * @brief      Threaded version of the attack. 
 * 
 * Victim thread will run like a state machine controlled by the attacker 
 * process via global variables to ensure memory reads occur consistently and 
 * repeatably between prime and probe phases.
 *
 * @param[in]  numruns  The number of times to repeat the attack.
 */
void l1pp_datacollection_threaded(int numruns) {

    cache = getL1DCache(); /*!< Cache parameters. */
    pthread_t vic_thread; /*!< Victim pthread */
    uint8_t *primed_memory; /*!< Pointer to the start of the primed memory area. */
    struct l1pp_result_t *rundata; /*!< Results from the l1pp probe. */

    printf("\t[Collecting Data]\n");
    printf("\tData collection process has started.[THREADED]\n");

    FILE *f = fopen("l1pp_datacollection_attacker_fork.csv", "w");
    
    // Spawn victim thread
    pthread_create(&vic_thread, NULL, &l1pp_dc_victim_thread, NULL);

    g_ATTACK_RUNNING = 1;
    while (!g_VICTIM_RUNNING) { } // wait for victim to start


    printf("\tVictim has initialized. Starting attack.\n");
    for (int i=0; i<numruns; i++) {

        printf("\r\tRun %d of %d...\taligned on 0x%lx",i+1,numruns, (uint64_t)primed_memory);
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
            fprintf(f, "0x%lx,0x%lx,%ld\n",
                rundata[j].addr,
                rundata[j].phys,
                rundata[j].time);
        }
    }
    g_ATTACK_RUNNING = 0; // signal that the attack is complete

    // wait for victim to terminate
    pthread_join(vic_thread, NULL);

    fclose(f);
    printf("\n\tAttack run complete.\n");

}


/**
 * @brief      Forked process version of the attack.
 * 
 * The `*primed_memory` area gets allocated by l1pp_prime() and is used to
 * make sure segmentation faults don't happen when doing prime and probes. This
 * same pointer gets sent off to l1pp_probe() to probe the same area that was
 * primed.
 * The victim process is forked off by this parent process and is controlled via
 * kill() signals.
 * CPU affinity is set to ensure that victim and attacker end up using the same
 * CPU.
 *
 * @param[in]  numruns  Number of times to repeat the attack.
 */
void l1pp_datacollection_forked(int numruns) {
    cache = getL1DCache(); /*!< Cache parameters. */
    uint8_t *primed_memory; /*!< Pointer to the start of the primed memory area. */
    struct l1pp_result_t *rundata; /*!< Results from the l1pp probe. */
    pid_t child; /*!< The PID of the child process launched by the attacker. */
    int cpuid = get_hartid(); /*!< The cpuid that the main process is running on. */

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

        printf("\r\tRun %d of %d...\taligned on 0x%lx",i+1,numruns, (uint64_t)primed_memory);
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
        for (int j=0; j<(cache.sets*cache.ways); j++){
            fprintf(f, "0x%lx,0x%lx,%ld\n",
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

    int numruns = 0; /*!< Number of times to perform the L1PP attack cycle. */

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