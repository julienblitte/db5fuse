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

/**
 * @brief extract a string column from database (system)
 * @param reloffset structure offset
 * @param size value size
 * @return corresponding column
 */
db5_string_entry *db5_dat_select_string_column(const size_t reloffset, const size_t size);

/**
 * @brief extract a string column from database (user friendly)
 * @param member the column to extract
 * @return corresponding column
 */
#define db5_dat_str_extract(member)	db5_dat_select_string_column(offsetof(db5_row,member),membersizeof(db5_row,member))

/**
 * @brief free a string column
 * @param index the string column to free
 * @param count number of lines
 */
void db5_dat_free_string_column(db5_string_entry *index, const uint32_t count);

/**
 * @brief extract a number column from database (system)
 * @param reloffset structure offset
 * @return corresponding column
 */
db5_number_entry *db5_dat_select_number_column(const size_t reloffset);

/**
 * @brief extract a number column from database (user friendly)
 * @param member the column to extract
 * @return corresponding column
 */
#define db5_dat_num_extract(member)	db5_dat_select_number_column(offsetof(db5_row,member))

/**
 * @brief free a number column
 * @param index the number column to free
 */
void inline db5_dat_free_number_column(db5_number_entry *index);

#endif

