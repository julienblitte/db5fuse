/**
 * @file logger.h
 * @brief Header - Log activities
 * @author Julien Blitte
 * @version 0.1
 */
#ifndef INC_LOGGER_H
#define INC_LOGGER_H
#include <stdarg.h>
#include <stdbool.h>

#include "config.h"

/** @brief log-level emergency (system is unusable) */
#define	LOG_EMERG	0
/** @brief log-level alert (action must be taken immediately) */
#define	LOG_ALERT	1
/** @brief log-level critical (critical conditions) */
#define	LOG_CRIT	2
/** @brief log-level error (error conditions) */
#define	LOG_ERR		3
/** @brief log-level warning (warning conditions) */
#define	LOG_WARNING	4
/** @brief log-level notice (normal but significant condition) */
#define	LOG_NOTICE	5
/** @brief log-level info (informational) */
#define	LOG_INFO	6
/** @brief log-level debug (debug-level messages) */
#define	LOG_DEBUG	7


/** @brief defined log-level */
#ifndef CONFIG_LOG_LEVEL
	#define CONFIG_LOG_LEVEL	LOG_NOTICE
#endif

/** @brief log filename - utf8 */
#ifndef CONFIG_LOG_FILENAME
	#define CONFIG_LOG_FILENAME	"db5fuse.log"
#endif



/** @brief add_log cirital loglevel */
#define ADDLOG_CRITICAL		LOG_CRIT
/** @brief add_log assertion is not true loglevel */
#define ADDLOG_CHECK		LOG_ERR
/** @brief add_log operation failed loglevel */
#define ADDLOG_FAIL		LOG_ERR
/** @brief add_log recoverable error loglevel */
#define ADDLOG_RECOVER		LOG_WARNING	
/** @brief add_log error loglevel */
#define	ADDLOG_USER_ERROR	LOG_NOTICE
/** @brief add_log notice loglevel */
#define	ADDLOG_NOTICE		LOG_NOTICE
/** @brief add_log operation inform (fuse function called) loglevel */
#define ADDLOG_OPERATION	LOG_INFO
/** @brief add_log operation success loglevel */
#define ADDLOG_OP_SUCCESS	LOG_DEBUG
/** @brief add_log debug loglevel */
#define ADDLOG_DEBUG		LOG_DEBUG
/** @brief add_log dump loglevel */
#define ADDLOG_DUMP		LOG_DEBUG
/** @brief add_log verbose loglevel */
#define ADDLOG_VERBOSE		(LOG_DEBUG+1)




/**
 * @brief initialize log file
 */
void open_log();
/**
 * @brief add an entry to log file
 * @param level log level
 * @param context of event (object, library, etc.) - utf8
 * @param format text to log (printf format) - utf8
 * @return true if successfull
 */
bool add_log(int level, const char *context, const char *format, ...);
/**
 * @brief close log file
 */
void close_log();

/**
 * @brief dump a (latin1) string variable to log
 * @param var name of variable - utf8
 * @param value variable content - latin1
 * @return true if successfull
 */
bool log_dump_latin1(const char *var, const char *value);

/**
 * @brief dump a (utf8) string variable to log
 * @param var name of variable - utf8
 * @param value variable content - utf8
 * @return true if successfull
 */
bool log_dump(const char *var, const char *value);

#endif
