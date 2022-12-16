#include <stdio.h>
#include "types.c"
#include "utils.c"

int main(int argc, char** argv) {

	struct string* s1 = string_create();
	assign_string_chars(s1, "this will probably work");

	printf("hello string '%s' length=%i\n", s1->bytes, s1->length);

    return 0;
}