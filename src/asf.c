/**
 * @file asf.c
 * @brief Source - Asf file format
 * @author Julien Blitte
 * @version 0.1
 */
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

#include "asf.h"
#include "db5_types.h"
#include "file.h"
#include "logger.h"
#include "check.h"
#include "wstring.h"

/** @brief asf file magic value */
#define ASF_MAGIC	0x3026b275

/** @brief size of asf tag */
#define SIZEOF_TAG	34

/** @brief UID of an asf tag */
typedef unsigned char guid_t[16];

/** @brief first UID of asf header */
const guid_t title_artist  = { 0x33, 0x26, 0xB2, 0x75, 0x8E, 0x66, 0xCF, 0x11, 0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C };

/**
 * @brief asf tag
 */
typedef struct asf_tag_t
{
	/** @brief uid of tag */
	guid_t guid;
	/** @brief size of tag */
	uint32_t record_size;
	/** @brief ruf */
	uint32_t reserved_1;
	/** @brief title size */
	uint16_t title_size;
	/** @brief artist size */
	uint16_t artist_size;
} asf_tag;


/**
 * @brief compare item1 to item2, 32 bits by 32 bits
 * @param item1 first element to compare
 * @param item2 second element to compare
 * @param size size of shorter item, have to be a multiple of 4
 * @result true if item1 is equivalent to item2
 */
static inline bool compare4(const void *item1, const void *item2, const size_t size)
{
	int i;

	for(i=0; i < size/sizeof(uint32_t); i++)
	{
		if (((uint32_t *)item1)[i] != ((uint32_t *)item2)[i])
		{
			return false;
		}
	}

	return true;
}

/**
 * @brief find the header in an buffer by finding specified guid
 * @param guid the guid of header to find
 * @param buffer the search buffer
 * @paran the size of buffer
 */
static size_t asf_find_header(const guid_t *guid, const char *buffer, const size_t len)
{
	size_t result;

	check(guid != NULL);
	check(buffer != NULL);
	check(len > 0);

	/* no enought data */
	if (len < sizeof(guid_t))
	{
		return len;
	}

	for(result = 0; result <= (len - sizeof(guid_t)); result++)
	{
		if (compare4(buffer+result, &guid, sizeof(guid_t)))
		{
			return result;
		}
	}

	return len;
}

bool asf_generate_row(const char *filename, db5_row *row)
{
	FILE *wma;
	size_t read;
	size_t position;
	asf_tag *header;
	char *artist;
	char *title;

	check(filename != NULL);
	check(row != NULL);

	add_log(ADDLOG_DEBUG, "[asf]gen_row", "preparing information for '%s'\n", filename);

	/* TODO: get real values from WMA file */
	row->bitrate = 128000;
	row->samplerate = 44100;

#ifndef NOJOKE
	row->year = 1984;
#endif

	strncpy(row->album, "Microsoft WMA", membersizeof(db5_row, album)/2);
	strncpy(row->genre, "WMA file", membersizeof(db5_row, genre)/2);

	wma = fopen(filename, "rb");
	if (wma == NULL)
	{
		add_log(ADDLOG_RECOVER, "[asf]gen_row", "unable to open file '%s': %s\n", filename, strerror(errno));
		return true;
	}

	read = fread(file_common_buffer, 1, sizeof(file_common_buffer), wma);
	if (read == 0)
	{
		add_log(ADDLOG_RECOVER, "[asf]gen_row", "unable to read file '%s'", filename);
		fclose(wma);
		return true;
	}

	fclose(wma);

	row->filesize = file_filesize(filename);
	row->duration = row->filesize / (row->bitrate / 8);

	position = asf_find_header(&title_artist, file_common_buffer, read);
	if (position >= read)
	{
		add_log(ADDLOG_NOTICE, "[asf]gen_row", "unable to find tag in file '%s'\n", filename);
		return true;
	}

	header = (asf_tag *)(file_common_buffer+position);

	if (SIZEOF_TAG + header->title_size + header->artist_size != header->record_size)
	{
		add_log(ADDLOG_NOTICE, "[asf]gen_row", "tag size is invalid in file '%s'\n", filename);
		return true;
	}

	if ((position + header->record_size) > read)
	{
		add_log(ADDLOG_NOTICE, "[asf]gen_row", "tag seems too big in filename '%s'\n", filename);
		return true;
	}

	artist = malloc(header->artist_size);
	if (artist == NULL)
	{
		add_log(ADDLOG_RECOVER, "[asf]gen_row", "not enougth memory\n");
	}
	else
	{
		memcpy(artist, file_common_buffer + position + SIZEOF_TAG + header->title_size, header->artist_size);
		ws_wstoa(artist, header->artist_size);
		strncpy(row->artist, artist, membersizeof(db5_row, artist)/2);
		free(artist);
	}

	title = malloc(header->title_size);
	if (title != NULL)
	{
		add_log(ADDLOG_RECOVER, "[asf]gen_row", "not enougth memory\n");
	}
	else
	{
		memcpy(title, file_common_buffer + position + SIZEOF_TAG, header->title_size);
		ws_wstoa(title, header->title_size);
		strncpy(row->title, title, membersizeof(db5_row, title)/2);
		free(title);
	}

	return true;
}

