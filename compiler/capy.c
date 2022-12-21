#include <stdio.h>
#include "types.c"
#include "utils.c"
#include "parser.c"

int main(int argc, char** argv) {

	if(argc <= 1)
		return 0;

	// to do: check if file exists

	struct string* src_filename = string_create_with_chars(argv[1]);
	struct string* src = file_get_contents(src_filename);
	parse(src);

    return 0;
}
