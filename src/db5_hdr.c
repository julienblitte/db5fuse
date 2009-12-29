/**
 * @file db5_hdr.c
 * @brief Source - Database db5, meta-data
 * @author Julien Blitte
 * @version 0.1
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "check.h"
#include "db5_hdr.h"
#include "db5_types.h"
#include "file.h"
#include "logger.h"

/** @brief Offset in database meta-data file */
#define DB5_HDR_COUNT_OFFSET	1040

/** @brief Database meta-data file */
static FILE *db5_hdr;

/** @brief Current number entries */
static uint32_t count;

bool db5_hdr_init()
{
	db5_hdr = file_fcaseopen(CONFIG_DB5_DATA_DIR, CONFIG_DB5_HDR_FILE, "rb+");

	if (db5_hdr == NULL)
	{
		add_log(ADDLOG_CRITICAL, "[db5/hdr]init", "unable to init database\n");
		return false;
	}

	if (fseek(db5_hdr, DB5_HDR_COUNT_OFFSET, SEEK_SET) != 0)
	{
		add_log(ADDLOG_CRITICAL, "[db5/hdr]init", "unable to find count value\n");
		return false;
	}
	if (fread(&count, sizeof(count), 1, db5_hdr) != 1)
	{
		add_log(ADDLOG_CRITICAL, "[db5/hdr]init", "unable to read count value\n");
		return false;
	}

	return true;
}

bool db5_hdr_free()
{
	fclose(db5_hdr);

	return true;
}

uint32_t db5_hdr_count()
{
	if (count == 0)
	{
		add_log(ADDLOG_NOTICE, "[db5/hdr]get", "the value count is zero\n");
	}
	return count;
}

bool db5_hdr_grow(const int delta)
{
	count += delta;

	if (fseek(db5_hdr, DB5_HDR_COUNT_OFFSET, SEEK_SET) != 0)
	{
		add_log(ADDLOG_FAIL, "[db5/hdr]add", "unable to find count value\n");
		return false;
	}
	if (fwrite(&count, sizeof(count), 1, db5_hdr) != 1)
	{
		add_log(ADDLOG_FAIL, "[db5/hdr]add", "unable to write count value\n");
		return false;
	}
	if (fflush(db5_hdr) != 0)
	{
		add_log(ADDLOG_FAIL, "[db5/hdr]add", "unable to write count value (flush)\n");
		return false;
	}

	return true;
}

