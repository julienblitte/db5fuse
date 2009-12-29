/**
 * @file file.c
 * @brief Source - File operations
 * @author Julien Blitte
 * @version 0.1
 */
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "check.h"
#include "logger.h"

/** @brief common buffer used for file operation */
char file_common_buffer[10240];

off_t file_filesize(const char *filename)
{
	struct stat s;

	check(filename != NULL);

	if (stat(filename, &s) != 0)
	{
		add_log(ADDLOG_FAIL, "[file]filesize", "unable to get stat information: %s\n", strerror(errno));
		log_dump("filename", filename);
		return 0;
	}

	return s.st_size;
}

off_t file_filesize_f(FILE *file)
{
	struct stat filestat;

	check(file != NULL);

	if (fstat(fileno(file), &filestat) != 0)
	{
		add_log(ADDLOG_FAIL, "[file]filesize_f", "unable to get stat information: %s\n", strerror(errno));
		return 0;
	}

	return filestat.st_size;
}

void file_path_explode(char *path, char **dir, char **file, char **ext)
{
	check(path != NULL);
	check(dir != NULL);
	check(file != NULL);
	check(ext != NULL);

	*file = strrchr(path, '/');
	if (*file == NULL)
	{
		*dir = NULL;
		*file = path;
	}
	else
	{
		*dir = path;
		**file = '\0';
		(*file)++;
	}

	*ext = strrchr(*file, '.');
	if (*ext != NULL)
	{
		**ext = '\0';
		(*ext)++;
	}
}

bool file_set_context(const char *device)
{
	check(device != NULL);

	if (chdir(device) != 0)
	{
		add_log(ADDLOG_FAIL, "[file]set_context", "unable to work in directory\n");
		log_dump("device", device);
		return false;
	}

	return true;
}

bool file_exists(const char *path)
{
	check(path != NULL);

	return (access(path, F_OK) == 0 ? true : false);
}

const char *file_remove_headslash(const char *path)
{
	check(path != NULL);

	while(*path == '/')
	{
		path++;
	}

	return path;
}

const char *file_get_extension(const char *path)
{
	const char *result;

	check(path != NULL);

	/* return strchrnul(path, '.'); */

	result = strrchr(path, '.');
	if (result == NULL)
	{
		result = "";
	}

	return result;
}

void file_windows_slashes(char *path)
{
	check(path != NULL);

	while(*path != '\0')
	{
		if (*path == '/')
		{
			*path = '\\';
		}
		path++;
	}
}

FILE *file_fcaseopen(const char *directory, const char *filename, const char *mode)
{
	DIR *dir;
	struct dirent *entry;
	char filepath[PATH_MAX];

	check(directory != NULL);
	check(filename != NULL);

	dir = opendir(directory);
	if (dir == NULL)
	{
		add_log(ADDLOG_FAIL, "[file]fcaseopen", "unable to open directory\n");
		log_dump("directory", directory);
		log_dump("filename", filename);
		return NULL;
	}

	entry = readdir(dir);
	while(entry != NULL)
	{
		if (strcasecmp(filename, entry->d_name) == 0)
		{
			closedir(dir);
			add_log(ADDLOG_DUMP, "[file]fcaseopen", "resolved\n");
			log_dump("filename", filename);
			log_dump("entry", entry->d_name);

			snprintf(filepath, sizeof(filepath), "%s/%s", directory, entry->d_name);
			return fopen(filepath, mode);
		}
		entry = readdir(dir);
	}
	closedir(dir);

	add_log(ADDLOG_RECOVER, "[file]fcaseopen", "unable to find file in directory\n");
	log_dump("filename", filename);
	log_dump("directory", directory);

	snprintf(filepath, sizeof(filepath), "%s/%s", directory, filename);
	return fopen(filepath, mode);
}

bool file_truncate(FILE *file, off_t len)
{
	int fd;

	check(file != NULL);

	fd = fileno(file);
	if (fd == -1)
	{
		add_log(ADDLOG_FAIL, "[file]truncate", "unable to get descriptor of stream 0x%p: %s\n", file, strerror(errno));
		return false;
	}
	if (ftruncate(fd, len) != 0)
	{
		add_log(ADDLOG_FAIL, "[file]truncate", "unable to truncate file: %s\n", strerror(errno));
		return false;
	}

	return true;
}

