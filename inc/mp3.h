/**
 * @file mp3.h
 * @brief Header - Mp3 file format
 * @author Julien Blitte
 * @version 0.1
 */
#ifndef INC_MP3_H
#define INC_MP3_H
#include <stdbool.h>
#include "db5_types.h"

/**
 * @brief generate a db5 row retrieving needed informations in file
 * @param filename the local filename - utf8
 * @param row a pointer to return results
 * @return true if successfull
 */
bool mp3_generate_row(const char *filename, db5_row *row);

#endif
