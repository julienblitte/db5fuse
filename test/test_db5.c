#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include  "file.h"
#include  "db5_hdr.h"
#include  "db5_dat.h"
#include  "db5.h"
#include  "db5_idx.h"
#include  "names.h"


int main(int argc, char *argv[])
{
	uint32_t i, count;
	db5_row row;

	assert(argc == 2);

	assert(file_set_context(argv[1]) == true);

	assert(db5_hdr_init() == true);
	assert(db5_dat_init() == true);
	assert(names_init() == true);

	count = db5_hdr_count();
	for(i=0; i < count; i++)
	{
		assert(db5_dat_select_row(i, &row) == true);
		db5_unwidechar_row(&row);
		db5_print_row(&row);
	}

	printf("locate 'a5ebec2.wma': %d\n", db5_dat_select_by_filename("a5ebec2.wma"));
	

	db5_index();

	names_free();
	db5_dat_free();
	db5_hdr_free();

	return EXIT_SUCCESS;
}
