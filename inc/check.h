/**
 * @file check.h
 * @brief Header - Local assert
 * @author Julien Blitte
 * @version 0.1
 */
#include <stdlib.h>
#include "logger.h"

/**
 * @brief local assert function that use internal log system
 * @param condition must be true, else a log entry will be added
 */
#define check(condition)	if(!(condition)){add_log(ADDLOG_CHECK,"assert","Assertion failed at %s:%u.\n",__FILE__,__LINE__);}

