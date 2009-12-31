/**
 * @file db5.c
 * @brief Source - Database db5
 * @author Julien Blitte
 * @version 0.1
 */
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include "asf.h"
#include "check.h"
#include "config.h"
#include "db5.h"
#include "db5_dat.h"
#include "db5_hdr.h"
#include "db5_idx.h"
#include "db5_types.h"
#include "file.h"
#include "mp3.h"
#include "names.h"
#include "utf8.h"
#include "wstring.h"

bool db5_init()
{
	if (db5_hdr_init() == false)
	{
		return false;
	}
	if (db5_dat_init() == false)
	{
		db5_hdr_free();
		return false;
	}
	if (names_init() == false)
	{
		db5_hdr_free();
		db5_dat_free();
		return false;
	}

	return true;
}

void db5_widechar_row(db5_row *row)
{
	check(row != NULL);

	ws_atows(row->filepath, membersizeof(db5_row, filepath));
	ws_atows(row->filename, membersizeof(db5_row, filename));
	ws_atows(row->artist,   membersizeof(db5_row, artist));
	ws_atows(row->album,    membersizeof(db5_row, album));
	ws_atows(row->genre,    membersizeof(db5_row, genre));
	ws_atows(row->title,    membersizeof(db5_row, title));
}

void db5_unwidechar_row(db5_row *row)
{
	check(row != NULL);

	ws_wstoa(row->filepath, membersizeof(db5_row, filepath));
	ws_wstoa(row->filename, membersizeof(db5_row, filename));
	ws_wstoa(row->artist,   membersizeof(db5_row, artist));
	ws_wstoa(row->album,    membersizeof(db5_row, album));
	ws_wstoa(row->genre,    membersizeof(db5_row, genre));
	ws_wstoa(row->title,    membersizeof(db5_row, title));
}

bool db5_generate_row(const char *localfile, db5_row *row)
{
	char *dir, *file, *ext;
	char localfile_latin1[PATH_MAX];
	char namebuffer[PATH_MAX];

	check(localfile != NULL);
	check(row != NULL);

	add_log(ADDLOG_DUMP, "[db5]generate_row", "building informations for '%s'\n", localfile);

	memset(row, 0, sizeof(db5_row));

	/* charset handling */
	utf8_iso8859(localfile, localfile_latin1, sizeof(localfile_latin1));

	/* path information */
	strncpy(namebuffer, localfile_latin1, sizeof(namebuffer));
	file_path_explode(namebuffer, &dir, &file, &ext);

	/* check file extension */
	if (ext == NULL)
	{
		ext = "";
		add_log(ADDLOG_RECOVER, "[db5]generate_row", "unable to get extension of '%s'\n", localfile);
	}
	if (strcasecmp(ext, CONFIG_ASF_EXT) != 0 && strcasecmp(ext, CONFIG_MPEG_EXT) != 0)
	{
		add_log(ADDLOG_FAIL, "[db5]generate_row", "extension is unknow\n");
		log_dump_latin1("ext", ext);
		return false;
	}

	/* directory */
	snprintf(row->filepath, membersizeof(db5_row, filepath)/2, "%s/", dir);
	file_windows_slashes(row->filepath);

	/* filename */
	snprintf(row->filename, membersizeof(db5_row, filename)/2, "%s.%s", file, ext);

	if (!file_exists(localfile))
	{
		add_log(ADDLOG_RECOVER, "[db5]generate_row", "unable to get information from file, default values will be used\n");
		log_dump("localfile", localfile);

		/* default values */
		snprintf(row->artist,membersizeof(db5_row, artist)/2, CONFIG_DEFAULT_ARTIST);
		snprintf(row->album, membersizeof(db5_row, album)/2, CONFIG_DEFAULT_ALBUM);
		snprintf(row->genre, membersizeof(db5_row, genre)/2, CONFIG_DEFAULT_GENRE);
		snprintf(row->title, membersizeof(db5_row, title)/2, CONFIG_DEFAULT_TITLE);

		return true;
	}

	if (strcasecmp(ext, CONFIG_ASF_EXT) == 0)
	{
		return asf_generate_row(localfile, row);
	}
	else if (strcasecmp(ext, CONFIG_MPEG_EXT) == 0)
	{
		return mp3_generate_row(localfile, row);
	}

	return false;
}


