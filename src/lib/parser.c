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

ast_node* parser_create_node(parser_state* p, ast_node_type type) 
{
	ast_node* node = arena_alloc(default_arena, sizeof(ast_node));
	node->id = p->id_counter++;
	node->t = p->current_token;
	node->node_type = type;
	return node;
}

void debug_print_nodetype(ast_node_type type) 
{
	switch(type) {
		case AST_MODULE: printf("MODULE"); break;
		case AST_BLOCK: printf("BLOCK"); break;
		case AST_EXPRESSION: printf("EXPRESSION"); break;
		case AST_FNCALL: printf("FNCALL"); break;
		case AST_PARAM: printf("PARAM"); break;
		case AST_REF: printf("REF"); break;
		case AST_STRINGLITERAL: printf("STRINGLITERAL"); break;
		default: printf("UNKNOWN");
	}
}

void parser_next_token(parser_state* p) 
{
	if (p->current_token) {
		p->last_token = p->current_token;
		p->current_token = p->current_token->next;
	}
}

int parser_match(parser_state* p, s32 look_ahead, s32 token_type, char* equals_content) 
{
	token* t = p->current_token;
	while (look_ahead > 0 && t) {
		t = t->next;
		look_ahead--;
	}
	return t && t->type == token_type && 
	       (equals_content == 0 || string_equals_cstr(t->content, equals_content, true));
}

void parser_expect(parser_state* p, s32 look_ahead, s32 token_type, char* equals_content)
{
	if(!equals_content) equals_content = "-";
	s32 match = parser_match(p, look_ahead, token_type, equals_content);
	if (!match) {
		code_position pos = get_code_position(p->current_token->src, p->current_token->src_pos);
		fprintf(stderr, ANSI_COLOR_RED "Error: Expected token type %c/%s at %lu:%lu\n" ANSI_COLOR_RESET,
				token_type, equals_content, pos.line, pos.column);
		exit(1);
	}
}

void parser_error(parser_state* p, const char* message) 
{
	code_position pos = get_code_position(p->current_token->src, p->current_token->src_pos);
	fprintf(stderr, ANSI_COLOR_RED "Error: %s at line %lu col %lu at '%s'\n" ANSI_COLOR_RESET,
			message, pos.line, pos.column, p->current_token->content->data);
	exit(1);
}

ast_node* parse_expression(parser_state* p);

ast_node* parser_append_child(ast_node* parent, ast_node* child) 
{
	if (!parent->children) {
		parent->children = child;
	} else {
		parent->children_last->next = child;
	}
	parent->children_last = child;
	child->next = NULL;
	return child;
}

ast_node* parse_function_call(parser_state* p)
{
	ast_node* fncall = parser_create_node(p, AST_FNCALL);
	fncall->identifier = p->current_token->content;
	parser_next_token(p);
	parser_expect(p, 0, TOK_PUNCT, "(");
	parser_next_token(p);
	s32 param_counter = 0;
	while(p->current_token && !parser_match(p, 0, TOK_PUNCT, ")"))
	{
		ast_node* param = parser_create_node(p, AST_PARAM);
		param->counter = param_counter++;
		parser_append_child(fncall, param);
		param->rvalue = parse_expression(p);
		if(!parser_match(p, 0, TOK_PUNCT, ",") && !parser_match(p, 0, TOK_PUNCT, ")"))
		{
			parser_error(p, "unexpected token in parameter list");
		}
		if(parser_match(p, 0, TOK_PUNCT, ","))
			parser_next_token(p);
	}
	parser_expect(p, 0, TOK_PUNCT, ")");
	parser_next_token(p);
	return fncall;
}

ast_node* parser_get_or_create_literal(parser_state* p, token* t) 
{
	ast_node* new_lit = parser_create_node(p, AST_STRINGLITERAL);
	new_lit->identifier = t->content;
	new_lit->t = t;
	new_lit->next = p->module_root->literals;
	if(p->module_root->literals) 
		p->module_root->literals->next = new_lit;
	else 
		p->module_root->literals = new_lit;
	return new_lit;
}

ast_node* parse_expression(parser_state* p) 
{
	ast_node* expr = parser_create_node(p, AST_EXPRESSION);
	if(parser_match(p, 0, TOK_ALPHA, 0)) 
	{
		if(parser_match(p, 1, TOK_PUNCT, "(")) // function call
		{
			parser_append_child(expr, parse_function_call(p));
		}
	}  
	else if(parser_match(p, 0, TOK_QLITERAL, 0)) 
	{
		ast_node* val = parser_append_child(expr, parser_create_node(p, AST_REF));
		val->ref = parser_get_or_create_literal(p, p->current_token);
		parser_next_token(p);
	} 
	else 
	{
		parser_error(p, "Unexpected token in expression");
	}
	return expr;
}

ast_node* parse_block(parser_state* p) 
{
	ast_node* block = parser_create_node(p, AST_BLOCK);

	while (p->current_token) 
	{
		if(parser_match(p, 0, TOK_ALPHA, 0))
		{
			parser_append_child(block, parse_expression(p));
		}
		else
		{
			parser_error(p, "Unexpected token in block");
		}
	}

	return block;
}

parser_state* parse(tokenizer_state* tok_state) 
{
	parser_state* p = arena_alloc(default_arena, sizeof(parser_state));
	p->tok = tok_state;
	p->current_token = tok_state->tokens;
	p->current_node = p->module_root = parser_create_node(p, AST_MODULE);
	p->current_node->t = tok_state->tokens;
	ast_node* blocks = p->module_root->children = parse_block(p);
	while(p->current_token)
	{
		blocks->next = parse_block(p);
		blocks = blocks->next;
	}
	return p;
}

void debug_ast_node(ast_node* node, s32 indent, s32 sameline) 
{
	if (!node) return;
	code_position pos = get_code_position(node->t->src, node->t->src_pos);
	if(!sameline)
	{
		printf(ANSI_COLOR_BLUE "%05lu | " ANSI_COLOR_RESET, pos.line);
		for(s32 i = 0; i < indent; i++)
			printf("  ");
	}
	debug_print_nodetype(node->node_type);
	if(node->node_type == AST_MODULE || node->node_type == AST_BLOCK || node->node_type == AST_EXPRESSION || node->node_type == AST_FNCALL)
	{
		if(node->identifier)
			printf(" %s", string_cstr(node->identifier));
		printf("\n");
		if(node->literals)
		{
			ast_node* lit = node->literals;
			while(lit)
			{
				printf(ANSI_COLOR_GREEN "      | #%lu ", lit->id);
				debug_print_nodetype(lit->node_type);
				printf(" '%s'\n" ANSI_COLOR_RESET, string_cstr(lit->identifier));
				lit = lit->next;
			}
		}
		ast_node* c = node->children;
		while(c)
		{
			debug_ast_node(c, indent+1, 0);
			c = c->next;
		}
	}
	else if(node->node_type == AST_PARAM)
	{
		printf(" %lu ", node->counter);
		debug_ast_node(node->rvalue, indent+1, 1);
	}
	else if(node->node_type == AST_REF)
	{
		printf(" #%lu \n", node->ref->id);
	}
	else if(node->node_type == AST_STRINGLITERAL)
	{
		printf(" '%s' ", string_cstr(node->identifier));
		debug_ast_node(node->rvalue, indent+1, 1);
		printf("\n");
	}
	else
	{
		printf("\n");
	}
}