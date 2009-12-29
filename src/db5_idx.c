/**
 * @file db5_idx.c
 * @brief Source - Database db5, index
 * @author Julien Blitte
 * @version 0.1
 */
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "check.h"
#include  "db5_dat.h"
#include  "db5_hdr.h"
#include  "db5_idx.h"
#include  "db5_types.h"
#include  "file.h"
#include "logger.h"


/** @brief get n-th byte of an object */
#define byteof(c,i) (((char *)&c)[(i)])


/**
 * @brief compare two db5_string_entry
 * @param entry1 first string
 * @param entry2 second string
 * @return postive if entry1 > entry2, negative else
 */
static int db5_idx_string_compare(const void *entry1, const void *entry2)
{
	check(entry1 != NULL);
	check(entry2 != NULL);

	return strcasecmp(((db5_string_entry *)entry1)->value, ((db5_string_entry *)entry2)->value);
}

/**
 * @brief order a string column
 * @param list string column
 * @param count number of entries
 */
static void db5_idx_order_string_column(db5_string_entry *list, const uint32_t count)
{
	check(list != NULL);

	qsort(list, count, sizeof(db5_string_entry), db5_idx_string_compare);
}

/**
 * @brief compare two db5_number_entry
 * @param entry1 first number
 * @param entry2 second number
 * @return postive if entry1 > entry2, negative else
 */
static int db5_idx_number_compare(const void *entry1, const void *entry2)
{
	check(entry1 != NULL);
	check(entry2 != NULL);

	return ((db5_number_entry *)entry1)->value - ((db5_number_entry *)entry2)->value;
}

/**
 * @brief order a number column
 * @param list number column
 * @param count number of entries
 */
void db5_idx_order_number_column(db5_number_entry *list, const uint32_t count)
{
	check(list != NULL);

	qsort(list, count, sizeof(db5_number_entry), db5_idx_number_compare);
}

/**
 * @brief generate an index
 * @param index data pointer
 * @param count number of elements
 * @param code index code to generate
 * @param numeric true is data are number
 * @return true if successfull
 */
bool db5_idx_index_column_full(void *index, const uint32_t count, const uint32_t code, const bool numeric)
{
	FILE *file;
	char filename[32];
	uint32_t i;
	struct { uint32_t hidden; uint32_t position; uint32_t uid; } item;

	check(index != NULL);

	snprintf(filename, sizeof(filename), CONFIG_DB5_IDX_FILE, byteof(code, 0), byteof(code, 1), byteof(code, 2), byteof(code, 3));
	file = file_fcaseopen(CONFIG_DB5_DATA_DIR, filename, "rb+");

	if (file == NULL)
	{
		return false;
	}

	item.hidden = 0;
	for(i=0; i < count; i++)
	{
		if (numeric)
		{
			item.position = ((db5_number_entry *)index)[i].position;
			item.uid = ((db5_number_entry *)index)[i].value;
		}
		else
		{
			item.position = ((db5_string_entry *)index)[i].position;
			item.uid = ((db5_string_entry *)index)[i].crc32;
		}


		if (fwrite(&item, sizeof(item), 1, file) != 1)
		{
			fclose(file);
			return false;
		}
	}

	fclose(file);
	
	return true;
}

bool db5_idx_index_string_column(const size_t reloffset, const size_t size, const uint32_t code)
{
	db5_string_entry *list;
	uint32_t count;

	check(reloffset <= sizeof(db5_row));

	count = db5_hdr_count();

	list = db5_dat_select_string_column(reloffset, size);
	if (list == NULL)
	{
		add_log(ADDLOG_FAIL, "[db5/idx]index_str", "error while indexing '%c%c%c%c' (select)\n",
			byteof(code, 0), byteof(code, 1), byteof(code, 2), byteof(code, 3));
		return false;
	}

	db5_idx_order_string_column(list, count);
	if (db5_idx_index_column_full(list, count, code, false) == false)
	{
		add_log(ADDLOG_FAIL, "[db5/idx]index_str", "error generating index '%c%c%c%c'\n",
			byteof(code, 0), byteof(code, 1), byteof(code, 2), byteof(code, 3));
		return false;
	}

	db5_dat_free_string_column(list, count);

	return true;
}

bool db5_idx_index_number_column(const size_t reloffset, const uint32_t code)
{
	db5_number_entry *list;
	uint32_t count;

	check(reloffset < sizeof(db5_row));

	count = db5_hdr_count();

	list = db5_dat_select_number_column(reloffset);
	if (list == NULL)
	{
		add_log(ADDLOG_FAIL, "[db5/idx]index_num", "error while indexing '%c%c%c%c' (select)\n",
			byteof(code, 0), byteof(code, 1), byteof(code, 2), byteof(code, 3));
		return false;
	}

	db5_idx_order_number_column(list, count);
	if (db5_idx_index_column_full(list, count, code, true) == false)
	{
		add_log(ADDLOG_FAIL, "[db5/idx]index_num", "error generating index '%c%c%c%c'\n",
			byteof(code, 0), byteof(code, 1), byteof(code, 2), byteof(code, 3));
		return false;
	}

	db5_dat_free_number_column(list);

	return true;
}

