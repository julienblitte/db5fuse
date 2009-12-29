#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "asf.h"
#include "db5.h"
#include "db5_types.h"

int main(int argc, char *argv[])
{
	db5_row row;

	assert(argc == 2);

	asf_generate_row(argv[1], &row);
	db5_print_row(&row);

	return EXIT_SUCCESS;
}
