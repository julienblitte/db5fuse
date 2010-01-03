/**
 * @file db5_dat.h
 * @brief Header - Database db5, data
 * @author Julien Blitte
 * @version 0.1
 */
#ifndef INC_DB5_DAT_H
#define INC_DB5_DAT_H
#include <stdbool.h>
#include <stdint.h>

#include "db5_types.h"

/**
 * @brief magic value returned when a row is not found in database
 */
#define DB5_ROW_NOT_FOUND	((uint32_t)-1)

/**
 * @brief initialize databse
 * @return true if is successfull
 */
bool db5_dat_init();

/**
 * @brief clean database on exiting
 */
void db5_dat_free();

/**
 * @brief read an entry into database
 * @param index entry position
 * @param entry read entry
 * @return true if is successfull
 */
bool db5_dat_select_row(const uint32_t index, db5_row *entry);

/**
 * @brief modify an entry into database
 * @param index entry position
 * @param entry new entry
 * @return true if is successfull
 */
bool db5_dat_update(const uint32_t index, db5_row *entry);

/**
 * @brief add an entry into database
 * @param entry new entry
 * @return true if is successfull
 */
bool db5_dat_insert(db5_row *entry);

/**
 * @brief delete an entry of database
 * @param index entry position
 * @return true if is successfull
 */
bool db5_dat_delete_row(const uint32_t index);

/**
 * @brief find a row corresponding to a file
 * @param filename the local filename - latin1
 * @return row position, (unsigned)-1 if no found
 */
uint32_t db5_dat_select_by_filename(const char *filename);

#endif

