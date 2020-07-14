/**
 * @defgroup   DEBUG_LOG debug log
 *
 * @file       debug_log.c
 * @brief      This file implements debug log.
 *
 * @author     Bradley Evans
 * @date       June 2020
 */
#include "debug_log.h"


extern char g_DEBUG;


void _debug_msg(char const * caller_name, char msg[]) {
    if (g_DEBUG) {
        printf("DEBUG: [%s] %s\n", caller_name, msg);
    }
}