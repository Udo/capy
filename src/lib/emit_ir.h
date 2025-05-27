/* Auto-generated header from emit_ir.c */
/* DO NOT EDIT MANUALLY */
#ifndef SRC_LIB_EMIT_IR_H
#define SRC_LIB_EMIT_IR_H

#include "types.h"
#include "tokenizer.h"
#include "parser.h"

typedef struct emitter_state emitter_state;
struct emitter_state
{
	parser_state* p;
    string* out;
};






void emit_block(emitter_state* e, ast_node* block);

string* emit_ir(parser_state* p);

#endif /* SRC_LIB_EMIT_IR_H */
