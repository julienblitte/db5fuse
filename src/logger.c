/**
 * @file logger.c
 * @brief Source - Log activities
 * @author Julien Blitte
 * @version 0.1
 */
#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

#include "logger.h"
#include "utf8.h"

/** @brief Log level labels */
static const char *log_levels[] =
{
	"emerg",
	"alert",
	"critic",
	"error",
	"warning",
	"notice",
	"info",
	"debug",
	"dump"
};

/** @brief Log file */
static FILE *log_filename;

void open_log()
{
	log_filename = fopen(CONFIG_LOG_FILENAME, "wt");
}

bool log_dump_latin1(const char *var, const char *value)
{
	char utf8[256];

	assert(var != NULL);
	assert(value != NULL);

	iso8859_utf8(value, utf8, sizeof(utf8));

	return add_log(ADDLOG_DUMP, "[log]dump", "%s(latin1): '%s'\n", var, utf8);
}

bool log_dump(const char *var, const char *value)
{
	assert(var != NULL);
	assert(value != NULL);

	return add_log(ADDLOG_DUMP, "[log]dump", "%s(utf8): '%s'\n", var, value);
}

bool add_log(int level, const char *context, const char *format, ...)
{
	va_list ap;

	assert(context != NULL);
	assert(format != NULL);
	assert(log_filename != NULL);
	assert((level >= LOG_EMERG) && (level <= LOG_DEBUG));

	/* too verbose */
	if (level > CONFIG_LOG_LEVEL)
	{
		return true;
	}

	/* output not open */
	if (log_filename == NULL)
	{
		log_filename = fopen(CONFIG_LOG_FILENAME, "at");
		if (log_filename == NULL)
		{
			return false;
		}
	}

	/* print header */
	if (fprintf(log_filename, "%s.%s: ", context, log_levels[level]) < 0)
	{
		return false;
	}

	/* print message */
	va_start(ap, format);
	if (vfprintf(log_filename, format, ap) >= 0)
	{
		va_end (ap);
		fflush(log_filename);

		return true;
	}
	va_end (ap);

	return false;
}

void close_log()
{
	assert(log_filename != NULL);
	fclose(log_filename);

	log_filename = NULL;
}
