/* Auto-generated header from tokenizer.c */
/* DO NOT EDIT MANUALLY */
#ifndef SRC_LIB_TOKENIZER_H
#define SRC_LIB_TOKENIZER_H

#include "types.h"

typedef struct token token;
struct token {
 char* s;
 token* next;
}
;
char* tokenizer_test();

#endif /* SRC_LIB_TOKENIZER_H */
