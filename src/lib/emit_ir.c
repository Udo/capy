#include "types.h"
#include "tokenizer.h"
#include "parser.h"

typedef struct emitter_state emitter_state;
struct emitter_state
{
	parser_state* p;
    string* out;
};

static void emit_external(emitter_state* e, char* return_type, char* name, char* args)
{
    string_append_cstr(e->out, "declare ");
    string_append_cstr(e->out, return_type);
    string_append_cstr(e->out, " @");
    string_append_cstr(e->out, name);
    string_append_cstr(e->out, "(");
    string_append_cstr(e->out, args);
    string_append_cstr(e->out, ") nounwind\n");
}

static void emit_vector(emitter_state* e, char* name, char* storage_tags, char* element_type, u64 element_size, u64 size, void* data)
{
    string_append_cstr(e->out, "@");
    string_append_cstr(e->out, name);
    string_append_cstr(e->out, " = ");
    string_append_cstr(e->out, storage_tags);
    string_append_cstr(e->out, " [");
    string_append_u64(e->out, size);
    string_append_cstr(e->out, " x ");
    string_append_cstr(e->out, element_type);
    string_append_cstr(e->out, "] [");
    
    for (u64 i = 0; i < size; i++) {
        if (i > 0) {
            string_append_cstr(e->out, ", ");
        }
        string_append_cstr(e->out, " i8 u");
        // handle size 1, 4, 8 - emit as hex-encoded with string_append_cstr(e->out, encoded_value);
        string_append_hex(e->out, ((u8*)data)[i * element_size], element_size * 2);
    }
    
    string_append_cstr(e->out, "]\n");
}

static void emit_string(emitter_state* e, char* name, char* content)
{
    u64 len = strlen(content);
    emit_vector(e, name, "private constant", "i8", 1, len + 1, (u8*)content);
}

static void emit_function_header(emitter_state* e, char* return_type, char* name, char* args)
{
    string_append_cstr(e->out, "define ");
    string_append_cstr(e->out, return_type);
    string_append_cstr(e->out, " @");
    string_append_cstr(e->out, name);
    string_append_cstr(e->out, "(");
    string_append_cstr(e->out, args);
    string_append_cstr(e->out, ") {\n");
    string_append_cstr(e->out, "entry:\n");
}

static void emit_function_footer(emitter_state* e, char* return_type, char* return_value)
{
    string_append_cstr(e->out, "  br label %exit\n");
    string_append_cstr(e->out, "exit:\n");
    string_append_cstr(e->out, "  ret ");
    string_append_cstr(e->out, return_type);
    string_append_cstr(e->out, " ");
    string_append_cstr(e->out, return_value);
    string_append_cstr(e->out, "\n");
    string_append_cstr(e->out, "}\n");
}

string* emit_ir(parser_state* p)
{
    emitter_state* e = arena_alloc(default_arena, sizeof(emitter_state));
    e->out = string_create(1024);

    string_append_cstr(e->out, "; ModuleID = 'hello_world'\n");
    string_append_cstr(e->out, "target triple = \"x86_64-pc-linux-gnu\"\n");
    string_append_cstr(e->out, "\n");
    //emit_vector(e, ".str", "private constant", "i8", 1, 14, (u8*)"Hello, Capy!\0");
    emit_string(e, ".str", "Hello, Capy!\0");
    emit_external(e, "i32", "puts", "i8* nocapture");
    string_append_cstr(e->out, "\n");
    emit_function_header(e, "i32", "main", "");
    string_append_cstr(e->out, "  %call = call i32 @puts(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str, i32 0, i32 0))\n");
    emit_function_footer(e, "i32", "0");

    return e->out;
}