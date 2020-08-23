/**
 * @defgroup   L1_CONTENTION l1_contention
 *
 * @file       l1_contention.c
 * @brief      Function implementations that measures the effect of L1
 * cache contention on memory reads. This effect can be used to create
 * covert channels or side channels.
 *
 * @author     Bradley Evans
 * @date       June 2020
 */
#include "l1_contention.h"


char g_VICTIM_RUNNING = 0;  /*!< Global flag that indicates if victim loop is running. */
char g_ATTACK_RUNNING = 0;  /*!< Global flag that indicates if attacker loop is running. */


/**
 * @brief      This function performs a memory read and times that memory read.
 * If the attacking process is able to successfully evict from the cache, then 
 * we should see the read time spike periodically.
 *
 * @param      victim  The victim address.
 */
void * l1_contention_victim_process(void *victim) {
    
    // victim initializing
    int v;
    uint64_t start, end;

    FILE *f = fopen("victim_process_timing_data.csv", "w");

    char *msg = malloc(100);
    printf("\tvictim proc, on cpu%d.\n", get_hartid());

    // wait for attacker to begin
    g_VICTIM_RUNNING = 1;
    while (!g_ATTACK_RUNNING) { }
    sprintf(msg, "Victim is running. Target %llx. Attacker running? %d", victim, g_ATTACK_RUNNING);
    debug_msg(msg);

    // victim reads from memory

    printf("index,runcycles,start,end\n");
    for (int i=0; i<200; i++) {
        serialize();
        start = cycles();
        v = asm_load(victim);
        serialize();
        end = cycles();
        printf("%d,%d,%d,%d\n", i, end-start, start, end);
    }

    fclose(f);
    
    // victim teardown

    g_VICTIM_RUNNING = 0;
    debug_msg("Victim is done. File at victim_process_timing_data.csv");
}


/**
 * @brief      Creates L1 cache contention by periodically flushing
 * the cache.
 *
 * @param      victim  Target address for flushing.
 * @param[in]  cache   Struct containing cache parameters.
 */
void l1_contention_attack_process(void *victim, struct cache_t cache) {

    // attack initializing
    printf("\tattack proc, on cpu%d.\n", get_hartid());
    char *msg = malloc(100);
    g_ATTACK_RUNNING = 1;
    while (!g_VICTIM_RUNNING)  { } // wait for victim
    sprintf(msg, "Attacker is running. Victim %llx. Victim running? %d", (uint64_t)victim, g_VICTIM_RUNNING);
    debug_msg(msg);

    // attacker creates contention by flushing
    // the cache periodically
    while (g_VICTIM_RUNNING) {
        flushcache((uint64_t)victim, (uint64_t)sizeof(victim), cache);
        usleep(100);
    }

    // attack teardown

    g_ATTACK_RUNNING = 0;
    debug_msg("Attacker is done!");
}


/**
 * @brief      Demo runner for L1 cache contention measurements.
 *
 * @param      victim   Target address.
 * @param[in]  cache    Cache parameters.
 */
void l1_contention_demo(int *victim, struct cache_t cache) {
    int cpuid = get_hartid();
    pthread_t vic_thread;
    
    // Spawn victim thread
    pthread_create(&vic_thread, NULL, &l1_contention_victim_process, victim);
    
    /* 
     * Originally I wanted to spawn an attacker thread and a victim
     * thread, but the attacker thread would segfault once we called
     * the flushcache routine. Placing the attack function back into
     * the main process resolved the segfault.
     */

    // Spawn attacker thread
    // pthread_create(&att_thread, NULL, &l1_contention_attack_process, victim);
    l1_contention_attack_process(victim, cache);
    debug_msg("Attack function ended, back in l1_contention_demo.");


    debug_msg("Waiting for victim to complete...");
    pthread_join(vic_thread, NULL);
    // debug_msg("Victim done! Waiting for attacker to complete...");
    // pthread_join(att_thread, NULL);
    debug_msg("Attacker and victim threads exited.");

    printf("\tL1 Contention Demonstration Complete.\n");
}