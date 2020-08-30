#include "prototype_fork_to_same_cpu.h"


/**
 * @brief      Prints the cpu affinity.
 * Taken from https://stackoverflow.com/questions/10490756/how-to-use-sched-getaffinity-and-sched-setaffinity-in-linux-from-c
 */
void print_affinity() {
    cpu_set_t mask;
    long nproc, i;

    if (sched_getaffinity(0, sizeof(cpu_set_t), &mask) == -1) {
        perror("sched_getaffinity");
        assert(false);
    }
    nproc = sysconf(_SC_NPROCESSORS_ONLN);
    printf("sched_getaffinity = ");
    for (i = 0; i < nproc; i++) {
        printf("%d ", CPU_ISSET(i, &mask));
    }
    printf("\n");
}


/**
 * @brief      Determines which CPU this process is running on.
 * Adapted from: 
 * https://hpcf.umbc.edu/general-productivity/checking-which-cpus-are-used-by-your-program/
 *
 * @return     The hartid.
 */
int get_hartid() {

    FILE *fp = fopen("/proc/self/stat", "r");
    long to_read = 8192;
    char buff[to_read];
    int read = fread(buff, sizeof(char), to_read, fp);
    fclose(fp);

    char* line = strtok(buff, " ");
    for (int i=1; i<38; i++) {
        line = strtok(NULL, " ");
    }
    line = strtok(NULL, " ");

    return atoi(line);
}


void sigquit() {
    printf("pid %d recieved SIGQUIT.\n", getpid());
    exit(0);
}


void processA(pid_t child) {
    printf("ProcessA: pid %d on cpu%d.\n", getpid(), get_hartid());
    usleep(10000);

    printf("ProcessA: sending SIGQUIT.\n");
    kill(child,SIGQUIT);

    exit(0);
}


void processB() {
    printf("ProcessB: pid %d on cpu%d.\n", getpid(), get_hartid());
    while(1) {
        // wait for daddy
    }
}


int main() {
    int cpuid = get_hartid();

    // set our affinity to the current CPU.
    // the child should inherit this affinity and run
    // on the same CPU.
    print_affinity();
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(cpuid,&mask);
    sched_setaffinity(0, sizeof(cpu_set_t), &mask);
    print_affinity();


    printf("Forking two processes into the same CPU.\n");
    printf("The original process pid %d is on cpu%d.\n", getpid(), get_hartid());



    pid_t child = fork();
    if (child == 0) {
        // child process
        if (get_hartid() != cpuid) {
            printf("Did not end up on right CPUID.\n");
            exit(1);
        }
        signal(SIGQUIT, sigquit);
        processB();
    }

    processA(child);

    printf("End of pid %d from main().\n", getpid());

    return 0;
}