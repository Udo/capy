#include <stdio.h>
#include "../lib/types.h"
#include "../lib/tokenizer.h"
#include "../lib/parser.h"
#include "../lib/emit_ir.h"

int main(int argc, char **argv) {
	default_arena = arena_create(1024*1024*64);
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
		exit(1);
	}

	tokenizer_state* t = tokenize(string_read_file(argv[1]), argv[1]);
	parser_state* p = parse(t);
    string* ir = emit_ir(p);
    printf("%s", string_cstr(ir));
    string_write_file(ir, "bin/example.ll");

    arena_print_stats(default_arena);
	return 0;
}
