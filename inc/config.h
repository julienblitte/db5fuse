/**
 * @file config.h
 * @brief Header - Main configuration
 * @author Julien Blitte
 * @version 0.1
 */
#ifndef INC_CONFIG_H
#define INC_CONFIG_H

/** @brief the music directory path on device - linux format - ascii */
#define CONFIG_MUSIC_PATH	"MUSIC"

/** @brief the maximum logged level */
#define CONFIG_LOG_LEVEL	7
/** @brief the log filename - utf8 */
#define CONFIG_LOG_FILENAME	"db5fuse.log"

/** @brief default arstist - ascii */
#define CONFIG_DEFAULT_ARTIST	"Unknow artist"
/** @brief default album - ascii */
#define CONFIG_DEFAULT_ALBUM	"Unknow album"
/** @brief default genre - ascii */
#define CONFIG_DEFAULT_GENRE	"Unknow"
/** @brief default title - ascii */
#define CONFIG_DEFAULT_TITLE	"Unknow title"

/** @brief asf file extension - ascii */
#define CONFIG_ASF_EXT		"wma"
/** @brief mp3 file extension - ascii */
#define CONFIG_MPEG_EXT		"mp3"

/** @brief maximum of db5 database entries */
#define CONFIG_MAX_DB5_ENTRY	4294967293U

/** @brief relative path do database files - utf8 */
#define CONFIG_DB5_DATA_DIR	"System/DATA"
/** @brief database data filename - utf8 */
#define CONFIG_DB5_DAT_FILE	"DB5000.DAT"
/** @brief database meta-data filename - utf8 */
#define CONFIG_DB5_HDR_FILE	"DB5000.HDR"
/** @brief database indexes filename format - utf8 */
#define CONFIG_DB5_IDX_FILE	"DB5000_%c%c%c%c.IDX"
/** @brief database names filename - utf8 */
#define CONFIG_NAMES_FILE	"Names.txt"

#endif

