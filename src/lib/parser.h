/* Auto-generated header from parser.c */
/* DO NOT EDIT MANUALLY */
#ifndef SRC_LIB_PARSER_H
#define SRC_LIB_PARSER_H

#include "types.h"
#include "tokenizer.h"

typedef enum {
 AST_KEYWORD,
 AST_LIT_NUMBER,
 AST_LIT_STRING,
 AST_IDENTIFIER,
 AST_OP_BINARY,
 AST_OP_UNARY,
 AST_EXPRESSION,
 AST_ASSIGNMENT,
 AST_DECL_TYPE_PRIMITIVE,
 AST_DECL_TYPE_STRUCT, 
 AST_DECL_TYPE_STRUCT_VARIANT,
 AST_DECL_TYPE_ENUM,
 AST_DECL_TYPE_FUNCTION,
 AST_DECL_VAR,
 AST_DECL_BLOCK,
 AST_TYPE,
 AST_TYPE_PATTERN_MATCH,
 AST_CALL,
 AST_BLOCK,
 AST_MODULE,
} ast_node_type;
typedef struct ast_node ast_node;
struct ast_node {
 token* t;
 ast_node_type node_type;
 ast_node* next;
 ast_node* left;
 ast_node* right;
 ast_node* children;
 ast_node* type;
 string* identifier;
 string* lit_value;
};
typedef struct parser_state parser_state;
struct parser_state {
 tokenizer_state* tok;
 token* current_token;
 u32 current_indent;
 u8 is_fresh_line;
 ast_node* module_root;
 ast_node* current_node;
};
ast_node* parser_create_node(ast_node_type node_type, token* tok);
char* get_ast_node_type_name(ast_node_type type);
void debug_ast_node(ast_node* node, int indent);
void token_unexpected(parser_state* p, char* msg);
token* get_token(parser_state* p, u32 steps);
token* consume_whitespace(parser_state* p);
token* consume_token(parser_state* p);
token* consume_token_expect(parser_state* p, u8 ttype, char* content);
bool match_token(parser_state* p, u32 steps, u8 ttype, char* content);
ast_node* parser_append_node(ast_node* parent, ast_node* new_node);
ast_node* parse_call(parser_state* p);
ast_node* parse_statement(parser_state* p);
void parse_block(parser_state* p);
bool is_end_of_expression(parser_state* p);
typedef struct {
 char* op;
 int prec;
} op_prec_t;
extern op_prec_t op_precs[];
int get_precedence(token* tok);
ast_node* parse_expression_rbp(parser_state* p, int rbp);
ast_node* parse_expression(parser_state* p);
void parse_argument_list(parser_state* p, ast_node* parent);
ast_node* parse_call(parser_state* p);
ast_node* parse_statement(parser_state* p);
void parse_block(parser_state* p);
parser_state* parse(tokenizer_state* tok_state);

#endif /* SRC_LIB_PARSER_H */
