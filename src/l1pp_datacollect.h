#ifndef __L1PP_DATACOLLECT_H__
#define __L1PP_DATACOLLECT_H__


#define _GNU_SOURCE


#include "low.h"
#include "l1pp/l1pp.h"


#include <sys/signal.h>
#include <sched.h>


#define SIG_VICREAD 40
#define SIG_PRIME 41
#define SIG_PROBE 42




void victimEvict(uint8_t *addr);
void victim_sigquit();
void victim_read();
void * l1pp_dc_victim_process();
void * l1pp_dc_victim_thread();


void l1pp_datacollection_threaded(int numruns);
void l1pp_datacollection_forked(int numruns);


#endif /* __L1PP_DATACOLLECT_H__ */