char **db5_select_filename()
{
	uint32_t count, i;
	db5_row row;
	char **result;
	char filename[PATH_MAX];
	
	add_log(ADDLOG_DEBUG, "[db5]select_filename", "called\n");

	count = db5_hdr_count();

	result = (char **)malloc((count+1) * sizeof(char *));
	if (result == NULL)
	{
		add_log(ADDLOG_CRITICAL, "[db5]select_filename", "not enougth memory\n");
		return NULL;
	}

	for(i=0; i < count; i++)
	{
		if (db5_dat_select_row(i, &row) != true)
		{
			add_log(ADDLOG_FAIL, "[db5]select_filename", "unable to get file information form database, entry id: %u\n", i);
			free(result);
			return NULL;
		}
		/* ws_wstoa(row.filename, membersizeof(db5_row, filename)); */
		db5_unwidechar_row(&row);

		iso8859_utf8(names_select_longname(row.filename), filename, sizeof(filename));

		result[i] = (char *)malloc((strlen(filename)+1)*sizeof(char));
		if (result[i] == NULL)
		{
			add_log(ADDLOG_CRITICAL, "[db5]select_filename", "not enougth memory\n");
			return NULL;
		}

		strcpy(result[i], filename);
	}

	result[count] = NULL;

	add_log(ADDLOG_DUMP, "[db5]select_filename", "returns %u file(s)\n", count);

	return result;
}

void db5_free()
{
	db5_hdr_free();
	db5_dat_free();
	names_free();
}

bool db5_insert(const char *filename)
{
	char shortname[membersizeof(db5_row, filename)];
	char filename_latin1[PATH_MAX];
	char localfile[PATH_MAX];
	db5_row row;

	check(filename != NULL);

	/* convert to latin 1 */
	utf8_iso8859(filename, filename_latin1, sizeof(filename_latin1));

	/* test if file is already in names database, else add it */
	if (!names_select_shortname(filename_latin1, shortname, sizeof(shortname)))
	{
		names_insert(filename_latin1);
	}

	/* file must be in names database now */
	if (!names_select_shortname(filename_latin1, shortname, sizeof(shortname)))
	{
		add_log(ADDLOG_FAIL, "[db5]insert", "unable to insert file '%s' into name database\n", filename);
		return false;
	}

	add_log(ADDLOG_DUMP, "[db5]insert", "$filename -> $shortname\n");
	log_dump("filename", filename);
	log_dump_latin1("shortname", shortname);

	/* test if short file is already in dat database */
	if (db5_dat_select_by_filename(filename_latin1) != DB5_ROW_NOT_FOUND)
	{
		add_log(ADDLOG_FAIL, "[db5]insert", "file '%s' already exists\n", filename);
		return false;
	}

	/* generate localfile */
	db5_shortname_to_localfile(shortname, localfile, sizeof(localfile));

	/* generate information */
	if (db5_generate_row(localfile, &row) != true)
	{
		add_log(ADDLOG_FAIL, "[db5]insert", "unable to generate row from file '%s'\n", filename);
		return false;
	}
	db5_widechar_row(&row);

	/* if first char of filename is a dot, flag up the hidden field */
	row.hidden = (uint32_t)(filename[0] == '.');

	/* insert row */
	if (db5_dat_insert(&row) != true)
	{
		add_log(ADDLOG_FAIL, "[db5]insert", "unable to insert row in database '%s'\n", filename);
		return false;
	}

	return true;
}

bool db5_update(const char *filename)
{
	uint32_t row_index;
	db5_row row;
	char shortname[membersizeof(db5_row, filename)];
	char localfile[PATH_MAX];

	check(filename != NULL);

	/* retrieve shortname */
	if (!db5_longname_to_shortname(filename, shortname, sizeof(shortname)))
	{
		add_log(ADDLOG_FAIL, "[db5]update", "unable to get short file name for '%s'\n", filename);
		return false;
	}

	/* retrieve row in dat database */
	row_index = db5_dat_select_by_filename(shortname);
	if (row_index == DB5_ROW_NOT_FOUND)
	{
		add_log(ADDLOG_FAIL, "[db5]update", "unable to find file in database\n");
		log_dump_latin1("shortname", shortname);
		log_dump("filename", filename);
		return false;
	}

	add_log(ADDLOG_DUMP, "[db5]update", "database entry is %u\n", row_index);

	/* retrieve local file */
	db5_shortname_to_localfile(shortname, localfile, sizeof(localfile));

	/* generate information */
	if (!db5_generate_row(localfile, &row))
	{
		add_log(ADDLOG_FAIL, "[db5]update", "unable to generate row from file\n");
		log_dump("localfile", localfile);
		log_dump("filename", filename);
		return false;
	}
	db5_widechar_row(&row);

	/* if first char of filename is a dot, flag up the hidden field */
	row.hidden = (uint32_t)(filename[0] == '.');

	/* update row in dat database */
	if (!db5_dat_update(row_index, &row))
	{
		add_log(ADDLOG_FAIL, "[db5]update", "error writting info in database for file '%s'\n", filename);
		return false;
	}

	return true;
}

