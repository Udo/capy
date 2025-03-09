/* Auto-generated header from interpreter.c */
/* DO NOT EDIT MANUALLY */
#ifndef SRC_LIB_INTERPRETER_H
#define SRC_LIB_INTERPRETER_H

#include "types.h"
#include "tokenizer.h"
#include "parser.h"

typedef struct interpreter_scope interpreter_scope;
struct interpreter_scope
{
    interpreter_scope* parent;
    symbol* symbols;
    
};

typedef struct interpreter_state interpreter_state;
struct interpreter_state
{
    parser_state* p;

};

void interpret_ast(parser_state* p);

#endif /* SRC_LIB_INTERPRETER_H */
