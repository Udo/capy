#include "types.h"

typedef struct token token;

struct token
{
	char* s;
	token* next;
};

char* tokenizer_test()
{
	print_stack_trace();
	return "LET'S GO";
}

