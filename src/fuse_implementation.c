/**
 * @file fuse_implementation.c
 * @brief Source - Filesystem, implementation
 * @author Julien Blitte
 * @version 0.1
 */
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <syslog.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "check.h"
#include "db5.h"
#include "file.h"
#include "logger.h"
#include "utf8.h"



/*

Exhaustive function returned values list:
-----------------------------------------
Not enought rights         -EPERM
File not found             -ENOENT
Disk or IO error           -EIO
Bad inode                  -EBADF
Not enought memory         -ENOMEM
Process have to rights     -EACCES
File already in use        -EBUSY
File exists                -EEXIST
Invalid argument           -EINVAL
File too large             -EFBIG
No space left on device    -ENOSPC

Success                    -ESUCCESS

*/


/** @brief operation is ok */
#define ESUCCESS	0


/** @brief Device to mount */
char *fuse_device;

/** @brief Datetime of mount */
static time_t fuse_mount_date;

/** @brief Real name of current file */
static char fuse_localfile[PATH_MAX];
/** @brief Last error, get from errno */
static int fuse_error;

/* unmount fuse file system on error */
static void fuse_impl_exit()
{
	check(fuse_get_context() != NULL);

	fuse_device = NULL;
	fuse_exit(fuse_get_context()->fuse);
}

/* initialize filesystem */
void *fuse_impl_init (struct fuse_conn_info *fs_attr)
{
	check(fuse_device != NULL);

	fuse_mount_date = time(NULL);
	
	if (file_set_context(fuse_device) != true)
	{
		/* do not use LOG_CRIT because it is syslog, not local log */
		syslog(LOG_ERR, "[fuse]init: fatal, unable to reach device '%s'\n", fuse_device);
		fuse_impl_exit();
	}

	open_log();
	add_log(ADDLOG_DEBUG, "[fuse]init", "version compiled the %s at %s\n", __DATE__, __TIME__);
	add_log(ADDLOG_OPERATION, "[fuse]init", "initialization, device is '%s'\n", fuse_device);

	if (db5_init() != true)
	{
		add_log(ADDLOG_CRITICAL, "[fuse]init", "unable to initialize filesystem\n");
		fuse_impl_exit();
	}

	add_log(ADDLOG_OP_SUCCESS, "[fuse]init", "done.\n");

	return NULL;
}

/* clean up filesystem */
void fuse_impl_destroy (void *data)
{
	if (fuse_device != NULL)
	{
		add_log(ADDLOG_OPERATION, "[fuse]destroy", "building indexes\n");
		db5_index();
	}

	add_log(ADDLOG_OPERATION, "[fuse]destroy", "exiting filesystem\n");
	db5_free();

	add_log(ADDLOG_DEBUG, "[fuse]destroy", "good bye!\n");
	close_log();
}

/* create and open a file */
int fuse_impl_create (const char *path, mode_t mode, struct fuse_file_info *filedata)
{
	check(path != NULL);
	check(filedata != NULL);

	(void) mode;

	add_log(ADDLOG_OPERATION, "[fuse]create", "called, args='%s',0%o\n", path, (unsigned int)mode);

	/* test if file already exists */
	if (db5_exists(file_remove_headslash(path)))
	{
		add_log(ADDLOG_USER_ERROR, "[fuse]create", "file '%s' already exists\n", path);
		/* file already exists */
		return -EEXIST;
	}

	/* insert file */
	if (!db5_insert(file_remove_headslash(path)))
	{
		add_log(ADDLOG_FAIL, "[fuse]create", "unable to insert file '%s' in database\n", path);
		/* filesystem error */
		return -EIO;
	}

	/* retrieve local file */
	if (!db5_localfile(file_remove_headslash(path), fuse_localfile, sizeof(fuse_localfile)))
	{
		add_log(ADDLOG_FAIL, "[fuse]create", "unable to retrieve local file of '%s'\n", path);
		/* filesystem error */
		return -EIO;
	}

	add_log(ADDLOG_DUMP, "[fuse]create", "$path -> $localfile\n", path, fuse_localfile);
	log_dump("path", path);
	log_dump("localfile", fuse_localfile);

	/* open file */
	filedata->fh = open(fuse_localfile, filedata->flags, 0644);
	if (filedata->fh == -1)
	{
		fuse_error = errno;
		add_log(ADDLOG_FAIL, "[fuse]open", "open fail: '%s'\n", strerror(fuse_error));
		/* io error */
		return -fuse_error;
	}

	add_log(ADDLOG_OP_SUCCESS, "[fuse]create", "done.\n");

	/* success */
	return -ESUCCESS;
}