bool db5_delete(const char *filename)
{
	uint32_t row_index;
	char shortname[membersizeof(db5_row, filename)];

	check(filename != NULL);

	/* retrieve shortname */
	if (!db5_longname_to_shortname(filename, shortname, sizeof(shortname)))
	{
		add_log(ADDLOG_FAIL, "[db5]delete", "unable to get short file name for '%s'\n", filename);
		return false;
	}

	/* retrieve row in dat database */
	row_index = db5_dat_select_by_filename(shortname);
	if (row_index == DB5_ROW_NOT_FOUND)
	{
		add_log(ADDLOG_FAIL, "[db5]delete", "unable to find file in database\n");
		log_dump_latin1("shortname", shortname);
		log_dump("filename", filename);
		return false;
	}

	if (!db5_dat_delete_row(row_index))
	{
		add_log(ADDLOG_FAIL, "[db5]remove", "unable to delete file from dat database, row is %u\n", row_index);
		log_dump_latin1("shortname", shortname);
		log_dump("filename", filename);
		return false;
	}

	if (names_delete(filename))
	{
		add_log(ADDLOG_RECOVER, "[db5]remove", "unable to remove file '%s' from names database\n", filename);
	}

	return true;
}

bool db5_index()
{
	unsigned int result;

	result = 0;

	if (db5_idx_index_str(filename, DB5_IDX_CODE_FILENAME) == true)
	{
		result++;
	}

	result <<= 1;
	if (db5_idx_index_str(filepath, DB5_IDX_CODE_FILEPATH) == true)
	{
		result++;
	}

	result <<= 1;
	if (db5_idx_index_str(album, DB5_IDX_CODE_ALBUM) == true)
	{
		result++;
	}

	result <<= 1;
	if (db5_idx_index_str(genre, DB5_IDX_CODE_GENRE) == true)
	{
		result++;
	}

	result <<= 1;
	if (db5_idx_index_str(title, DB5_IDX_CODE_TITLE) == true)
	{
		result++;
	}

	result <<= 1;
	if (db5_idx_index_str(artist, DB5_IDX_CODE_ARTIST) == true)
	{
		result++;
	}

	result <<= 1;
	if (db5_idx_index_num(track, DB5_IDX_CODE_TRACK) == true)
	{
		result++;
	}

	result <<= 1;
	if (db5_idx_index_num(source, DB5_IDX_CODE_SOURCE) == true)
	{
		result++;
	}

	return (result == 0xff);
}

bool db5_localfile(const char *filename, char *localfile, const size_t localfile_size)
{
	char shortname[membersizeof(db5_row, filename)];

	check(filename != NULL);
	check(localfile != NULL);
	check(localfile_size > 0);

	if (!db5_longname_to_shortname(filename, shortname, sizeof(shortname)))
	{
		add_log(ADDLOG_USER_ERROR, "[db5]localfile", "unable to get short file name for '%s'\n", filename);
		localfile[0] = '\0';
		return false;
	}

	db5_shortname_to_localfile(shortname, localfile, localfile_size);

	return true;
}

