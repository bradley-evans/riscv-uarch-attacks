#include "debug_log.h"


extern char g_DEBUG;


void _debug_msg(char const * caller_name, char msg[]) {
    if (g_DEBUG) {
        printf("DEBUG: [%s] %s\n", caller_name, msg);
    }
}