#include <assert.h>
#include <stdlib.h>
#include "names.h"
#include "file.h"

#define TEST_NAME	"My song name éè®€ÿæ.mp3"

int main(int argc, char *argv[])
{
	assert(argc == 2);
	assert(file_set_context(argv[1]) == true);

	names_init();
	names_print();
	names_insert(TEST_NAME);
	names_delete(TEST_NAME);
	names_save();
	names_free();

	return EXIT_SUCCESS;
}