/* change the access and modification times */
int fuse_impl_utimens (const char *path, const struct timespec tv[2])
{
	struct utimbuf time;

	check(path != NULL);

	add_log(ADDLOG_OPERATION, "[fuse]utimens", "called, args='%s',(%u,%u)\n", path, tv[0], tv[1]);

	if (db5_localfile(file_remove_headslash(path), fuse_localfile, sizeof(fuse_localfile)) != true)
	{
		add_log(ADDLOG_USER_ERROR, "[fuse]utimens", "file '%s' does not exists\n", fuse_localfile);
		/* file does not exists */
		return -ENOENT;
	}

	time.actime = tv[0].tv_sec;
	time.modtime = tv[1].tv_sec;

	if (utime(fuse_localfile, &time) != 0)
	{
		fuse_error = errno;
		add_log(ADDLOG_FAIL, "[fuse]utimens", "unable to set access/modification time\n");
		log_dump("path", path);
		log_dump("localfile", fuse_localfile);

		add_log(ADDLOG_FAIL, "[fuse]utimens", "utime: 0x%x '%s'\n", fuse_error, strerror(fuse_error));
		/* io error */
		return -fuse_error;
	}

	add_log(ADDLOG_OP_SUCCESS, "[fuse]utimens", "done.\n");

	/* success */
	return -ESUCCESS;
}

/* file attributes */
int fuse_impl_getattr(const char *path, struct stat *attr)
{
	struct stat localattr;

	check(path != NULL);
	check(attr != NULL);
	check(fuse_get_context() != NULL);

	add_log(ADDLOG_OPERATION, "[fuse]getattr", "called, args='%s'\n", path);

	memset(attr, 0, sizeof(attr));

	/* root path */
	if(strcmp(path, "/") == 0)
	{
		/* mode = 0755 */
		attr->st_mode = S_IFDIR | 0755;
		/* hard link */
		attr->st_nlink = 2;
		/* file size */
		attr->st_size = db5_count();
		/* access, modifiation and creation time */
		attr->st_atime = fuse_mount_date;
		attr->st_mtime = fuse_mount_date;
		attr->st_ctime = fuse_mount_date;
		/* current user and group */
		attr->st_uid = fuse_get_context()->uid;
		attr->st_gid = fuse_get_context()->gid;

		add_log(ADDLOG_OP_SUCCESS, "[fuse]getattr", "done.\n");

		/* success */
		return -ESUCCESS;
	}

	attr->st_ino = 0;

	if (!db5_localfile(file_remove_headslash(path), fuse_localfile, sizeof(fuse_localfile)))
	{
		add_log(ADDLOG_USER_ERROR, "[fuse]getattr", "unable to find local file for '%s'\n", path);
		/* file does not exists */
		return -ENOENT;
	}

	if (stat(fuse_localfile, &localattr) != 0)
	{
		fuse_error = errno;
		add_log(ADDLOG_FAIL, "[fuse]getattr", "unable to get information from local file: %s\n", strerror(fuse_error));
		log_dump("localfile", fuse_localfile);
		log_dump("path", path);
		/* io error */
		return -fuse_error;
	}

	/* file size */
	attr->st_size = localattr.st_size;
	/* access, modifiation and creation time */
	attr->st_atime = localattr.st_atime;
	attr->st_mtime = localattr.st_mtime;
	attr->st_ctime = localattr.st_ctime;
	/* used blocks */
	attr->st_blocks = localattr.st_blocks;
	
	attr->st_mode = S_IFREG | 0644;               /* mode = 0644       */
	attr->st_nlink = 1;                           /* hard link         */
	/* current user and group */
	attr->st_uid = fuse_get_context()->uid;
	attr->st_gid = fuse_get_context()->gid;
	
	add_log(ADDLOG_OP_SUCCESS, "[fuse]getattr", "done.\n");

	/* success */
	return -ESUCCESS;
}

