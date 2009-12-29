/**
 * @file db5_hdr.h
 * @brief Header - Database db5, meta-data
 * @author Julien Blitte
 * @version 0.1
 */
#ifndef INC_DB5_HDR_H
#define INC_DB5_HDR_H
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#include "db5_types.h"

/**
 * @brief initialize meta-database
 * @return true if successfull
 */
bool db5_hdr_init();

/**
 * @brief free meta-database
 * @return true if successfull
 */
bool db5_hdr_free();

/**
 * @brief get current row count
 * @return count of row in database
 */
uint32_t db5_hdr_count();

/**
 * @brief update row number in meta-database
 * @param delta the value to add to current count
 * @return true if successfull
 */
bool db5_hdr_grow(const int delta);

#endif

