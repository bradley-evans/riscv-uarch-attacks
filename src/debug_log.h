#ifndef __DEBUG_LOG_H__
#define __DEBUG_LOG_H__


#include <stdio.h>


void _debug_msg(char const * caller_name, char msg[]);


#define debug_msg(args) _debug_msg(__func__, args)


#endif