/**
 * @file db5.h
 * @brief Header - Database db5
 * @author Julien Blitte
 * @version 0.1
 */
#ifndef INC_DB5_H
#define INC_DB5_H
#include <stdbool.h>

#include "db5_types.h"

/**
 * @brief open database
 * @return true if successfull
 */
bool db5_init();

/**
 * @brief close database
 */
void db5_free();

/**
 * @brief convert all strings of an entry to char
 * @param entry to use/edit
 */
void db5_unwidechar_row(db5_row *entry);

/**
 * @brief convert all strings of an entry to wide char
 * @param entry to store
 */
void db5_widechar_row(db5_row *entry);

/**
 * @brief update file information in database
 * @param filename the virtual name - utf8
 * @return true if successfull
 */
bool db5_update(const char *filename);

/**
 * @brief add a file in database
 * @param filename the virtual name - utf8
 * @return true if successfull
 */
bool db5_insert(const char *filename);

/**
 * @brief remove a file from database
 * @param filename the virtual name - utf8
 * @return true if successfull
 */
bool db5_delete(const char *filename);

/**
 * @brief list files entry
 * @return a NULL terminated array to all virutal filenames - utf8
 */
char **db5_select_filename();

/**
 * @brief index all columns
 * @return true if successfull
 */
bool db5_index();

/**
 * @brief retrieve the local file name of a longname
 * @param filename longname to convert to local filename - utf8
 * @param localfile buffer where result is returned - utf8
 * @param localfile_size size of localfile
 * @return true if successfull
 */
bool db5_localfile(const char *filename, char *localfile, const size_t localfile_size);

/**
 * @brief retrieve existing shortname from a longname
 * @param longname the filename to resolve in shortname - utf8
 * @param shortname buffer where shortname will be stored - latin1
 * @param shortname_size of shortname
 * @return true if successfull
 */
bool db5_longname_to_shortname(const char *longname, char *shortname, const size_t shortname_size);

/**
 * @brief generate a database entry for a file - you must use db5_widechar_row after this function
 * @param localfile file to generate database entry - utf8
 * @param row where database entry is stored
 * @return true if successfull
*/
bool db5_generate_row(const char *localfile, db5_row *row);

/**
 * @brief complete the path of a short filename to get local filename (real path)
 * @param shortname the filename to complete path - latin1
 * @param localfile buffer where localfile will be stored - utf8
 * @param localfile_size of localfile
 * @return true if successfull
 */
bool db5_shortname_to_localfile(const char *shortname, char *localfile, const size_t localfile_size);

/**
 * @brief test if a file is in database
 * @param filename file to check in database - utf8
 * @return true if file is in database
 */
bool db5_exists(const char *filename);

/**
 * @brief get the number of files
 * return the number of files in database
 */
uint32_t db5_count();

/**
 * @brief print a database entry
 * @param row the row to print
 */
void db5_print_row(db5_row *row);

#endif

