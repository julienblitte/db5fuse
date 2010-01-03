/**
 * @file db5_index.h
 * @brief Header - Database db5, index
 * @author Julien Blitte
 * @version 0.1
 */
#ifndef INC_DB5_IDX_H
#define INC_DB5_IDX_H
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#include "db5_types.h"
#include "db5_dat.h"


/** @brief index used for test - ascii - not null terminated */
#define DB5_IDX_CODE_DEV	0x56454440 /* '@DEV' */
/** @brief filename index - ascii - not null terminated */
#define DB5_IDX_CODE_FILENAME	0x4d414e46 /* 'FNAM' */
/** @brief filepath index - ascii - not null terminated */
#define DB5_IDX_CODE_FILEPATH	0x48545046 /* 'FPTH' */
/** @brief album index - ascii - not null terminated */
#define DB5_IDX_CODE_ALBUM	0x424c4154 /* 'TALB' */
/** @brief genre index - ascii - not null terminated */
#define DB5_IDX_CODE_GENRE	0x4e4f4354 /* 'TCON' */
/** @brief title index - ascii - not null terminated */
#define DB5_IDX_CODE_TITLE	0x32544954 /* 'TIT2' */
/** @brief artist index - ascii - not null terminated */
#define DB5_IDX_CODE_ARTIST	0x31455054 /* 'TPE1' */
/** @brief track index - ascii - not null terminated */
#define DB5_IDX_CODE_TRACK	0x4b435254 /* 'TRCK' */
/** @brief source index	- ascii - not null terminated */
#define DB5_IDX_CODE_SOURCE	0x43525358 /* 'XSRC' */

/**
 * @brief index a column (system)
 * @param reloffset element address in structure line
 * @param size size of element
 * @param code index code to generate
 * @return true if successfull
 */
bool db5_index_index_column(const ptrdiff_t reloffset, const size_t size, const uint32_t code);

/**
 * @brief index a string column (user friendly)
 * @param member element in structure line
 * @param code index code to generate
 * @return true if successfull
 */
#define db5_index_colindex(member,code) 	db5_index_index_column(offsetof(db5_row,member),membersizeof(db5_row,member),code)

#endif
