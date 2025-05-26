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
	TOK_NUMBER = 'N', 
	TOK_QLITERAL = 'Q', 
	TOK_UNKNOWN = '?',
} token_type;

typedef struct {
    char* op;
    int prec;
} operator_table_def;

extern operator_table_def operator_table[];

typedef struct token token;
struct token
{
	u8 type;
	string* content;
	u32 indent;
	u8 is_first_on_line;
	u32 hblockid;
	u64 src_pos;
	string* src; 
	token* next;
};

typedef struct tokenizer_state tokenizer_state;
struct tokenizer_state
{
	token* tokens;
	string* src;
	string* filename;
};

int is_operator_prefix(const char* candidate) ;

u8 tok_get_char_type(u8 c);

typedef struct code_position code_position;
struct code_position
{
	u64 line;
	u64 column;
};

code_position get_code_position(string* content, u64 pos);

#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_LIGHTBLUE "\x1b[94m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_LIGHTRED "\x1b[91m"
#define ANSI_COLOR_GRAY    "\x1b[90m"
#define ANSI_COLOR_WHITE   "\x1b[97m"
#define ANSI_COLOR_BOLD    "\x1b[1m"
#define ANSI_COLOR_DIM     "\x1b[2m"
#define ANSI_COLOR_UNDER   "\x1b[4m"
#define ANSI_COLOR_BLINK   "\x1b[5m"
#define ANSI_COLOR_INV     "\x1b[7m"
#define ANSI_COLOR_RESET   "\x1b[0m"

void pretty_print_lineatpos(string* content, u64 pos);

token* new_token(u8 ctype, token* prev, u64 pos, string* src);

void tok_print_repeat(char* str, u32 times);

void tok_print_single(token* token_current);

void tok_print(token* token_current);

token* tokenizer_cleanup(token* token_list);

tokenizer_state* tokenize(string* content, char* filename);

#endif /* SRC_LIB_TOKENIZER_H */
