/**
 * @file names.h
 * @brief Header - Database db5, long filename support
 * @author Julien Blitte
 * @version 0.1
 */
#ifndef INC_NAMES_H
#define INC_NAMES_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/**
 * @brief initialize longname support
 * @return true if successfull
 */
bool names_init();

/**
 * @brief convert short name to longname
 * @param shortname name to convert to longname - latin1
 * @return longname or NULL if error - latin1
 */
const char *names_select_longname(const char *shortname);

/**
 * @brief convert long name to shortname
 * @param longname name to convert to shortname - latin1
 * @param shortname data to store shortname - latin1
 * @param shortname_size size of shortname
 * @return true if successfull
 */
bool names_generate_shortname(const char *longname, char *shortname, const size_t shortname_size);

/**
 * @brief save names data on file
 * @return true if successfull
 */
bool names_save();

/**
 * @brief add an entry in names list
 * @param filename longname to remove - latin1
 */
void names_insert(const char *filename);

/**
 * @brief delete an entry in names list
 * @param filename longname to remove - latin1
 * @return true if successfull
 */
bool names_delete(const char *filename);

/**
 * @brief print names list (debug)
 */
void names_print();

/**
 * @brief free names list
 */
void names_free();

/**
 * @brief retrieve look in names list to find shortname using longname
 * @param filename filename to find - latin1
 * @param shortname buffer where shortname will be stored - latin1
 * @param shortname_size of shortname
 * @return true if successfull
 */
bool names_select_shortname(const char *filename, char *shortname, const size_t shortname_size);

#endif
