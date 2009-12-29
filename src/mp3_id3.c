/**
 * @file mp3_id3.c
 * @brief Source - Mp3 file format, id3 tags
 * @author Julien Blitte
 * @version 0.1
 */
#include <id3tag.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "check.h"
#include "mp3_id3.h"
#include "wstring.h"

#ifndef id3_field_getlatin1s
/**
 * @brief id3tag lack: get a latin1 string list item
 * @param field id3 field
 * @param index string index
 * @return string to extract
 */
id3_latin1_t const *id3_field_getlatin1s(union id3_field const *field, unsigned int index)
{
	return field->latin1list.strings[index];
}
#endif

/**
 * @brief duplicate an id3_latin1_t string
 * @param source string to duplicate
 * @return a copy of source (do not forget to free it)
 */
id3_latin1_t *id3_latin1duplicate(id3_latin1_t const *source)
{
	size_t size;
	id3_latin1_t *result;

	check(source != NULL);

	size = sizeof(char) * strlen((char *)source);
	result = (id3_latin1_t *)malloc(size);
	if (result != NULL)
	{
		memcpy(result, source, size);
	}

	check(result != NULL);
	return result;
}

/* http://mpgedit.org/mpgedit/mpeg_format/mpeghdr.htm#MPEGTAG */
/** @brief standard id3 genres */
const char *id3_genres[] = { "Blues", "Classic Rock", "Country", "Dance", "Disco", "Funk", "Grunge", "Hip-Hop",
			"Jazz", "Metal", "New Age", "Oldies", "Other", "Pop", "R&B", "Rap",
			"Reggae", "Rock", "Techno", "Industrial", "Alternative", "Ska", "Death Metal", "Pranks",
			"Soundtrack", "Euro-Techno", "Ambient", "Trip-Hop", "Vocal", "Jazz+Funk", "Fusion", "Trance",
			"Classical", "Instrumental", "Acid", "House", "Game", "Sound Clip", "Gospel", "Noise",
			"AlternRock", "Bass", "Soul", "Punk", "Space", "Meditative", "Instrumental Pop", "Instrumental Rock",
			"Ethnic", "Gothic", "Darkwave", "Techno-Industrial", "Electronic", "Pop-Folk", "Eurodance", "Dream",
			"Southern Rock", "Comedy", "Cult", "Gangsta", "Top 40", "Christian Rap", "Pop/Funk", "Jungle",
			"Native American", "Cabaret", "New Wave", "Psychadelic", "Rave", "Showtunes", "Trailer", "Lo-Fi",
			"Tribal", "Acid Punk", "Acid Jazz", "Polka", "Retro", "Musical", "Rock & Roll", "Hard Rock",
			"Folk", "Folk-Rock", "National Folk", "Swing", "Fast Fusion", "Bebob", "Latin", "Revival",
			"Celtic", "Bluegrass", "Avantgarde", "Gothic Rock", "Progressive Rock", "Psychedelic Rock", "Symphonic Rock", "Slow Rock",
			"Big Band", "Chorus", "Easy Listening", "Acoustic", "Humour", "Speech", "Chanson", "Opera",
			"Chamber Music", "Sonata", "Symphony", "Booty Bass", "Primus", "Porn Groove", "Satire", "Slow Jam",
			"Club", "Tango", "Samba", "Folklore", "Ballad", "Power Ballad", "Rhythmic Soul", "Freestyle",
			"Duet", "Punk Rock", "Drum Solo", "A capella", "Euro-House", "Dance Hall" };

char *id3_get_string(struct id3_tag *tag, char const *frameid)
{
	struct id3_frame *frame;
	union id3_field *field;
	unsigned int i;

	check(tag != NULL);
	check(frameid != NULL);

	frame = id3_tag_findframe(tag, frameid, 0);
	if (frame == NULL)
	{
		return ID3_STRING_ERROR;
	}

	for(i=0; i < frame->nfields; i++)
	{
		field = id3_frame_field(frame, i);
		if (field == NULL)
		{
			return ID3_STRING_ERROR;
		}

		switch(id3_field_type(field))
		{
			case ID3_FIELD_TYPE_TEXTENCODING:
			case ID3_FIELD_TYPE_BINARYDATA:
			case ID3_FIELD_TYPE_LANGUAGE:
			case ID3_FIELD_TYPE_FRAMEID:
			case ID3_FIELD_TYPE_DATE:
				/* ignore */
			break;
			case ID3_FIELD_TYPE_INT8:
			case ID3_FIELD_TYPE_INT16:
			case ID3_FIELD_TYPE_INT24:
			case ID3_FIELD_TYPE_INT32:
			case ID3_FIELD_TYPE_INT32PLUS:
				/* int */
			break;
			case ID3_FIELD_TYPE_LATIN1:
				return (char *)id3_latin1duplicate(id3_field_getlatin1(field));
			break;
			case ID3_FIELD_TYPE_LATIN1FULL:
				return (char *)id3_latin1duplicate(id3_field_getfulllatin1(field));
			break;
			case ID3_FIELD_TYPE_LATIN1LIST:
				return (char *)id3_latin1duplicate(id3_field_getlatin1s(field, 0));
			break;
			case ID3_FIELD_TYPE_STRING:
				return (char *)id3_ucs4_latin1duplicate(id3_field_getstring(field));
			break;
			case ID3_FIELD_TYPE_STRINGFULL:
				return (char *)id3_ucs4_latin1duplicate(id3_field_getfullstring(field));
			break;
			case ID3_FIELD_TYPE_STRINGLIST:
				return (char *)id3_ucs4_latin1duplicate(id3_field_getstrings(field, 0));
			break;
		}
	}

	return ID3_STRING_ERROR;
}

unsigned long id3_get_int(struct id3_tag *tag, char const *frameid)
{
	struct id3_frame *frame;
	union id3_field *field;
	char *string_int;
	unsigned int i;
	unsigned long result;

	check(tag != NULL);
	check(frameid != NULL);

	frame = id3_tag_findframe(tag, frameid, 0);
	if (frame == NULL)
	{
		return ID3_INT_ERROR;
	}

	for(i=0; i < frame->nfields; i++)
	{
		field = id3_frame_field(frame, i);
		if (field == NULL)
		{
			return ID3_INT_ERROR;
		}

		switch(id3_field_type(field))
		{
			case ID3_FIELD_TYPE_TEXTENCODING:
			case ID3_FIELD_TYPE_BINARYDATA:
			case ID3_FIELD_TYPE_LANGUAGE:
			case ID3_FIELD_TYPE_FRAMEID:
			case ID3_FIELD_TYPE_DATE:
				/* ignore */
			break;
			case ID3_FIELD_TYPE_INT8:
			case ID3_FIELD_TYPE_INT16:
			case ID3_FIELD_TYPE_INT24:
			case ID3_FIELD_TYPE_INT32:
			case ID3_FIELD_TYPE_INT32PLUS:
				return id3_field_getint(field);
			break;
			case ID3_FIELD_TYPE_LATIN1:
			case ID3_FIELD_TYPE_LATIN1FULL:
			case ID3_FIELD_TYPE_LATIN1LIST:
			case ID3_FIELD_TYPE_STRING:
			case ID3_FIELD_TYPE_STRINGFULL:
			case ID3_FIELD_TYPE_STRINGLIST:
				/* string */
			break;
		}
	}

	/* no integer field found, try with string */
	string_int = id3_get_string(tag, frameid);
	if (string_int != NULL)
	{
		result = atol(string_int);
		free(string_int);

		return result;
	}

	return ID3_INT_ERROR;
}
