/**
 * @file names.c
 * @brief Source - Database db5, long filename support
 * @author Julien Blitte
 * @version 0.1
 */
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "check.h"
#include "config.h"
#include "crc32.h"
#include "db5_types.h"
#include "file.h"
#include "logger.h"
#include "names.h"
#include "utf8.h"

/**
 * @brief node of name translation linked list
 */
typedef struct name_trans_t
{
	/** @brief hash of original name */
	uint32_t crc32;
	/** @brief original name - latin1 */
	char *longname;
	/** @brief link to next entry */
	struct name_trans_t *next;
} name_trans;

/** @brief linked list of name translation */
static name_trans *head, *tail;

/**
 * @brief insert a name translation in linked list
 * @param crc32 checksum of filename
 * @param long filename - latin1
 */
static void names_insert_full(const uint32_t crc32, const char *filename)
{
	size_t length;

	check(crc32 != 0);
	check(filename != NULL);

	if (head == NULL)
	{
		head = malloc(sizeof(name_trans));
		check(head != NULL);

		tail = head;
	}
	else
	{
		tail->next = malloc(sizeof(name_trans));
		check(tail->next != NULL);

		tail = tail->next;
	}

	length = strlen(filename);

	tail->longname = malloc(length+1);
	check(tail->longname != NULL);
	strncpy(tail->longname, filename, length+1);

	tail->crc32 = crc32;
	tail->next = NULL;
}

bool names_select_shortname(const char *filename, char *shortname, const size_t shortname_size)
{
	name_trans *current;
	const char *ext;

	check(filename != NULL);
	check(shortname != NULL);
	check(shortname_size > 0);

	if (shortname_size != 0)
	{
		shortname[0] = '\0';
	}

	ext = file_get_extension(filename);
	if (ext == NULL)
	{
		add_log(ADDLOG_USER_ERROR, "[names]select_short", "unable to get extension");
		log_dump_latin1("filename", filename);
		return false;
	}

	current = head;
	while(current != NULL)
	{
		if (strcmp(filename, current->longname) == 0)
		{
			/* generate filename using crc32 and original extension */
			if (snprintf(shortname, shortname_size, "%x%s", current->crc32, ext) >= shortname_size)
			{
				return false;
			}
			return true;
		}
		current = current->next;
	}

	return false;
}

/**
 * @brief retrieve complete name by a crc
 * @param crc32 checksum of filename
 * @return long filename - latin1
 */
static const char *names_select_by_crc(const uint32_t crc32)
{
	name_trans *current;

	check(crc32 != 0);

	current = head;
	while(current != NULL)
	{
		if (current->crc32 == crc32)
		{
			return current->longname;
		}
		current = current->next;
	}
	return NULL;
}

bool names_init()
{
	char shortname[PATH_MAX], longname[PATH_MAX];
	FILE *names;
	int length;
	uint32_t crc32;

	crc32_init();

	head = NULL;
	tail = NULL;

	names = file_fcaseopen(".", CONFIG_NAMES_FILE, "rt");
	if (names == NULL)
	{
		add_log(ADDLOG_FAIL, "[names]init", "unable to load database\n");
	}
	else
	{
		/* recycle variable 'shortname' */
		while(!feof(names))
		{
			if (fgets(shortname, sizeof(shortname), names) && fgets(longname, sizeof(longname), names))
			{
				for(length = 0; longname[length] != '\0'; length++)
				{
					if ((longname[length] == '\r') || (longname[length] == '\n'))
					{
						longname[length] = '\0';
						break;
					}
				}
				crc32 = strtoul(shortname, NULL, 16);

				names_insert_full(crc32, longname);
			}
		}
		fclose(names);
	}

	if (head == NULL)
	{
		add_log(ADDLOG_NOTICE, "[names]init", "name database is empty\n");
	}
	
	return true;
}

void names_insert(const char *filename)
{
	uint32_t crc32;

	check(filename != NULL);

	crc32 = strcrc32(filename);

	names_insert_full(crc32, filename);

	if (!names_save())
	{
		add_log(ADDLOG_RECOVER, "[names]insert", "error while saving names list\n");
		log_dump_latin1("filename", filename);
	}
}

const char *names_select_longname(const char *shortname)
{
	const char *result;
	uint32_t crc32;

	check(shortname != NULL);

	/* retrieve crc32 in the name */
	crc32 = strtoul(shortname, NULL, 16);
	if (crc32 == 0)
	{
		return shortname;
	}

	/* locate the original long filename */
	result = names_select_by_crc(crc32);
	if (result == NULL)
	{
		return shortname;
	}

	return result;
}

bool names_generate_shortname(const char *longname, char *shortname, const size_t shortname_size)
{
	const char *ext;
	uint32_t crc32;

	check(longname != NULL);
	check(shortname != NULL);
	check(shortname_size > 0);

	ext = file_get_extension(longname);

	/* compute crc32 of the filename */
	crc32 = strcrc32(longname);

	/* generate filename using crc32 and original extension */
	if (snprintf(shortname, shortname_size, "%x%s", crc32, ext) >= shortname_size)
	{
		return false;
	}

	return true;
}

bool names_save()
{
	char *ext;
	FILE *names;
	name_trans *current;

	current = head;

	names = file_fcaseopen(".", CONFIG_NAMES_FILE, "wb");
	if (names == NULL)
	{
		add_log(ADDLOG_FAIL, "[names]save", "unable to save database\n");
		return false;
	}

	while(current != NULL)
	{
		check(current->longname != NULL);
		ext = strrchr(current->longname, '.');

		fprintf(names, "%x%s\r\n%s\r\n", current->crc32, ext, current->longname);

		current = current->next;
	}

	fclose(names);

	return true;
}

bool names_delete(const char *filename)
{
	name_trans *previous;
	name_trans *current;

	check(filename != NULL);

	previous = NULL;
	current = head;
	while(current != NULL)
	{
		if (strcmp(filename, current->longname) == 0)
		{
			if (previous != NULL)
			{
				previous->next = current->next;
				free(current);
			}
			else
			{
				head = current->next;
				free(current);
			}

			if (!names_save())
			{
				add_log(ADDLOG_RECOVER, "[names]insert", "error while saving names list\n");
			}
			return true;
		}
		previous = current, current = current->next;
	}
	return false;
}

void names_print()
{
	name_trans *current;

	current = head;
	while(current != NULL)
	{
		add_log(ADDLOG_DUMP, "[names]print", "checksum %08x\n", current->crc32);
		log_dump_latin1("current->filename", current->longname);
		current = current->next;
	}
}

void names_free()
{
	name_trans *current;

	while(head != NULL)
	{
		current = head;
		head = head->next;
		free(current);
	}
}

