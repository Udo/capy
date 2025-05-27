/* Auto-generated header from parser.c */
/* DO NOT EDIT MANUALLY */
#ifndef SRC_LIB_PARSER_H
#define SRC_LIB_PARSER_H

#include "types.h"
#include "tokenizer.h"

#define CLOSING_BRACE "}" 

typedef enum {
    AST_MODULE,
    AST_BLOCK,
    AST_EXPRESSION,
	AST_FNCALL,
	AST_PARAM,
	AST_STRINGLITERAL,
	AST_REF,
} ast_node_type;

typedef struct ast_node ast_node;
struct ast_node
{
	token* t;
	s32 node_type;
	ast_node* next;
	ast_node* children;
	ast_node* children_last;
	ast_node* literals;
	ast_node* ref;
	string* identifier;
	u64 counter;
	u64 id;
	ast_node* rvalue;
};

typedef struct parser_state parser_state;
struct parser_state
{
	tokenizer_state* tok;
	token* current_token;
	token* last_token;
	ast_node* module_root;
	ast_node* current_node;
	u64 id_counter;
};

ast_node* parser_create_node(parser_state* p, ast_node_type type);

void debug_print_nodetype(ast_node_type type);

void parser_next_token(parser_state* p);

int parser_match(parser_state* p, s32 look_ahead, s32 token_type, char* equals_content);

void parser_expect(parser_state* p, s32 look_ahead, s32 token_type, char* equals_content);

void parser_error(parser_state* p, const char* message);

ast_node* parse_expression(parser_state* p);

ast_node* parser_append_child(ast_node* parent, ast_node* child);

ast_node* parse_function_call(parser_state* p);

ast_node* parser_get_or_create_literal(parser_state* p, token* t);


ast_node* parse_block(parser_state* p);

parser_state* parse(tokenizer_state* tok_state);

void debug_ast_node(ast_node* node, s32 indent, s32 sameline);

#endif /* SRC_LIB_PARSER_H */
