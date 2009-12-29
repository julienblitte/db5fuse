/**
 * @file file.h
 * @brief Header - File operations
 * @author Julien Blitte
 * @version 0.1
 */
#ifndef INC_FILE_H
#define INC_FILE_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

/** @brief shared buffer for file operation */
extern char file_common_buffer[10240];

/**
 * @brief get size of a file
 * @param filename the path to file - utf8
 * @result size of file
 */
off_t file_filesize(const char *filename);

/**
 * @brief get size of an opened file
 * @param f an opened file
 * @result size of file
 */
off_t file_filesize_f(FILE *f);

/**
 * @brief modify path to directory, file and extension string - path is destroyed
 * @param path the path to explode in directory, file and extension - destroyed - utf8 or latin1
 * @param dir directory pointer address - path's charset
 * @param file file pointer address - path's charset
 * @param ext extension pointer address - path's charset
 */
void file_path_explode(char *path, char **dir, char **file, char **ext);

/**
 * @brief set the working directory (the device where a db5 database is)
 * @param device the device where we want to fix working directory - utf8
 * @result true is successfull
 */
bool file_set_context(const char *device);

/**
 * @brief test if the file exists
 * @param path to filename - utf8
 * @result true is filename exists
 */
bool file_exists(const char *path);

/**
 * @brief test if the file exists
 * @param path to filename - utf8 or latin1
 * @result true is filename exists
 */
const char *file_remove_headslash(const char *path);

/**
 * @brief return extension of a file (incluing dot)
 * @param path filename/path to get extension - utf8 or latin1
 * @result pointer to extension in the filename
 */
const char *file_get_extension(const char *path);

/**
 * @brief alter a path string to turn slashes to counterslashes
 * @param path filename/path to turn slashes - utf8 or latin1
 */
void file_windows_slashes(char *path);

/**
 * @brief open the first file that match to specified filename without case sensitivity
 * @param directory the directory where file must be found - utf8
 * @param filename the filename to try to find - utf8
 * @param mode file mode
 * @return file handle or NULL if not found and file cannot be created
 */
FILE *file_fcaseopen(const char *directory, const char *filename, const char *mode);

/**
 * @brief truncate a file
 * @param f file to truncate
 * @param len new size of file
 * @return true if successfull
 */
bool file_truncate(FILE *f, const off_t len);

#endif
