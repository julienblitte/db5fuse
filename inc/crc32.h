/**
 * @file crc32.h
 * @brief Header - Crc32 checksum
 * @author Julien Blitte
 * @version 0.1
 */
#ifndef INC_CRC32_H
#define INC_CRC32_H
#include <stdint.h>
#include <stdlib.h>

/**
 * @brief traditional polynomial value
 */
#define CRC32_POLYNOMINAL	0x04c11db7

/**
 * @brief initialize crc32 compute table
 * @param polynominal seed
 */
void crc32_init_seed(const uint32_t polynominal);

/**
 * @brief initialize crc32 compute table with default seed
 */
#define crc32_init()	crc32_init_seed(CRC32_POLYNOMINAL);

/**
 * @brief compute an array crc32 checksum
 * @param data data source
 * @param size size of source
 * @return data checksum
 */
uint32_t crc32(const char *data, const size_t size);

/**
 * @brief compute a file crc32 checksum
 * @param path file source - utf8
 * @return file checksum
 */
uint32_t crc32_file(const char *path);

/**
 * @brief compute a string crc32 checksum
 * @param data string source
 * @return string checksum
 */
uint32_t strcrc32(const char *data);

#endif

