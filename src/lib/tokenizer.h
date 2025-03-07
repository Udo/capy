/* Auto-generated header from tokenizer.c */
/* DO NOT EDIT MANUALLY */
#ifndef SRC_LIB_TOKENIZER_H
#define SRC_LIB_TOKENIZER_H

#include "types.h"

typedef enum {
 TOK_NEWLINE = 'L', 
 TOK_INDENT = 'I', 
 TOK_SPACE = 'S', 
 TOK_PUNCT = 'P', 
 TOK_ALPHA = 'A', 
 TOK_COMMENT = '#', 
 TOK_NLITERAL = 'N', 
 TOK_QLITERAL = 'Q', 
 TOK_UNKNOWN = '?',
} token_type;
typedef struct token token;
struct token {
 u8 type;
 string* content;
 u32 hblockid; 
 u64 src_pos;
 token* next;
};
typedef struct tokenizer_state tokenizer_state;
struct tokenizer_state {
 token* tokens;
 string* src;
 string* filename;
};
u8 tok_get_char_type(u8 c);
token* new_token(u8 ctype, token* prev, u64 pos);
void tok_print_repeat(char* str, u32 times);
void tok_print_single(token* token_current);
void tok_print(token* token_current);
tokenizer_state* tokenize(string* content, char* filename);

#endif /* SRC_LIB_TOKENIZER_H */
