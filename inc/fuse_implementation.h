/**
 * @file fuse_implementation.h
 * @brief Header - Filesystem, implementation
 * @author Julien Blitte
 * @version 0.1
 */
#ifndef INC_DB5FUSE_OPERATIONS_H
#define INC_DB5FUSE_OPERATIONS_H
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/**
 * @brief the mounted point - utf8
 */
extern const char *fuse_device;

/**
 * @brief file attributes
 * @param path file - utf8
 * @param attr attributes to return
 * @return error code, 0 if successfull
 */
int fuse_impl_getattr (const char *path, struct stat *attr);

/**
 * @brief remove a file
 * @param path file - utf8
 * @return error code, 0 if successfull
 */
int fuse_impl_unlink (const char *path);

/**
 * @brief rename a file
 * @param path file - utf8
 * @param newname the new name of the file - utf8
 * @return error code, 0 if successfull
 */
int fuse_impl_rename (const char *path, const char *newname);

/**
 * @brief change the size of a file
 * @param path file - utf8
 * @param newsize the new size of file
 * @return error code, 0 if successfull
 */
int fuse_impl_truncate (const char *path, off_t newsize);

/**
 * @brief open a file
 * @param path file - utf8
 * @param filedata file information
 * @return error code, 0 if successfull
 */
int fuse_impl_open (const char *path, struct fuse_file_info *filedata);

/**
 * @brief read data from an open file
 * @param path file - utf8
 * @param data data
 * @param size data size
 * @param offset offset in file
 * @param filedata file information
 * @return error code, 0 if successfull
 */
int fuse_impl_read (const char *path, char *data, size_t size, off_t offset, struct fuse_file_info *filedata);

/**
 * @brief write data to an open file
 * @param path file - utf8
 * @param data data
 * @param size data size
 * @param offset offset in file
 * @param filedata file information
 * @return error code, 0 if successfull
 */
int fuse_impl_write (const char *path, const char *data, size_t size, off_t offset, struct fuse_file_info *filedata);

/**
 * @brief get file system statistics
 * @param path file - utf8
 * @param stat structure for returning information
 * @return error code, 0 if successfull
 */
int fuse_impl_statfs (const char *path, struct statvfs *stat);

/**
 * @brief flush cached data
 * @param path file - utf8
 * @param filedata file information
 * @return error code, 0 if successfull
 */
int fuse_impl_flush (const char *path, struct fuse_file_info *filedata);

/**
 * @brief read directory
 * @param path diretory - utf8
 * @param data (not used)
 * @param filler callback function to list files
 * @param offset (not used)
 * @param filedata file information
 * @return error code, 0 if successfull
 */
int fuse_impl_readdir(const char *path, void *data, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *filedata);

/**
 * @brief initialize filesystem
 * @param fs_attr (not used)
 */
void *fuse_impl_init (struct fuse_conn_info *fs_attr);

/**
 * @brief clean up filesystem
 * @param data (not used)
 */
void fuse_impl_destroy (void *data);

/**
 * @brief create and open a file
 * @param path file - utf8
 * @param mode file mode
 * @param filedata file information
 * @return error code, 0 if successfull
 */
int fuse_impl_create (const char *path, mode_t mode, struct fuse_file_info *filedata);

/**
 * @brief change the access and modification times
 * @param path file - utf8
 * @param tv new time
 * @return error code, 0 if successfull
 */
int fuse_impl_utimens (const char *path, const struct timespec tv[2]);

/**
 * @brief flush opened file (sync data)
 * @param path file - utf8
 * @param inode file's inode
 * @param filedata file information
 * @return error code, 0 if successfull
 */
int fuse_impl_fsync(const char *path, int inode, struct fuse_file_info *filedata);

#endif

