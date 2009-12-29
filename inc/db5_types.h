/**
 * @file db5_types.h
 * @brief Header - Database db5, types
 * @author Julien Blitte
 * @version 0.1
 */
#ifndef INC_DB5_GENERAL_H
#define INC_DB5_GENERAL_H

#include <stdint.h>

/**
 * @brief get size of a structure member
 * @param structure where element is 
 * @param member the member of structure to get size
 * @return size of element
 */
#define membersizeof(structure,member)	(sizeof(((structure*)NULL)->member))

/**
 * @brief a database row
 */
typedef struct
{
	/** @brief hidden entry */
	uint32_t hidden;
	/** @brief ruf */
	uint32_t reserved[2];
	/** @brief real file path - latin1 */
	char filepath[2*28];
	/** @brief real filename - latin1 */
	char filename[2*16];
	/** @brief bitrate, bit/s */
	uint32_t bitrate;
	/** @brief samplerate, Hz */
	uint32_t samplerate;
	/** @brief duration, sec */
	uint32_t duration;
	/** @brief artist name - latin1 */
	char artist[2*40];
	/** @brief album name - latin1 */
	char album[2*40];
	/** @brief genre of music - latin1 */
	char genre[2*20];
	/** @brief title name - latin1 */
	char title[2*40];
	/** @brief track number */
	uint32_t track;
	/** @brief year */
	uint32_t year;
	/** @brief size of file */
	uint32_t filesize;
	/** @brief source of entry is computer */
	#define DB5_SOURCE_FILE		0
	/** @brief source of entry is optical record */
	#define DB5_SOURCE_OPTICAL	1
	/** @brief source of entry is analog record */
	#define DB5_SOURCE_ANALOG	2
	/** @brief source of entry is micro record */
	#define DB5_SOURCE_MICRO	3
	/** @brief source of entry */
	uint32_t source; 
} db5_row;

/**
 * @brief string entry
 */
typedef struct
{
	/** @brief value of entry */
	char *value;
	/** @brief position of entry */
	uint32_t position;
	/** @brief checksum of entry */
	uint32_t crc32;
} db5_string_entry;

/**
 * @brief number entry
 */
typedef struct
{
	/** @brief value of entry */
	uint32_t value;
	/** @brief position of entry */
	uint32_t position;
} db5_number_entry;

/**
 * @brief the music file path
 */
extern const char *db5_music_path;

#endif
