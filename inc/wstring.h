/**
 * @file wstring.h
 * @brief Header - Widechar string
 * @author Julien Blitte
 * @version 0.1
 */
#ifndef INC_WSTRING_H
#define INC_WSTRING_H

#include <stdlib.h>

/**
 * @brief convert a widechar string to char string
 * @param widestring string to turn to char string, this parameter is altered - widechar latin1
 * @param length of widestring
 */
void ws_wstoa(char *widestring, const size_t length);

/**
 * @brief convert a char string to widechar string
 * @param string char string to turn to widechar string, this parameter is altered - latin1
 * @param length of string
 */
void ws_atows(char *string, const size_t length);

/**
 * @brief copy memory area reversing byte order
 * @param dest destination - binary
 * @param src source - binary
 * @param len size of data
 */
void *ws_memswapcpy(void *dest, void *src, const size_t len);

#endif