/* remove a file */
int fuse_impl_unlink (const char *path)
{
	check(path != NULL);

	add_log(ADDLOG_OPERATION, "[fuse]unlink", "called, args='%s'\n", path);

	if (!db5_localfile(file_remove_headslash(path), fuse_localfile, sizeof(fuse_localfile)))
	{
		add_log(ADDLOG_USER_ERROR, "[fuse]unlink", "unable to find file '%s'\n", path);
		/* file does not exists */
		return -ENOENT;
	}

	if (!db5_delete(file_remove_headslash(path)))
	{
		add_log(ADDLOG_FAIL, "[fuse]unlink", "unable to remove file '%s' from database\n", path);
		/* filesystem error */
		return -EIO;
	}

	if (unlink(fuse_localfile) != 0)
	{
		add_log(ADDLOG_RECOVER, "[fuse]unlink", "unable to remove local file: %s\n", strerror(fuse_error));
		log_dump("localfile", fuse_localfile);
		log_dump("path", path);
	}

	add_log(ADDLOG_OP_SUCCESS, "[fuse]unlink", "done.\n");

	/* success */
	return -ESUCCESS;
}

/* rename a file */
int fuse_impl_rename (const char *path, const char *newname)
{
	char localfile_new[PATH_MAX];

	check(path != NULL);
	check(newname != NULL);

	add_log(ADDLOG_OPERATION, "[fuse]rename", "called, args='%s' -> '%s'\n", path, newname);

	if (!db5_exists(file_remove_headslash(path)))
	{
		add_log(ADDLOG_USER_ERROR, "[fuse]rename", "source file '%s' does not exists\n", path);
		/* file does not exists */
		return -ENOENT;
	}

	if (db5_exists(file_remove_headslash(newname)))
	{
		add_log(ADDLOG_USER_ERROR, "[fuse]rename", "destination file '%s' already exists\n", newname);
		/* file already exists */
		return -EEXIST;
	}

	/* path localname */
	if (!db5_localfile(file_remove_headslash(path), fuse_localfile, sizeof(fuse_localfile)))
	{
		add_log(ADDLOG_FAIL, "[fuse]rename", "unable to locate local file of '%s'\n", path);
		/* filesystem error */
		return -EIO;
	}

	/* insert new file in database */
	if (!db5_insert(file_remove_headslash(newname)))
	{
		add_log(ADDLOG_FAIL, "[fuse]rename", "unable to insert '%s' in database\n", newname);
		/* filesystem error */
		return -EIO;
	}

	/* retrieve name of new entry */
	if (!db5_localfile(file_remove_headslash(newname), localfile_new, sizeof(localfile_new)))
	{
		add_log(ADDLOG_FAIL, "[fuse]rename", "unable to locate local file of '%s'\n", newname);
		/* filesystem error */
		return -EIO;
	}

	add_log(ADDLOG_DEBUG, "[fuse]rename", "renaming local file\n");
	log_dump("source", fuse_localfile);
	log_dump("destination", localfile_new);

	/* rename old file */
	if (rename(fuse_localfile, localfile_new) != 0)
	{
		fuse_error = errno;
		add_log(ADDLOG_FAIL, "[fuse]rename", "unable to rename local file: %s\n", strerror(fuse_error));
		log_dump("source", fuse_localfile);
		log_dump("destination", localfile_new);
		/* io error */
		return -fuse_error;
	}

	/* remove source from database */
	if (!db5_delete(file_remove_headslash(path)))
	{
		add_log(ADDLOG_FAIL, "[fuse]rename", "unable to remove '%s' in database\n", path);
		/* filesystem error */
		return -EIO;
	}

	/* update database entry */
	db5_update(file_remove_headslash(newname));

	add_log(ADDLOG_OP_SUCCESS, "[fuse]rename", "done.\n");

	/* success */
	return -ESUCCESS;
}

