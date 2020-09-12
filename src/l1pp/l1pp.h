/**
 * @defgroup   L1PP l1pp
 *
 * @file       l1pp.h
 * @ingroup    L1PP l1pp
 * @brief      L1PP prototypes and structures.
 *
 * @author     Bradley Evans
 * @date       July 2020
 */
#ifndef __L1PP_H__
#define __L1PP_H__

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>


#include "../util.h"
#include "../debug_log.h"
#include "../low.h"
#include "../addr.h"


/**
 * @brief      Stores results from an L1PP probe so it can be passed back for
 * analysis.
 */
struct l1pp_result_t {
    uint64_t addr;  /*!< The probed address. */
    uint64_t phys;  /*!< The address' physical address. */
    uint64_t time;  /*!< The memory access time of the probe. */
    uint64_t run;   /*!< Which run this is, e.g. 1st run, 2nd run */
};

uint8_t * l1pp_prime(struct cache_t cache);
struct l1pp_result_t * l1pp_probe(uint8_t *primed_memory, struct cache_t cache);
void * l1pp_victim();
void l1pp_demo(struct cache_t cache);

#endif // __L1PP_H__