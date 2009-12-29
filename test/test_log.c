#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "logger.h"
#include "config.h"

int main(int argc, char *argv[])
{
	open_log();

	if (add_log(ADDLOG_DUMP, "test", "aleat value=%lu\n", random()))
	{
		printf("ok: %s\n", CONFIG_LOG_FILENAME);
	}
	else
	{
		printf("nok!\n");
	}

	close_log();

	return EXIT_SUCCESS;
}
