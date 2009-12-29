/**
 * @file mp3_id3.h
 * @brief Header - Mp3 file format, id3 tags
 * @author Julien Blitte
 * @version 0.1
 */
#ifndef INC_MP3_ID3_H
#define INC_MP3_ID3_H

/* options: -lid3tag */

/** @brief magic value returned when an error occurs (string) */
#define ID3_STRING_ERROR	(NULL)
/** @brief magic value returned when an error occurs (integer) */
#define ID3_INT_ERROR	((unsigned)-1)

/** @brief number of music genres */
#define ID3_NB_GENRES	127
/** @brief music genres defined */
extern const char *id3_genres[ID3_NB_GENRES];

/**
 * @brief get a id3 tag value (string)
 * @param tag pointer to tag structure
 * @param frameid the value to get - ascii
 * @return tag value
 */
char *id3_get_string(struct id3_tag *tag, char const *frameid);
/**
 * @brief get a id3 tag value (string)
 * @param tag pointer to tag structure
 * @param frameid the value to get - ascii
 * @return tag value
 */
unsigned long id3_get_int(struct id3_tag *tag, char const *frameid);

#endif
