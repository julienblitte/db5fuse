/**
 * @file fuse_main.c
 * @brief Source - Filesystem, main
 * @author Julien Blitte
 * @version 0.1
 */
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "fuse_implementation.h"
#include "check.h"


/**
 * @brief usage function
 */
static void usage()
{
	fprintf(stderr, "usage: db5fuse <hddfilesystem> <mountpoint>\n\n");
	fprintf(stderr, "       <hddfilesystem> is the mounted HDD100/HDD120 fat file system path.\n");
	fprintf(stderr, "       <mountpoint> is where the filesystem will be mounted.\n");
	exit(EXIT_FAILURE);
}

/**
 * @brief list of fuse implemented operations
 */
static struct fuse_operations fuse_oper = 
{
	.getattr = &fuse_impl_getattr,
	.unlink = &fuse_impl_unlink,
	.rename = &fuse_impl_rename,
	.truncate = &fuse_impl_truncate,
	.open = &fuse_impl_open,
	.read = &fuse_impl_read,
	.write = &fuse_impl_write,
	.statfs = &fuse_impl_statfs,
	.flush = &fuse_impl_flush,
	.readdir = &fuse_impl_readdir,
	.init = &fuse_impl_init,
	.destroy = &fuse_impl_destroy,
	.create  = &fuse_impl_create,
	.utimens = &fuse_impl_utimens,
	.fsync = &fuse_impl_fsync
};

/**
 * @brief resolve a path to absolute one
 * @param relative the relative path to resolte - utf8
 * @return the absolute path or NULL if function failed - utf8
 */
static const char *absolute_path(const char *relative)
{
	char *backup_dir;
	const char *result;

	check(relative != NULL);

	/* save current dir */
	backup_dir = getcwd(NULL, 0);
	if (backup_dir == NULL)
	{
		return NULL;
	}

	/* change directory to one given as device */
	if (chdir(relative) != 0)
	{
		free(backup_dir);
		return NULL;
	}

	/* retrieve now current directory to get absolute path */
	result = getcwd(NULL, 0);

	/* restore old dir */
	chdir(backup_dir);
	free(backup_dir);

	return (const char *)result;
}

/**
 * @brief main entry point
 * @param argc argument count, including path of this binary
 * @param argv arguments value, first is the binary path
 * @return exit code
 */
int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		usage();
	}

	fuse_device = absolute_path(argv[1]);
	if (fuse_device == NULL)
	{
		fprintf(stderr, "Unable to reach device '%s'!", argv[1]);
		usage();
	}

	/* makes argv[1] disappear */
	argv[1] = argv[0];
	argv++; argc--;

	return fuse_main(argc, argv, &fuse_oper, NULL);
}
