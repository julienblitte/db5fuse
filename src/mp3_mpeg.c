/**
 * @file mp3_mpeg.c
 * @brief Source - Mp3 file format, data
 * @author Julien Blitte
 * @version 0.1
 */
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "check.h"
#include "file.h"
#include "mp3_mpeg.h"

/* http://www.mpgedit.org/mpgedit/mpeg_format/mpeghdr.htm */

/**
 * @brief bitrate by layer, version and bitrate index
 */
const unsigned int bitrate_index[16][2][3] =
{
	{ {0  , 0  , 0  }, { 0  , 0  , 0   } },
	{ {32 , 32 , 32 }, { 32 , 8  , 8   } },
	{ {64 , 48 , 40 }, { 48 , 16 , 16  } },
	{ {96 , 56 , 48 }, { 56 , 24 , 24  } },
	{ {128, 64 , 56 }, { 64 , 32 , 32  } },
	{ {160, 80 , 64 }, { 80 , 40 , 40  } },
	{ {192, 96 , 80 }, { 96 , 48 , 48  } },
	{ {224, 112, 96 }, { 112, 56 , 56  } },
	{ {256, 128, 112}, { 128, 64 , 64  } },
	{ {288, 160, 128}, { 144, 80 , 80  } },
	{ {320, 192, 160}, { 160, 96 , 96  } },
	{ {352, 224, 192}, { 176, 112, 112 } },
	{ {384, 256, 224}, { 192, 128, 128 } },
	{ {416, 320, 256}, { 224, 144, 144 } },
	{ {448, 384, 320}, { 256, 160, 160 } },
	{ {0  , 0  , 0  }, { 0  , 0  , 0   } },
};

/**
 * @brief bitrate by version and samplerate index
 */
const unsigned int samplerate_index[4][3] =
{
	{44100, 22050, 11025 },
	{48000, 24000, 12000 },
	{32000, 16000, 8000  },
	{0,     0,     0     }
};

ptrdiff_t mp3_next_frame(char *buffer, size_t len)
{
	size_t result;

	check(buffer != NULL);
	check(sizeof(mp3_frame) >= 2);

	result = 0;
	while(result < (len - sizeof(mp3_frame)))
	{
		if ((buffer[result] & 0xFF) == 0xFF)
		{
			if ((buffer[result+1] & 0xE0) == 0xE0)
			{
				return result;
			}
		}
		result++;
	}

	return len;
}

/**
 * @brief return a mpeg audio frame version
 * @param frame mpeg audio frame to analyze
 * @return the version of frame
 */
static unsigned int mp3_version(mp3_frame *frame)
{
	check(frame != NULL);

	switch (frame->version)
	{
		case MP3_VER_1:
			return 1;
			break;
		case MP3_VER_2:
		case MP3_VER_2_5:
			return 2;
			break;
	}
	return 0;
}

/**
 * @brief return a mpeg audio frame coding layer
 * @param frame mpeg audio frame to analyze
 * @return the coding layer of frame, it's mp3 if result is 3
 */
static unsigned int mp3_layer(mp3_frame *frame)
{
	check(frame != NULL);

	switch (frame->layer)
	{
		case MP3_LAYER_1:
			return 1;
			break;
		case MP3_LAYER_2:
			return 2;
			break;
		case MP3_LAYER_3:
			return 3;
			break;
	}
	return 0;
}

unsigned int mp3_bitrate(mp3_frame *frame)
{
	check(frame != NULL);

	return 1000 * bitrate_index[frame->bitrate][mp3_version(frame)-1][mp3_layer(frame)-1];
}

unsigned int mp3_samplerate(mp3_frame *frame)
{
	check(frame != NULL);

	switch (frame->version)
	{
		case MP3_VER_1:
			return samplerate_index[frame->samplerate][0];
			break;
		case MP3_VER_2:
			return samplerate_index[frame->samplerate][1];
			break;
		case MP3_VER_2_5:
			return samplerate_index[frame->samplerate][2];
			break;
	}
	return 0;
}

#ifdef FRAMESIZE
/**
 * @brief get the size of an mpeg audio frame
 * @param frame frame which to get size
 * @return the size of frame
 */
unsigned int mp3_framesize(mp3_frame *frame)
{
	unsigned int bitrate;
	unsigned int samplerate;

	check(frame != NULL);

	bitrate = mp3_bitrate(frame);
	samplerate = mp3_samplerate(frame);

	switch (frame->layer)
	{
		case MP3_LAYER_1:
			return 4*((12 * bitrate)/samplerate + frame->padding);
			break;
		case MP3_LAYER_2:
		case MP3_LAYER_3:
			return (144 * bitrate)/samplerate + frame->padding;
			break;
	}
	return 0;
}
#endif

unsigned int mp3_length(mp3_frame *frame, unsigned long filesize)
{
	check(frame != NULL);

	return (filesize/(mp3_bitrate(frame)/8));
}

