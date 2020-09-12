/**
 * @defgroup   L1PP l1pp
 * 
 * @file       l1pp_datacollect.h
 * @ingroup    L1PP l1pp
 * @brief      Data collection functions for L1PP attacks.
 *
 * @author     Bradley Evans
 * @date       August 2020
 */

#ifndef __L1PP_DATACOLLECT_H__
#define __L1PP_DATACOLLECT_H__


#define _GNU_SOURCE


#include <sys/signal.h>
#include <sched.h>
#include "low.h"
#include "l1pp/l1pp.h"


#define SIG_VICREAD 40  /*!< Signal the victim to perform a read operation. */


void victimEvict(uint8_t *addr);
void victim_sigquit();
void victim_read();
void * l1pp_dc_victim_process();
void * l1pp_dc_victim_thread();
void l1pp_datacollection_threaded(int numruns);
void l1pp_datacollection_forked(int numruns);


#endif /* __L1PP_DATACOLLECT_H__ */