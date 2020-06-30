#include "l1pp.h"


void l1pp_victim_process(int *victim) {
    for (int i=0; i<1000; i++) {
        for (int j=0; j<1000; j++) {
             loadword(victim);       
        }
        printf("\ttick...%d\n");
        delay(100);
    }
}


void l1pp_attack_process(int *evictor) {
    clock_t t;
    for (int k=0; i<1000; k++) {
        t = clock();
        loadword(evictor);
        t = clock() - t;
        printf("time: %lf\n",t);
    }
    delay(11);
}


void l1pp_demo(int *victim, int *evictor) {
    int cpuid = get_hartid();

    // We need to get the victim and attacker running on the
    // same CPU.
    if (fork() == 0) {
        while(get_hartid() != cpuid) {
            if (fork() != 0) {
                exit(0);
            }
        }
        printf("\tl1pp victim on cpu%d\n", get_hartid());
        l1pp_victim_process(victim);
    } else {
        printf("\tl1pp attacker on cpu%d\n", get_hartid());
        l1pp_attack_process(evictor);
    }
}