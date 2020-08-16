/**
 * @file l1pp.h
 * @author Bradley Evans
 * @date 22 July 2020
 * @brief Low level common functions for RISC-V side-channel attacks.
 */

#ifndef __L1PP_H__
#define __L1PP_H__

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>


#include "../util.h"
#include "../debug_log.h"
#include "../low.h"


static uint8_t* l1pp_prime (struct cache_t cache);
void l1pp_probe(uint8_t *primed_memory, struct cache_t cache);
void l1pp_victim();
void l1pp_demo(struct cache_t cache);

#endif // __L1PP_H__