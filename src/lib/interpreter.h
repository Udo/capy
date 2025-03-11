/* Auto-generated header from interpreter.c */
/* DO NOT EDIT MANUALLY */
#ifndef SRC_LIB_INTERPRETER_H
#define SRC_LIB_INTERPRETER_H

#include "types.h"
#include "tokenizer.h"
#include "parser.h"
#include "hashmap.h"
#include "builtin.h"

typedef struct interpreter_scope interpreter_scope;
struct interpreter_scope
{
    interpreter_scope* parent;
    symbol_table* symbol_table;
    hashmap* values;    
};

typedef struct interpreter_state interpreter_state;
struct interpreter_state
{
    parser_state* p;
    interpreter_scope* root_scope;
};

interpreter_scope* interpreter_create_scope(interpreter_scope* parent);

void interpret_node(interpreter_state* i, ast_node* n);

void interpret_ast(parser_state* p);

#endif /* SRC_LIB_INTERPRETER_H */
