/**
 * @defgroup   L1_CONTENTION l1_contention
 *
 * @file       l1_contention.h
 * @brief      Demonstration functions that show measurability of L1 cache contention.
 *
 * @author     Bradley Evans
 * @date       June 2020
 */
#ifndef __L1_CONTENTION_H__
#define __L1_CONTENTION_H__


#include <stdio.h>
#include <pthread.h>
#include <unistd.h>


#include "../util.h"
#include "../debug_log.h"
#include "../low.h"


void * l1_contention_victim_process(void *victim);
void l1_contention_attack_process(void *victim, struct cache_t cache);
void l1_contention_demo(int *victim, struct cache_t cache);


#endif