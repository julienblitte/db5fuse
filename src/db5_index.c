/**
 * @file db5_index.c
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
#include  "db5_index.h"
#include  "db5_types.h"
#include  "file.h"
#include  "crc32.h"
#include "logger.h"


/** @brief get n-th byte of an object */
#define byteof(c,i) (((char *)&c)[(i)])


/** @brief index entry */
typedef struct
{
	/** @brief if entry should be hidden */
	uint32_t hidden;
	/** @brief position of entry in database */
	uint32_t position;
	/** @brief unique identifier for this entry */
	uint32_t uid;
} index_entry;


/** @brief data used to generate index */
static char *index_master_data;

/** @brief size of data to index */
static uint32_t index_entry_size;



/**
 * @brief compare two index entries using index_master_data
 * @param entry1 first entry
 * @param entry2 second entry
 * @return postive if entry1 > entry2, negative else
 */
static int db5_index_compare_entries(const void *entry1, const void *entry2)
{
	uint32_t pos1, pos2;

	check(entry1 != NULL);
	check(entry2 != NULL);

	pos1 = ((index_entry *)entry1)->position;
	pos2 = ((index_entry *)entry2)->position;

	return memcmp(index_master_data+pos1*index_entry_size, index_master_data+pos2*index_entry_size, index_entry_size);
}

/**
 * @brief dump an index table
 * @param entries index entries
 * @param count number of entries
 */
static void index_dump_table(const index_entry *entries, const uint32_t count)
{
	uint32_t i;

	for(i=0; i < count; i++)
	{
		add_log(ADDLOG_DUMP, "[db5/index]index_col", " - [%08x] %u\n", entries[i].uid, entries[i].position);
	}
}


bool db5_index_index_column(const ptrdiff_t reloffset, const size_t size, const uint32_t code)
{
	char filename[PATH_MAX];
	db5_row row;
	FILE *file;
	uint32_t count, i;
	index_entry *entries;

	check(reloffset < sizeof(db5_row));
	check(reloffset+size <= sizeof(db5_row));

	add_log(ADDLOG_DEBUG, "[db5/index]index_col", "generating index for code '%c%c%c%c'\n",
		byteof(code, 0), byteof(code, 1), byteof(code, 2), byteof(code, 3));

	/* build index filename and open it */
	snprintf(filename, sizeof(filename), CONFIG_DB5_IDX_FILE, byteof(code, 0), byteof(code, 1), byteof(code, 2), byteof(code, 3));
	file = file_fcaseopen(CONFIG_DB5_DATA_DIR, filename, "wb");
	if (file == NULL)
	{
		add_log(ADDLOG_FAIL, "[db5/index]index_col", "unable to generate index file\n");
		return false;
	}

	/* get number of entries */
	count = db5_hdr_count();
	if (count == 0)
	{
		add_log(ADDLOG_NOTICE, "[db5/index]index_col", "no data to index\n");
		fclose(file);
		return false;
	}

	/* memory allocating */
	index_master_data = (char *)malloc(size*count);
	if (index_master_data == NULL)
	{
		add_log(ADDLOG_FAIL, "[db5/index]index_col", "not enought memory (%u entries)\n", count);
		fclose(file);
		return false;
	}

	entries = (index_entry *)malloc(sizeof(index_entry)*count);
	if (entries == NULL)
	{
		add_log(ADDLOG_FAIL, "[db5/index]index_col", "not enought memory (%u entries)\n", count);
		free(index_master_data), fclose(file);
		return false;
	}

	/* load master data in memory and prepare results */
	index_entry_size = size;

	for(i=0; i < count; i++)
	{
		if (!db5_dat_select_row(i, &row))
		{
			add_log(ADDLOG_FAIL, "[db5/index]index_col", "unable to read entry %u\n", i);
			free(index_master_data), free(entries), fclose(file);
			return false;
		}
		memcpy(index_master_data+i*size, ((char *)&row)+reloffset, size);

		entries[i].hidden = row.hidden;
		entries[i].position = i;
	}

	/* generate uid */
	if (size <= membersizeof(index_entry, uid))
	{
		for(i=0; i < count; i++)
		{
			entries[i].uid = 0;
			memcpy(&entries[i].uid, index_master_data+i*size, size);
		}
	}
	else
	{
		for(i=0; i < count; i++)
		{
			entries[i].uid = crc32(index_master_data+i*size, size);
		}
	}

	/* sort data */
	qsort(entries, count, sizeof(index_entry), db5_index_compare_entries);

#ifdef DEBUG
	index_dump_table(entries, count);
#endif

	/* save data */
	if (fwrite(entries, sizeof(index_entry), count, file) != count)
	{
		add_log(ADDLOG_FAIL, "[db5/index]index_col", "unable to wire index data to file '%s'\n", filename);
		free(index_master_data), free(entries), fclose(file);
		return false;
	}

	free(index_master_data);
	free(entries);
	fclose(file);

	add_log(ADDLOG_DEBUG, "[db5/index]index_col", "done.\n");

	return true;
}

