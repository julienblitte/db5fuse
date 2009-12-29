#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "check.h"
#include "config.h"
#include "db5_dat.h"
#include "db5.h"
#include "db5_hdr.h"
#include "db5_types.h"
#include "file.h"
#include "utf8.h"
#include "wstring.h"

void fsck_init(const char *fuse_device);
void fsck_check(const bool fix);
bool fsck_check_step1(const bool fix);
bool fsck_check_step2(const bool fix);
bool fsck_check_step3(const bool fix);
bool fsck_check_step4(const bool fix);
bool fsck_check_step5(const bool fix);
void fsck_free();

static uint32_t real_count;
static db5_row row;

void fsck_init(const char *fuse_device)
{
	check(fuse_device != NULL);

	if (file_set_context(fuse_device) != true)
	{
		/* do not use LOG_CRIT because it is syslog, not local log */
		fprintf(stderr, "fsck.db5: fatal, unable to reach device '%s'\n", fuse_device);
		exit(EXIT_FAILURE);
	}

	open_log();
	add_log(ADDLOG_NOTICE, "[fsck]init", "fsck.db5 will scan device '%s'\n", fuse_device);

	if (db5_init() != true)
	{
		add_log(ADDLOG_CRITICAL, "[fsck]init", "unable to initialize filesystem\n");
		exit(EXIT_FAILURE);
	}
}

void fsck_free()
{
	add_log(ADDLOG_NOTICE, "[fsck]free", "scan complete\n");
	add_log(ADDLOG_DEBUG, "[fsck]free", "exiting program\n");

	db5_free();
	close_log();
}

void fsck_check(const bool fix)
{
	add_log(ADDLOG_NOTICE, "[fsck]check", "filesystem will be checked\n");

	add_log(ADDLOG_OPERATION, "[fsck]check", "step1: check number of files\n");
	if (fsck_check_step1(fix))
	{
		add_log(ADDLOG_NOTICE, "[fsck]check", "step1 ok\n");
	}
	else
	{
		add_log(ADDLOG_CRITICAL, "[fsck]check", "error during step1\n");
		return;
	}

	add_log(ADDLOG_OPERATION, "[fsck]check", "step2: check music directory\n");
	if (fsck_check_step2(fix))
	{
		add_log(ADDLOG_NOTICE, "[fsck]check", "step2 ok\n");
	}
	else
	{
		add_log(ADDLOG_CRITICAL, "[fsck]check", "error during step2\n");
		return;
	}

	add_log(ADDLOG_OPERATION, "[fsck]check", "step3: check file file existence and refresh information\n");
	if (fsck_check_step3(fix))
	{
		add_log(ADDLOG_NOTICE, "[fsck]check", "step3 ok\n");
	}
	else
	{
		add_log(ADDLOG_CRITICAL, "[fsck]check", "error during step3\n");
		return;
	}

	add_log(ADDLOG_OPERATION, "[fsck]check", "step4: searching orphan files\n");
	if (fsck_check_step4(fix))
	{
		add_log(ADDLOG_NOTICE, "[fsck]check", "step4 ok\n");
	}
	else
	{
		add_log(ADDLOG_CRITICAL, "[fsck]check", "error during step4\n");
		return;
	}

	add_log(ADDLOG_OPERATION, "[fsck]check", "step5: re-generating index\n");
	if (fsck_check_step5(fix))
	{
		add_log(ADDLOG_NOTICE, "[fsck]check", "step5 ok\n");
	}
	else
	{
		add_log(ADDLOG_CRITICAL, "[fsck]check", "error during step5\n");
		return;
	}
}

bool fsck_check_step1(const bool fix)
{
	uint32_t local_count, i;
	
	/* check number of files */
	local_count = db5_hdr_count();
	add_log(ADDLOG_DEBUG, "[fsck]step1", "%u files registered\n", local_count);

	i=0;
	real_count = CONFIG_MAX_DB5_ENTRY;
	while(i < CONFIG_MAX_DB5_ENTRY)
	{
		if (!db5_dat_select_row(i, &row))
		{
			real_count = (i == 0 ? 0 : i-1);
			break;
		}
		i++;
	}
	add_log(ADDLOG_DEBUG, "[fsck]step1", "%u files are detected\n", real_count);

	if (real_count != local_count)
	{
		if (fix)
		{
			add_log(ADDLOG_FAIL, "[fsck]step1", "correct the number of files to %u\n", real_count);
			db5_hdr_grow(real_count-local_count);
		}
		else
		{
			add_log(ADDLOG_FAIL, "[fsck]step1", "number of files should be %u\n", real_count);
		}
	}

	return true;
}

bool fsck_check_step2(const bool fix)
{
	struct stat dirstat;

	/* check if music directory exists */
	if (stat(CONFIG_MUSIC_PATH, &dirstat) != 0)
	{
		if (fix)
		{
			add_log(ADDLOG_FAIL, "[fsck]step2", "creates directory '%s'\n", CONFIG_MUSIC_PATH);
			if (mkdir(CONFIG_MUSIC_PATH, 0755) != 0)
			{
				add_log(ADDLOG_CRITICAL, "[fsck]step2", "unable to create directory '%s'\n", CONFIG_MUSIC_PATH);
				return false;
			}
		}
		else
		{
			add_log(ADDLOG_CRITICAL, "[fsck]step2", "directory '%s' should exists\n", CONFIG_MUSIC_PATH);
			return false;
		}
	}
	else
	{
		if (!S_ISDIR(dirstat.st_mode))
		{
			add_log(ADDLOG_CRITICAL, "[fsck]step2", "'%s' is not a directory\n", CONFIG_MUSIC_PATH);
			return false;
		}
		else
		{
			add_log(ADDLOG_DEBUG, "[fsck]step2", "directory '%s' found\n", CONFIG_MUSIC_PATH);
		}
	}

	return true;
}

