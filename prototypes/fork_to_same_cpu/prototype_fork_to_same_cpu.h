#ifndef __PROTOTYPE_FORK_TO_SAME_CPU_H__
#define __PROTOTYPE_FORK_TO_SAME_CPU_H__


#define _GNU_SOURCE


#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include <sys/signal.h>
#include <sched.h>
#include <assert.h>
#include <stdbool.h>


#define SIGFLAG1 10


void sigquit();
void sighup();
void sigflag1();
void print_affinity();
void processA();
void processB();



#endif /* __PROTOTYPE_FORK_TO_SAME_CPU_H__ */