/* change the size of a file */
int fuse_impl_truncate (const char *path, off_t newsize)
{
	check(path != NULL);

	add_log(ADDLOG_OPERATION, "[fuse]truncate", "called, args='%s', %u\n", path, newsize);

	if (!db5_localfile(file_remove_headslash(path), fuse_localfile, sizeof(fuse_localfile)))
	{
		add_log(ADDLOG_USER_ERROR, "[fuse]truncate", "unable to find file '%s'\n", path);
		/* file does not exists */
		return -ENOENT;
	}

	if (truncate(fuse_localfile, newsize) != 0)
	{
		fuse_error = errno;
		add_log(ADDLOG_FAIL, "[fuse]truncate", "unable to truncate local file: %s\n", strerror(fuse_error));
		log_dump("localfile", fuse_localfile);
		log_dump("path", path);
		/* io error */
		return -fuse_error;
	}

	add_log(ADDLOG_OP_SUCCESS, "[fuse]truncate", "done.\n");

	/* success */
	return -ESUCCESS;
}

/* open a file */
int fuse_impl_open(const char *path, struct fuse_file_info *filedata)
{
	check(path != NULL);
	check(filedata != NULL);

	add_log(ADDLOG_OPERATION, "[fuse]open", "called, args='%s'\n", path);

	if (db5_localfile(file_remove_headslash(path), fuse_localfile, sizeof(fuse_localfile)) != true)
	{
		add_log(ADDLOG_USER_ERROR, "[fuse]open", "unable to find file '%s'\n", path);
		/* file does not exists */
		return -ENOENT;
	}


	filedata->fh = open(fuse_localfile, filedata->flags);

	if (filedata->fh == -1)
	{
		fuse_error = errno;
		add_log(ADDLOG_FAIL, "[fuse]open", "open fail: '%s'\n", strerror(fuse_error));
		/* io error */
		return -fuse_error;
	}

	add_log(ADDLOG_OP_SUCCESS, "[fuse]open", "done.\n");

	/* success */
	return -ESUCCESS;
}

/* read data from an open file */
int fuse_impl_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *filedata)
{
	int file;
	int result;
	
	check(path != NULL);
	/* check(buf != NULL); */
	check(filedata != NULL);
	check((int)filedata->fh != 0);

	add_log(ADDLOG_OPERATION, "[fuse]read", "called, args='%s',buf:%p,size:%u,off:%u\n", path, buf, size, offset);

	if (buf == NULL)
	{
		add_log(ADDLOG_USER_ERROR, "[fuse]read", "buffer is a NULL pointer\n");
		/* invalid argument */
		errno = EINVAL;
		return 0;
	}

	file = filedata->fh;

	if (lseek(file, offset, SEEK_SET) == (off_t)-1)
	{
		fuse_error = errno;
		add_log(ADDLOG_USER_ERROR, "[fuse]read", "read fail: '%s'\n", strerror(fuse_error));
		/* io error */
		errno = fuse_error;
		return 0;
	}

	result = read(file, buf, size);
	if (result == -1)
	{
		fuse_error = errno;
		add_log(ADDLOG_USER_ERROR, "[fuse]read", "read fail: '%s'\n", strerror(fuse_error));
		/* io error */
		errno = fuse_error;
		return 0;
	}

	add_log(ADDLOG_OP_SUCCESS, "[fuse]read", "done.\n");

	return result;
}

/* write data to an open file */
int fuse_impl_write (const char *path, const char *data, size_t size, off_t offset, struct fuse_file_info *filedata)
{
	int file;
	int result;

	check(path != NULL);
	/* check(data != NULL); */
	check(filedata != NULL);
	check((int)filedata->fh != 0);

	add_log(ADDLOG_OPERATION, "[fuse]write", "called, args='%s',buf:%p,size:%u,off:%u\n", path, data, size, offset);

	if (data == NULL)
	{
		add_log(ADDLOG_USER_ERROR, "[fuse]write", "buffer is a NULL pointer\n");
		/* invalid argument */
		errno = EINVAL;
		return 0;
	}

	file = filedata->fh;

	if (lseek(file, offset, SEEK_SET) == (off_t)-1)
	{
		fuse_error = errno;
		add_log(ADDLOG_USER_ERROR, "[fuse]write", "write fail: '%s'\n", strerror(fuse_error));
		/* io error */
		errno = fuse_error;
		return 0;
	}

	result = write(file, data, size);
	if (result == -1)
	{
		fuse_error = errno;
		add_log(ADDLOG_USER_ERROR, "[fuse]write", "write fail: '%s'\n", strerror(fuse_error));
		/* io error */
		errno = fuse_error;
		return 0;
	}

	add_log(ADDLOG_OP_SUCCESS, "[fuse]write", "done.\n");

	return result;
}

