/**
 * @file mp3_mpeg.h
 * @brief Header - Mp3 file format, data
 * @author Julien Blitte
 * @version 0.1
 */
#ifndef INC_MP3_MPEG_H
#define INC_MP3_MPEG_H

#include <stddef.h>

/* http://www.mpgedit.org/mpgedit/mpeg_format/mpeghdr.htm */

/**
 * @brief mpeg audio frame format
 */
typedef struct mp3_frame_t
{
	/** @brief emphasis */
	unsigned int emphasis : 2;
	/** @brief original media */
	unsigned int original : 1;
	/** @brief copyrighted */
	unsigned int copyright : 1;
	/** @brief mode extension */
	unsigned int mode : 2;
	/** @brief channel mode */
	unsigned int channel : 2;
	/** @brief for proprietary use */
	unsigned int private : 1;
	/** @brief padding */
	unsigned int padding : 1;
	/** @brief sample index */
	unsigned int samplerate : 2;
	/** @brief bitrate index */
	unsigned int bitrate : 4;
	/** @brief crc protected */
	unsigned int crc : 1;
	/** @brief layer 1 */
	#define MP3_LAYER_1	3
	/** @brief layer 2 */
	#define MP3_LAYER_2	2
	/** @brief layer 3 */
	#define MP3_LAYER_3	1
	/** @brief mpeg layer */
	unsigned int layer : 2;
	/** @brief version 1 */
	#define MP3_VER_1	3
	/** @brief version 2 */
	#define MP3_VER_2	2
	/** @brief version 2.5 */
	#define MP3_VER_2_5	0
	/** @brief mpeg version */
	unsigned int version : 2;
	/** @brief synchronisation */
	unsigned int magic : 11;
} mp3_frame;


/**
 * @brief locate the next mpeg frame in a buffer
 * @param buffer buffer in which to locate next frame - binary
 * @param len size of buffer
 * @return next frame in buffer
 */
ptrdiff_t mp3_next_frame(char *buffer, const size_t len); 

/**
 * @brief return a mpeg audio frame bitrate
 * @param frame mpeg audio frame to analyze
 * @return the bitrate of frame
 */
unsigned int mp3_bitrate(mp3_frame *frame); 

/**
 * @brief return a mpeg audio frame samplerate
 * @param frame mpeg audio frame to analyze
 * @return the samplerate of frame
 */
unsigned int mp3_samplerate(mp3_frame *frame); 

/**
 * @brief estimate the duration of an mpeg audio media using a frame and filesize
 * @param frame mpeg audio frame used to compute media duration
 * @param filesize filesize used to compute media duration
 * @return estimated media duration
 */
unsigned int mp3_length(mp3_frame *frame, const unsigned long filesize);

#endif
