#include "l1_contention.h"


char g_VICTIM_RUNNING = 0;
char g_ATTACK_RUNNING = 0;


void l1_contention_victim_process(void *victim) {
    
    // victim initializing

    char *msg = malloc(100);
    printf("\tvictim proc, on cpu%d.\n", get_hartid());
    g_VICTIM_RUNNING = 1;
    while (!g_ATTACK_RUNNING) { } // wait for attacker
    sprintf(msg, "Victim is running. Target %llx. Attacker running? %d", victim, g_ATTACK_RUNNING);
    debug_msg(msg);

    // victim reads from memory

    for (int i=0; i<2; i++) {
        for (int j=0; j<1000; j++) {
            asm_load(victim);
        }
        printf("\n\ttick...%d\n", i+1);
        sleep(1);
    }
    
    // victim teardown

    g_VICTIM_RUNNING = 0;
    debug_msg("Victim is done.");
}


void l1_contention_attack_process(void *evictor) {

    // attack initializing

    printf("\tattack proc, on cpu%d.\n", get_hartid());
    char *msg = malloc(100);
    struct timespec start, end;
    double time_spent;
    g_ATTACK_RUNNING = 1;
    while (!g_VICTIM_RUNNING)  { } // wait for victim
    sprintf(msg, "Attacker is running. Evictor %llx. Victim running? %d", evictor, g_VICTIM_RUNNING);
    debug_msg(msg);



    // attacker times memory accesses

    unsigned long long int count = 0;
    while (g_VICTIM_RUNNING) {
        count = count + 1;

        clock_gettime(CLOCK_REALTIME, &start);
        usleep(10000);
        clock_gettime(CLOCK_REALTIME, &end);
        time_spent = (((end.tv_sec - start.tv_sec)*1000000000) +
                     ((end.tv_nsec - start.tv_nsec)))/1000;
        printf("...read time: %f us, count %d\n", time_spent, count);

        usleep(1000);
    }

    printf("count: %lld\n", count);

    // attack teardown

    g_ATTACK_RUNNING = 0;
    debug_msg("Attacker is done.");
}


void l1_contention_demo(int *victim, int *evictor) {
    int cpuid = get_hartid();
    pthread_t vic_thread, att_thread;
    pthread_create(&vic_thread, NULL, &l1_contention_victim_process, victim);
    pthread_create(&att_thread, NULL, &l1_contention_attack_process, evictor);

    debug_msg("Waiting for victim to complete...");
    pthread_join(vic_thread, NULL);
    debug_msg("Victim done! Waiting for attacker to complete...");
    pthread_join(att_thread, NULL);
    debug_msg("Attacker and victim threads exited.");

    printf("\tL1 Contention Demonstration Complete.\n");

    pthread_exit(NULL);
}