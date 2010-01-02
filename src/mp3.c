/**
 * @file mp3.c
 * @brief Source - Mp3 file format
 * @author Julien Blitte
 * @version 0.1
 */
#include <errno.h>
#include <id3tag.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wstring.h>

#include "check.h"
#include "config.h"
#include "db5_types.h"
#include "file.h"
#include "logger.h"
#include "mp3_id3.h"
#include "mp3_mpeg.h"

/**
 * @brief generate mpeg and size information parts
 * @param filename file to get information - utf8
 * @param row row where information are stored
 * @return true if successfull
 */
static bool mp3_generate_row_mpeg_size(const char *filename, db5_row *row)
{
	FILE *mpeg;
	size_t read;
	ptrdiff_t offset;
	mp3_frame frame;

	check(filename != NULL);
	check(row != NULL);

	add_log(ADDLOG_DEBUG, "[mp3]gen_row_mpeg_size", "retrieving mpeg information for '%s'\n", filename);

	mpeg = fopen(filename, "rb");
	if (mpeg == NULL)
	{
		add_log(ADDLOG_RECOVER, "[mp3]gen_row_mpeg_size", "unable to open file '%s': %s\n", filename, strerror(errno));
		return false;
	}

	read = fread(file_common_buffer, 1, sizeof(file_common_buffer), mpeg);
	if (read == 0)
	{
		add_log(ADDLOG_RECOVER, "[mp3]gen_row_mpeg_size", "unable to read file '%s'\n", filename);
		fclose(mpeg);
		return false;
	}

	fclose(mpeg);

	row->filesize = file_filesize(filename);

	offset = mp3_next_frame(file_common_buffer, read);
	if (offset >= read)
	{
		add_log(ADDLOG_NOTICE, "[mp3]gen_row_mpeg_size", "unable to find first frame in file '%s'\n", filename);
		return false;
	}

	ws_memswapcpy(&frame, file_common_buffer+offset, sizeof(mp3_frame));
	row->bitrate = mp3_bitrate(&frame);
	row->samplerate = mp3_samplerate(&frame);
	row->duration = mp3_length(&frame, row->filesize);

	return true;
}


/**
 * @brief set variable to a default value if variable is set to 'error value'
 * @param variable variable to check for error value
 * @param default value to set in case of error
 */
#define int_defaultvalue(variable,default)	if (variable==ID3_INT_ERROR) variable = default;
/**
 * @brief generate tag information parts
 * @param filename file to get information - utf8
 * @param row row where information are stored
 * @return true if successfull
 */
static bool mp3_generate_row_id3(const char *filename, db5_row *row)
{
	struct id3_file *id3;
	struct id3_tag *tag;

	char *str;
	int num;

	check(filename != NULL);
	check(row != NULL);

	add_log(ADDLOG_DEBUG, "[mp3]gen_row_id3", "retrieving id3 tags for '%s'\n", filename);

	id3 = id3_file_open(filename, ID3_FILE_MODE_READONLY);
	if (id3 == NULL)
	{
		add_log(ADDLOG_RECOVER, "[mp3]gen_row_id3", "unable to open file '%s'\n", filename);
		return false;
	}
	
	tag = id3_file_tag(id3);
	if (tag == NULL)
	{
		add_log(ADDLOG_NOTICE, "[mp3]gen_row_id3", "not tag found in file '%s'\n", filename);
	}

	row->year = id3_get_int(tag, ID3_FRAME_YEAR);
	int_defaultvalue(row->year, 0);

	str = id3_get_string(tag, ID3_FRAME_ARTIST);
	if (str == ID3_STRING_ERROR)
	{
		strncpy(row->artist, CONFIG_DEFAULT_ARTIST, membersizeof(db5_row, artist)/2);
	}
	else
	{
		strncpy(row->artist, str, membersizeof(db5_row, artist)/2);
		free(str);
	}

	str = id3_get_string(tag, ID3_FRAME_ALBUM);
	if (str == ID3_STRING_ERROR)
	{
		strncpy(row->album, CONFIG_DEFAULT_ALBUM, membersizeof(db5_row, album)/2);
	}
	else
	{
		strncpy(row->album, str, membersizeof(db5_row, album)/2);
		free(str);
	}

	str = id3_get_string(tag, ID3_FRAME_TITLE);
	if (str == ID3_STRING_ERROR)
	{
		strncpy(row->title, CONFIG_DEFAULT_TITLE, membersizeof(db5_row, title)/2);
	}
	else
	{
		strncpy(row->title, str, membersizeof(db5_row, title)/2);
		free(str);
	}

	row->track = id3_get_int(tag, ID3_FRAME_TRACK);
	int_defaultvalue(row->track, 0);

	row->year = id3_get_int(tag, ID3_FRAME_YEAR);
	int_defaultvalue(row->year, 0);

	num = id3_get_int(tag, ID3_FRAME_GENRE);
	if (num == ID3_INT_ERROR || num >= ID3_NB_GENRES)
	{
		strncpy(row->genre, CONFIG_DEFAULT_GENRE, membersizeof(db5_row, genre)/2);
	}
	else
	{
		strncpy(row->genre, id3_genres[num], membersizeof(db5_row, genre)/2);
	}

	id3_file_close(id3);

	return true;
}

bool mp3_generate_row(const char *filename, db5_row *row)
{
	check(filename != NULL);
	check(row != NULL);

	add_log(ADDLOG_DEBUG, "[mp3]gen_row", "preparing information for '%s'\n", filename);

	if (!mp3_generate_row_mpeg_size(filename, row))
	{
		add_log(ADDLOG_RECOVER, "[mp3]gen_row", "unable to retrieve music length information for '%s'\n", filename);
	}
	if (!mp3_generate_row_id3(filename, row))
	{
		add_log(ADDLOG_RECOVER, "[mp3]gen_row", "unable to retrieve id3 tags for '%s'\n", filename);
	}

	return true;
}