bool db5_longname_to_shortname(const char *longname, char *shortname, const size_t shortname_size)
{
	char longname_latin1[PATH_MAX];

	check(longname != NULL);
	check(shortname != NULL);
	check(shortname_size > 0);

	/* convert to latin 1 */
	utf8_iso8859(longname, longname_latin1, sizeof(longname_latin1));

	/* first case: checksum of longname */
	if (names_generate_shortname(longname_latin1, shortname, shortname_size) != true)
	{
		add_log(ADDLOG_FAIL, "[db5]long_to_short", "cannot generate short filename, longname:'%s'\n", longname);
		return false;
	}
	if (db5_dat_select_by_filename(shortname) != DB5_ROW_NOT_FOUND)
	{
		add_log(ADDLOG_DUMP, "[db5]long_to_short", "$longname -> $shortname (checksum)\n");
		log_dump("longname", longname);
		log_dump_latin1("shortname", shortname);
		return true;
	}

	/* second case: unmodified name */
	strncpy(shortname, longname_latin1, shortname_size);
	if (db5_dat_select_by_filename(shortname) != DB5_ROW_NOT_FOUND)
	{
		add_log(ADDLOG_DUMP, "[db5]long_to_short", "$longname -> $shortname (same name)\n");
		log_dump("longname", longname);
		log_dump_latin1("shortname", shortname);
		return true;
	}

	/* third case: found in database */
	if (names_select_shortname(longname_latin1, shortname, shortname_size) == true)
	{
		if (db5_dat_select_by_filename(shortname) != DB5_ROW_NOT_FOUND)
		{
			add_log(ADDLOG_DUMP, "[db5]long_to_short", "$longname -> $shortname (database)\n");
			log_dump("longname", longname);
			log_dump_latin1("shortname", shortname);
			return true;
		}
	}

	/* no result */
	shortname[0] = '\0';
	add_log(ADDLOG_USER_ERROR, "[db5]long_to_short", "file '%s' not found\n", longname);
	return false;
}

bool db5_shortname_to_localfile(const char *shortname, char *localfile, const size_t localfile_size)
{
	char localfile_latin1[PATH_MAX];

	check(shortname != NULL);
	check(localfile != NULL);
	check(localfile_size > 0);

	if (snprintf(localfile_latin1, sizeof(localfile_latin1), "%s/%s", CONFIG_MUSIC_PATH, shortname) >= localfile_size)
	{
		/* not enough size */
		add_log(ADDLOG_FAIL, "[db5]short_to_local", "not enough size to write result in localfile, sizeof(localfile):%u\n", localfile_size);
		return false;
	}

	iso8859_utf8(localfile_latin1, localfile, localfile_size);

	add_log(ADDLOG_DUMP, "[db5]short_to_local", "$shortname -> $localfile\n");
	log_dump_latin1("shortname", shortname);
	log_dump("localfile", localfile);

	return true;
}

bool db5_exists(const char *filename)
{
	char shortname[membersizeof(db5_row, filename)];

	check(filename != NULL);

	add_log(ADDLOG_DEBUG, "[db5]exists", "called, filename:'%s'\n", filename);

	/* try to retrieve shortname */
	if (db5_longname_to_shortname(filename, shortname, sizeof(shortname)))
	{
		add_log(ADDLOG_DEBUG, "[db5]exists", "returns true\n");
		return true;
	}

	add_log(ADDLOG_DEBUG, "[db5]exists", "returns false\n");
	return false;
}

uint32_t db5_count()
{
	return db5_hdr_count();
}

void db5_print_row(db5_row *row)
{
	check(row != NULL);

	log_dump_latin1("(longname)", names_select_longname(row->filename));
	log_dump_latin1("dir", row->filepath);
	log_dump_latin1("file", row->filename);
	add_log(ADDLOG_DUMP, "[db5]print_row", "bitrate: %d bit/s\n", row->bitrate);
	add_log(ADDLOG_DUMP, "[db5]print_row", "samplerate: %d Hz\n", row->samplerate);
	add_log(ADDLOG_DUMP, "[db5]print_row", "duration: %d s\n", row->duration);
	log_dump_latin1("artist", row->artist);
	log_dump_latin1("album", row->album);
	log_dump_latin1("genere", row->genre);
	log_dump_latin1("title", row->title);
	add_log(ADDLOG_DUMP, "[db5]print_row", "track: %d\n", row->track);
	add_log(ADDLOG_DUMP, "[db5]print_row", "year: %d\n", row->year);
	add_log(ADDLOG_DUMP, "[db5]print_row", "filesize: %d\n", row->filesize);
	add_log(ADDLOG_DUMP, "[db5]print_row", "source: %d\n", row->source);
}

