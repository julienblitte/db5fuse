/**
 * @file utf8.c
 * @brief Source - Unicode utf-8
 * @author Julien Blitte
 * @version 0.1
 */
#include <stdio.h>
#include "check.h"

/*
 * Here, we have a lot of macro instead function to be more efficient after compiling
 */


/** @brief header bits added to one-byte utf8 characters */
#define UTF8_HEAD_7		0x00
/** @brief header mask used to add bits to one-byte utf8 characters */
#define UTF8_HEAD_7_MASK	0x80

/** @brief header bits added to two-bytes utf8 characters */
#define UTF8_HEAD_11		0xc0
/** @brief header mask used to add bits to two-bytes utf8 characters */
#define UTF8_HEAD_11_MASK	0xe0

/** @brief bits added to non-first bytes of multi-bytes utf8 characters */
#define UTF8_TAIL		0x80
/** @brief mask used to add bits to non-first bytes of multi-bytes utf8 characters */
#define UTF8_TAIL_MASK		0xc0
/** @brief multi-bytes utf8 characters bit shift */
#define UTF8_TAIL_SHIFT		6

/** @brief shift data to be coded in utf8 */
#define UTF8_SHIFTER(data,byte)	(data >> (byte*UTF8_TAIL_SHIFT))
/** @brief add utf-8 one-byte bits header */
#define UTF8_HEADER_7(data)	(UTF8_HEAD_7 | (data & ~UTF8_HEAD_7_MASK))
/** @brief add utf-8 two-bytes bits header */
#define UTF8_HEADER_11(data)	(UTF8_HEAD_11 | (data & ~UTF8_HEAD_11_MASK))
/** @brief add utf-8 non-first bytes bits header */
#define UTF8_TAILER(data)	(UTF8_TAIL | (data & ~UTF8_TAIL_MASK))

/** @brief test if the byte is a one-byte utf8 character */
#define IS_UTF8_HEADER_7(byte)	((unsigned char)(byte & UTF8_HEAD_7_MASK) == UTF8_HEAD_7)
/** @brief test if the byte is a two-byte utf8 character */
#define IS_UTF8_HEADER_11(byte)	((unsigned char)(byte & UTF8_HEAD_11_MASK) == UTF8_HEAD_11)
/** @brief test if the byte is a part of (and not the first of) a multi-byte utf8 character */
#define IS_UTF8_TAIL(byte)	((unsigned char)(byte & UTF8_TAIL_MASK) == UTF8_TAIL)

/** @brief unshift utf8 character to retrieve data */
#define UTF8_UNSHIFTER(data,byte)	(data << (byte*UTF8_TAIL_SHIFT))
/** @brief remove utf-8 one-byte bits header */
#define UTF8_UNHEADER_7(data)	(data & ~UTF8_HEAD_7_MASK)
/** @brief remove utf-8 two-bytes bits header */
#define UTF8_UNHEADER_11(data)	(data & ~UTF8_HEAD_11_MASK)
/** @brief remove utf-8 non-first bytes bits header */
#define UTF8_UNTAILER(data)	(data & ~UTF8_TAIL_MASK)


size_t iso8859_utf8(const char *source, char *dest, const size_t dest_size)
{
	size_t i;
	size_t j;
	unsigned char byte;

	check(source != NULL);
	check(dest != NULL);
	check(dest_size > 0);

	i = 0, j =0;
	while(i < dest_size)
	{
		byte = (unsigned)source[i];
		if (byte < 0x80)
		{
			dest[j] = UTF8_HEADER_7(UTF8_SHIFTER(byte,0));
			j++;
		}
		else
		{
			dest[j] = UTF8_HEADER_11(UTF8_SHIFTER(byte,1));
			j++;
			dest[j] = UTF8_TAILER(byte);
			j++;
		}
		
		if (byte == '\0')
		{
			break;
		}
		i++;
	}

	return j;
}

size_t utf8_iso8859(const char *source, char *dest, const size_t dest_size)
{
	size_t i;
	size_t j;
	unsigned char byte;

	check(source != NULL);
	check(dest != NULL);
	check(dest_size > 0);

	i = 0, j =0;
	while(i < dest_size)
	{
		byte = source[i];
		if (IS_UTF8_HEADER_7(byte))
		{
			dest[j] = UTF8_UNHEADER_7(byte);
			j++;
		}
		else if (IS_UTF8_HEADER_11(byte))
		{
			dest[j] = UTF8_UNSHIFTER(UTF8_UNHEADER_11(byte),1);
			i++, byte = source[i];
			dest[j] |= UTF8_UNTAILER(byte);
			j++;
		}

		if (byte == '\0')
		{
			break;
		}
		i++;
	}

	return j;
}
