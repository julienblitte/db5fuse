/**
 * @file utf8.h
 * @brief Header - Unicode utf-8
 * @author Julien Blitte
 * @version 0.1
 */
#ifndef INC_UTF8_H
#define INC_UTF8_H

/**
 * @brief convert iso to unicode
 * @param source latin1 string to convert to utf8 - latin1
 * @param dest destination where new string is stored - utf8
 * @param dest_size size of dest
 * @result size of new string
 */
size_t iso8859_utf8(const char *source, char *dest, const size_t dest_size);

/**
 * @brief convert iso to unicode
 * @param source utf8 string to convert to latin1 - utf8
 * @param dest destination where new string is stored - latin1
 * @param dest_size size of dest
 * @result size of new string
 */
size_t utf8_iso8859(const char *source, char *dest, const size_t dest_size);

#endif