bool fsck_check_step3(const bool fix)
{
	int fd;
	char localfile[PATH_MAX];
	uint32_t i;

	/* check if all files exists and refresh file infos */
	for(i=0; i < real_count; i++)
	{
		if (db5_dat_select_row(i, &row) != true)
		{
			add_log(ADDLOG_FAIL, "[fsck]step3", "unable to get file information form database, entry id: %u\n", i);
			return NULL;
		}
		ws_wstoa(row.filename, membersizeof(db5_row, filename));

		db5_shortname_to_localfile(row.filename, localfile, sizeof(localfile));

		if (!file_exists(localfile))
		{
			add_log(ADDLOG_FAIL, "[fsck]step3", "local file of entry %u does not exists\n", i);
			log_dump_latin1("shortname", row.filename);
			log_dump("localfile", localfile);

			if (fix)
			{
				fd = creat(localfile, 0644);
				if (fd == -1)
				{
					add_log(ADDLOG_FAIL, "[fsck]step3", "unable to recreate file '%s'\n", localfile);
				}
				else
				{
					add_log(ADDLOG_NOTICE, "[fsck]step3", "file '%s' recreated\n", localfile);
					close(fd);
				}
			}
		}

		if (fix)
		{
			db5_generate_row(localfile, &row);
			db5_widechar_row(&row);
			db5_dat_update(i, &row);
		}
	}

	return true;
}

bool fsck_check_step4(const bool fix)
{
	DIR *music;
	struct dirent *entry;
	char filename_latin1[PATH_MAX];
	char namebuffer[PATH_MAX];
	char *dir, *file, *ext;
	db5_row row;

	music = opendir(CONFIG_MUSIC_PATH);
	if (music == NULL)
	{
		add_log(ADDLOG_FAIL, "[fsck]step4", "unable to open music directory\n");
		return false;
	}

	entry = readdir(music);
	while(entry != NULL)
	{
		utf8_iso8859(entry->d_name, filename_latin1, sizeof(filename_latin1));
		strncpy(namebuffer, filename_latin1, sizeof(namebuffer));
		file_path_explode(namebuffer, &dir, &file, &ext);

		if (ext == NULL)
		{
			ext = "";
		}

		/* file have a correct extension */
		if (strcasecmp(ext, CONFIG_ASF_EXT) == 0 || strcasecmp(ext, CONFIG_MPEG_EXT) == 0)
		{
			/* check that the filename is enougth short to be in database */
			if (strlen(filename_latin1) <= (membersizeof(db5_row, filename)/2))
			{
				if (db5_dat_select_by_filename(filename_latin1) == DB5_ROW_NOT_FOUND)
				{
					add_log(ADDLOG_FAIL, "[fsck]step4", "file '%s' is not in database\n", entry->d_name);

					if (fix)
					{
						/* generate information */
						db5_shortname_to_localfile(filename_latin1, namebuffer, sizeof(namebuffer));
						if (db5_generate_row(namebuffer, &row) != true)
						{
							add_log(ADDLOG_FAIL, "[fsck]step4", "unable to generate row from file '%s'\n", namebuffer);
							return false;
						}
						db5_widechar_row(&row);

						/* if first char of filename is a dot, flag up the hidden field */
						row.hidden = (uint32_t)(filename_latin1[0] == '.');

						/* insert row */
						if (db5_dat_insert(&row) != true)
						{
							add_log(ADDLOG_FAIL, "[fsck]step4", "unable to insert row in database '%s'\n", namebuffer);
							return false;
						}
						else
						{
							add_log(ADDLOG_NOTICE, "[fsck]step4", "file '%s' added\n", namebuffer);
						}
					}
				}
			}
		}

		entry = readdir(music);
	}
	closedir(music);
	return true;
}

bool fsck_check_step5(const bool fix)
{
	if (fix)
	{
		return db5_index();
	}
	else
	{
		add_log(ADDLOG_DEBUG, "[fsck]step5", "read-only, step skipped\n");
		return true;
	}
}

void usage()
{
	fprintf(stderr, "usage: fsck.db5 [-f] <device>\n\n");
	fprintf(stderr, "  -f      use this option to fix errors.\n");
	fprintf(stderr, "          Default behavior is only to list errors without fixing.\n");
	fprintf(stderr, "  device  the path of db5 device\n\n");
	fprintf(stderr, "check a db5fuse filesystem to fix it.\n\n");

	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
	const char *device;
	bool fix;

	if (argc == 2)
	{
		device = argv[1];
		fix = false;
	}
	else if (argc == 3)
	{
		if (strcmp(argv[1], "-f") == 0)
		{
			device = argv[2];
			fix = true;
		}
		else if (strcmp(argv[2], "-f") == 0)
		{
			device = argv[1];
			fix = true;
		}
		else
		{
			usage();
		}
	}
	else
	{
		usage();
	}

	printf("Scan of '%s'.", device);
	printf("Informations will be stored in file '%s/%s'\n", device, CONFIG_LOG_FILENAME);

	fsck_init(device);
	fsck_check(fix);
	fsck_free();

	printf("done.\n");

	return EXIT_SUCCESS;
}

