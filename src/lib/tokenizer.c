#include "types.h"

typedef struct token token;

struct token
{
	char* s;
	token* next;
};

char* tokenizer_test()
{
	return "LET'S GO";
}

