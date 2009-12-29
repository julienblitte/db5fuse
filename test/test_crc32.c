#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "crc32.h"

int main(int argc, char *argv[])
{
	uint32_t crc;

	crc32_init();

	assert(argc == 2);

	crc = crc32_file(argv[1]);

	printf("crc32: %x\n", crc);

	return EXIT_SUCCESS;
}
