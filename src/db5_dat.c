/**
 * @file db5_dat.c
 * @brief Source - Database db5, data
 * @author Julien Blitte
 * @version 0.1
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>


#include "check.h"
#include "crc32.h"
#include "config.h"
#include "db5_dat.h"
#include "db5_hdr.h"
#include "db5_types.h"
#include "file.h"
#include "logger.h"
#include "wstring.h"

/** @brief Database data file */
static FILE *db5_dat;

bool db5_dat_init()
{
	crc32_init();

	db5_dat = file_fcaseopen(CONFIG_DB5_DATA_DIR, CONFIG_DB5_DAT_FILE, "rb+");

	if (db5_dat == NULL)
	{
		add_log(ADDLOG_CRITICAL, "[db5/dat]init", "unable to init database\n");
		return false;
	}
	
	return true;
}

void db5_dat_free()
{
	fclose(db5_dat);
}

bool db5_dat_select_row(const uint32_t index, db5_row *row)
{
	check(row != NULL);

	if (fseek(db5_dat, index*sizeof(db5_row), SEEK_SET) != 0)
	{
		add_log(ADDLOG_FAIL, "[db5/dat]read", "unable to find database row (reading)\n");
		return false;
	}
	if (fread(row, sizeof(db5_row), 1, db5_dat) != 1)
	{
		add_log(ADDLOG_FAIL, "[db5/dat]read", "unable to read into database\n");
		return false;
	}

	return true;
}

bool db5_dat_update(const uint32_t index, db5_row *row)
{
	check(row != NULL);

	if (fseek(db5_dat, index*sizeof(db5_row), SEEK_SET) != 0)
	{
		add_log(ADDLOG_FAIL, "[db5/dat]alter", "unable to find database row (writing)\n");
		return false;
	}
	if (fwrite(row, sizeof(db5_row), 1, db5_dat) != 1)
	{
		add_log(ADDLOG_FAIL, "[db5/dat]alter", "unable to write into database\n");
		return false;
	}

	return true;
}

bool db5_dat_insert(db5_row *row)
{
	check(row != NULL);

	if (db5_hdr_count() >= CONFIG_MAX_DB5_ENTRY)
	{
		add_log(ADDLOG_FAIL, "[db5/dat]add", "database if full\n");
		return false;
	}

	if (fseek(db5_dat, db5_hdr_count()*sizeof(db5_row), SEEK_SET) != 0)
	{
		add_log(ADDLOG_FAIL, "[db5/dat]add", "unable to add database row\n");
		return false;
	}
	if (fwrite(row, sizeof(db5_row), 1, db5_dat) != 1)
	{
		add_log(ADDLOG_FAIL, "[db5/dat]add", "unable to write into database\n");
		return false;
	}

	/* update meta-database */
	if (db5_hdr_grow(1) != true)
	{
		add_log(ADDLOG_FAIL, "[db5/dat]add", "unable to update meta-database\n");
		return false;
	}

	return true;
}

bool db5_dat_delete_row(const uint32_t index)
{
	db5_row row;
	uint32_t count;

	count = db5_hdr_count();

	if (index < 0 || index >= count)
	{
		add_log(ADDLOG_FAIL, "[db5/dat]delete", "index out of databse (%u)\n", index);
		return false;
	}

	if (index != count-1)
	{
		if (db5_dat_select_row(count-1, &row) == false)
		{
			return false;
		}

		if (db5_dat_update (index, &row) == false)
		{
			return false;
		}
	}

	/* update meta-database */
	if (db5_hdr_grow(-1) != true)
	{
		add_log(ADDLOG_FAIL, "[db5/dat]delete", "unable to update meta-database\n");
		return false;
	}

	/* resize database file */
	if (!file_truncate(db5_dat, count*sizeof(db5_row)))
	{
		add_log(ADDLOG_FAIL, "[db5/dat]delete", "unable to resize database file\n");
		return false;
	}

	return true;
}

/** @brief size of db5_row.filename */
#define filename_size	(membersizeof(db5_row, filename))

uint32_t db5_dat_select_by_filename(const char *filename)
{
	char shortname [filename_size];
	uint32_t count, i;
	db5_row row;

	check(filename != NULL);

	strncpy(shortname, filename, filename_size/2);
	ws_atows(shortname, filename_size);

	count = db5_hdr_count();
	for(i=0; i < count; i++)
	{
		if(!db5_dat_select_row(i, &row))
		{
			add_log(ADDLOG_FAIL, "[db5/dat]select_by_filename", "error reading database\n");
			return DB5_ROW_NOT_FOUND;
		}

		if (memcmp(row.filename, shortname, filename_size) == 0)
		{
			return i;
		}
	}

	return DB5_ROW_NOT_FOUND;
}

db5_string_entry *db5_dat_select_string_column(const size_t reloffset, const size_t size)
{
	db5_string_entry *result;
	db5_row row;
	uint32_t i, count;
	char *item;
	int len;

	check(reloffset < sizeof(db5_row));
	check(reloffset + size <= sizeof(db5_row));

	count = db5_hdr_count();
	
	result = (db5_string_entry *)calloc(count, sizeof(db5_string_entry));
	if (result == NULL)
	{
		add_log(ADDLOG_FAIL, "[db5/dat]select_string_column", "not enougth memory\n");
		return NULL;
	}

	for(i=0; i < count; i++)
	{
		if (db5_dat_select_row(i, &row) == false)
		{
			db5_dat_free_string_column(result, i);
			return NULL;
		}
		item = ((char *)&row) + reloffset;
		ws_wstoa(item, size);

		len = strlen(item);

		result[i].value = (char *)malloc(len+1);
		if (result[i].value == NULL)
		{
			db5_dat_free_string_column(result, i);
			return NULL;
		}

		result[i].position = i;
		strncpy(result[i].value, item, len+1);
		result[i].crc32 = strcrc32(result[i].value);
	}

	return result;
}

void db5_dat_free_string_column(db5_string_entry *index, const uint32_t count)
{
	uint32_t i;

	check(index != NULL);

	for(i = 0; i < count; i++)
	{
		free(index[i].value);
	}

	free(index);
}


db5_number_entry *db5_dat_select_number_column(const size_t reloffset)
{
	db5_number_entry *result;
	db5_row row;
	uint32_t i, count;
	uint32_t value;

	check(reloffset < sizeof(db5_row));

	count = db5_hdr_count();
	
	result = (db5_number_entry *)calloc(count, sizeof(db5_number_entry));
	check(result != NULL);

	for(i=0; i < count; i++)
	{
		if (db5_dat_select_row(i, &row) == false)
		{
			free(result);
			return NULL;
		}
		value = *(uint32_t *)(((ptrdiff_t)&row) + reloffset);

		result[i].value = value;
		result[i].position = i;
	}

	return result;
}

void inline db5_dat_free_number_column(db5_number_entry *index)
{
	check(index != NULL);
	free(index);
}
