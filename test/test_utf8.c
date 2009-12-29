#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "utf8.h"

int main(int argc, char *argv[])
{
	char buffer[1024];

	assert(argc == 2);

	printf("initial:%s\n", argv[1]);
	
	iso8859_utf8(argv[1], buffer, sizeof(buffer));
	printf("to utf8:%s\n", argv[1]);

	utf8_iso8859(argv[1], buffer, sizeof(buffer));
	printf("to iso8859:%s\n", argv[1]);

	return EXIT_SUCCESS;
}