/* get file system statistics */
int fuse_impl_statfs (const char *path, struct statvfs *stat)
{
	check(path != NULL);
	check(stat != NULL);

	add_log(ADDLOG_OPERATION, "[fuse]statfs", "called, args='%s'\n", path);

	if (statvfs(CONFIG_DB5_DATA_DIR, stat) != 0)
	{
		fuse_error = errno;
		add_log(ADDLOG_FAIL, "[fuse]statfs", "error during statfs: '%s'\n", strerror(fuse_error));
		/* io error */
		return -fuse_error;
	}

	add_log(ADDLOG_OP_SUCCESS, "[fuse]statfs", "done.\n");

	/* success */
	return -ESUCCESS;
}

/* flush cached meta-data (update database) */
int fuse_impl_flush (const char *path, struct fuse_file_info *filedata)
{
	check(path != NULL);
	check(filedata != NULL);
	check((int)filedata->fh != 0);

	add_log(ADDLOG_OPERATION, "[fuse]flush", "called, args='%s'\n", path);

	/* try to update database */
	if (db5_update(file_remove_headslash(path)) != true)
	{
		add_log(ADDLOG_RECOVER, "[fuse]flush", "unable to update database for '%s'\n", path);
	}

	add_log(ADDLOG_OP_SUCCESS, "[fuse]flush", "done.\n");

	/* success */
	return -ESUCCESS;
}

/* read directory */
int fuse_impl_readdir(const char *path, void *data, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *filedata)
{
	uint32_t i;
	char **files_list;

	check(path != NULL);
	check(filler != NULL);

	(void) offset;
	(void) filedata;

	add_log(ADDLOG_OPERATION, "[fuse]readdir", "called, args='%s',data:%p,filler:%p\n", path, data, filler);
	
	/* only root dir */
	if(strcmp(path, "/") != 0)
	{
		add_log(ADDLOG_USER_ERROR, "[fuse]readdir", "directory '%s' does not exist\n", path);
		/* file does not exists */
		return -ENOENT;
	}

	filler(data, ".", NULL, 0);
	filler(data, "..", NULL, 0);

	files_list = db5_select_filename();
	if (files_list == NULL)
	{
		add_log(ADDLOG_FAIL, "[fuse]readdir", "unable to get file information form database\n");
		/* filesystem error */
		return -EIO;
	}

	for(i=0; files_list[i] != NULL; i++)
	{
		add_log(ADDLOG_DEBUG, "[fuse]readdir", "%u:'%s'\n", i, files_list[i]);
		filler(data, files_list[i], NULL, 0);
		free(files_list[i]);
	}
	free(files_list);

	add_log(ADDLOG_OP_SUCCESS, "[fuse]readdir", "done.\n");

	/* success */
	return -ESUCCESS;
}

/* flush opened file */
int fuse_impl_fsync(const char *path, int inode, struct fuse_file_info *filedata)
{
	check(path != NULL);
	check(filedata != NULL);
	check((int)filedata->fh != 0);

	add_log(ADDLOG_OPERATION, "[fuse]fsync", "called, args='%s'\n", path);

	if (fsync((int)filedata->fh == -1))
	{
		fuse_error = errno;
		add_log(ADDLOG_FAIL, "[fuse]fsync", "sync fail: '%s'\n", strerror(fuse_error));
		/* io error */
		return -fuse_error;
	}

	add_log(ADDLOG_OP_SUCCESS, "[fuse]fsync", "done.\n");

	/* success */
	return -ESUCCESS;
}

