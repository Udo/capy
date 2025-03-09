#include "types.h"
#include "tokenizer.h"
#include "parser.h"
#include "hashmap.h"

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

interpreter_scope* interpreter_create_scope(interpreter_scope* parent)
{
    interpreter_scope* scope = arena_alloc(default_arena, sizeof(interpreter_scope));
    scope->parent = parent;
    scope->values = hashmap_create(32);
    return scope;
}

void interpret_node(interpreter_state* i, ast_node* n)
{
    if(!n)
        return;
    switch(n->node_type)
    {
        case AST_BLOCK:
        {
            break;
        }
        case AST_DECL_VAR:
        {
            break;
        }
        case AST_ASSIGNMENT:
        {
            break;
        }
        case AST_CALL:
        {
            break;
        }
        default:
            break;
    }
}

void interpret_ast(parser_state* p) 
{
    interpreter_state* i = arena_alloc(default_arena, sizeof(interpreter_state));
    i->p = p;
    i->root_scope = interpreter_create_scope(0);
    interpret_node(i, p->module_root);
}