/* Auto-generated header from parser.c */
/* DO NOT EDIT MANUALLY */
#ifndef SRC_LIB_PARSER_H
#define SRC_LIB_PARSER_H

#include "types.h"
#include "tokenizer.h"

#define CLOSING_BRACE "}" 

typedef enum {
	AST_DIRECTIVE,
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
typedef struct symbol symbol;
typedef struct symbol_table symbol_table;

struct ast_node
{
	token* t;
	ast_node_type node_type;
	ast_node* next;
	ast_node* left;
	ast_node* right;
	ast_node* children;
	ast_node* type;
	symbol_table* symbol_table;
	string* identifier;
	string* lit_value;
};

struct symbol
{
	string* name;
	ast_node* type;
	symbol* next;
};

struct symbol_table
{
	token* scope_start;
	symbol_table* parent;
	symbol* symbols;
	symbol* _last_symbol; 
};

typedef struct parser_state parser_state;
struct parser_state
{
	tokenizer_state* tok;
	token* current_token;
	token* last_token;
	ast_node* module_root;
	ast_node* current_node;
	symbol_table* current_symbol_table;
};


char* get_ast_node_type_name(ast_node_type type) ;

void debug_ast_node(ast_node* node, int indent, symbol_table* st) ;

void token_unexpected(parser_state* p, char* msg);

token* get_token(parser_state* p, u32 steps) ;

token* consume_whitespace(parser_state* p);

token* consume_token(parser_state* p);

token* consume_token_expect(parser_state* p, u8 ttype, char* content);

bool match_token(parser_state* p, u32 steps, u8 ttype, char* content);

void token_expect(parser_state* p, u8 ttype, char* content);

ast_node* parser_create_node(parser_state* p, ast_node_type type, token* t);

symbol_table* parser_create_symbol_table(parser_state* p);

symbol* parser_find_symbol_by_name(parser_state* p, string* name, bool recursive);

symbol* parser_create_symbol(parser_state* p, string* name, ast_node* type);

ast_node* parse_call(parser_state* p);
ast_node* parse_statement(parser_state* p);
ast_node* parse_block(parser_state* p);
ast_node* parse_declaration(parser_state* p);

bool is_end_of_expression(parser_state* p);

int get_precedence(token* tok) ;

void check_identifier(parser_state* p, string* id);






ast_node* parse_expression_rbp(parser_state* p, int rbp) ;

ast_node* parse_expression(parser_state* p) ;

void parse_argument_list(parser_state* p, ast_node* parent);


ast_node* parse_directive(parser_state* p);


ast_node* parse_primitive_type(parser_state* p);
ast_node* parse_struct_type(parser_state* p);
ast_node* parse_enum_type(parser_state* p);
ast_node* parse_function_or_grouped_type(parser_state* p);

ast_node* parse_type_expression(parser_state* p, char* stop_at);








void declare_symbol(parser_state* p, string* name, ast_node* type);

parser_state* parse(tokenizer_state* tok_state) ;

#endif /* SRC_LIB_PARSER_H */
