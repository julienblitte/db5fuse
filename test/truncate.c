#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		fprintf(stderr, "Usage: truncate <file> <new_size>\n");
		return EXIT_FAILURE;
	}

	if(truncate(argv[1], atoi(argv[2])) != 0)
	{
		fprintf(stderr, "Error: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
