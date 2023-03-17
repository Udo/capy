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
	void* t = parse(src);
	print_tokens(t);

	//printf("source file size: %i\n", src->length);

    return 0;
}
