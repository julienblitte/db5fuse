/**
 * @file crc32.c
 * @brief Source - Crc32 checksum
 * @author Julien Blitte
 * @version 0.1
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "crc32.h"

/** @brief Size of buffer used to compute crc of files */
#define CRC_BUFFER_SIZE	10240

/** @brief Crc compute table */
static uint32_t crc32_table[256];

/**
 * @brief perform a bit reflection (invert bit order)
 * @param value the value to relect
 * @param bitsize number of bit
 * @return reflection of value
 */
static uint32_t crc32_reflect(uint32_t value, const uint32_t bitsize)
{
	uint32_t result;
	int i;

	result = 0;
	for(i=0; i < bitsize; i++)
	{
		result = (result << 1) | (value & 1);
		value >>= 1;
	}
	return result;
}

/**
 * @brief continue computing crc with data
 * @param data the data to compute
 * @param size size of data
 * @return crc current crc value
 */
static uint32_t crc32_compute(const char *data, const size_t size, uint32_t crc)
{
	int i;

	crc =~ crc;

	/* perform the algorithm on each character using the lookup table values */
	for(i=0; i < size; i++) 
	{
		crc=((crc >> 8) & 0x00ffffff) ^ crc32_table[(crc & 0xff) ^ (data[i] & 0xff)];
	}

	return ~crc;
}

void crc32_init_seed(const uint32_t polynominal)
{
	int i, j;

	/* return if already initialized */
	if (*crc32_table) return;

	for(i=0; i < 256; i++)
	{
		crc32_table[i] = crc32_reflect(i, 8) << 24;

		for(j=0; j < 8; j++)
		{
			crc32_table[i]=((crc32_table[i] << 1) ^ ((crc32_table[i] & (1 << 31)) ? polynominal : 0));
		}
		crc32_table[i] = crc32_reflect(crc32_table[i], 32);
	}
}

uint32_t inline crc32(const char *data, const size_t size)
{
	return crc32_compute(data, size, 0);
}

uint32_t strcrc32(const char *data)
{
	int i;
	uint32_t crc;

	crc = 0xffffffff;

	/* perform the algorithm on each character using the lookup table values */
	for(i=0; data[i] != '\0'; i++) 
	{
		crc=((crc >> 8) & 0x00ffffff) ^ crc32_table[(crc & 0xff) ^ (data[i] & 0xff)];
	}

	return ~crc;
}
 
uint32_t crc32_file(const char *path)
{
	FILE *fp;
	char data[CRC_BUFFER_SIZE];
	size_t read;
	uint32_t crc;

	if (path == NULL)
	{
		return 0;
	}

	fp = fopen(path, "rb");
	if (fp == NULL)
	{
		fprintf(stderr, "crc32: unable to open file '%s'!\n", path);
		return 0;
	}

	crc = 0;
	while(!feof(fp))
	{
		read = fread(data, 1, sizeof(data), fp);
		crc = crc32_compute(data, read, crc);
	}

	fclose(fp);

	return crc;
}

