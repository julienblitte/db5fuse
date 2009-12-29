/**
 * @file wstring.c
 * @brief Source - Widechar string
 * @author Julien Blitte
 * @version 0.1
 */
#include <stdlib.h>
#include "check.h"
#include <string.h>
#include "wstring.h"

void ws_wstoa(char *widestring, const size_t length)
{
	int i;
	
	check(widestring != NULL);
	
	/* loop 'i=0' is useless here */
	for(i=1; i < length/2; i++)
	{
		widestring[i] = widestring[2*i];
	}

	widestring[i] = '\0';
}

void ws_atows(char *string, const size_t length)
{
	int i;

	check(string != NULL);
	
	for(i=length/2 - 1; i >= 0; i--)
	{
		string[2*i] = string[i];
		string[2*i+1] = '\0';
	}
}

void *ws_memswapcpy(void *dest, void *src, const size_t len)
{
	size_t i;

	check(dest != NULL);
	check(src != NULL);

	for(i=0; i < len; i++)
	{
		((char *)dest)[i] = ((char *)src)[len-i-1];
	}

	return dest;
}

