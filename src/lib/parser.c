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
    AST_DECL_TYPE_STRUCT, //
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
struct ast_node
{
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
struct parser_state
{
	tokenizer_state* tok;
	token* current_token;
	u32 current_indent;
	u8 is_fresh_line;
	ast_node* module_root;
	ast_node* current_node;
};

ast_node* parser_create_node(ast_node_type node_type, token* tok) {
	ast_node* node = arena_alloc(default_arena, sizeof(ast_node));
	node->t = tok;
	node->node_type = node_type;
	return node;
}

/* Return a string representation of an ast_node_type */
char* get_ast_node_type_name(ast_node_type type) {
	switch (type) {
		case AST_KEYWORD:                  return "KEYWORD";
		case AST_LIT_NUMBER:               return "LIT_NUMBER";
		case AST_LIT_STRING:               return "LIT_STRING";
		case AST_IDENTIFIER:               return "IDENTIFIER";
		case AST_OP_BINARY:                return "OP_BINARY";
		case AST_OP_UNARY:                 return "OP_UNARY";
		case AST_EXPRESSION:               return "EXPRESSION";
		case AST_ASSIGNMENT:               return "ASSIGNMENT";
		case AST_DECL_TYPE_PRIMITIVE:      return "DECL_TYPE_PRIMITIVE";
		case AST_DECL_TYPE_STRUCT:         return "DECL_TYPE_STRUCT";
		case AST_DECL_TYPE_STRUCT_VARIANT: return "DECL_TYPE_STRUCT_VARIANT";
		case AST_DECL_TYPE_ENUM:           return "DECL_TYPE_ENUM";
		case AST_DECL_TYPE_FUNCTION:       return "DECL_TYPE_FUNCTION";
		case AST_DECL_VAR:                 return "DECL_VAR";
		case AST_DECL_BLOCK:               return "DECL_BLOCK";
		case AST_TYPE:                     return "TYPE";
		case AST_TYPE_PATTERN_MATCH:       return "TYPE_PATTERN_MATCH";
		case AST_CALL:                     return "CALL";
		case AST_BLOCK:                    return "BLOCK";
		case AST_MODULE:                   return "MODULE";
		default:                           return "UNKNOWN";
	}
}

/* Debug: Recursively print an AST node with indentation */
void debug_ast_node(ast_node* node, int indent) {
	if (!node)
		return;
	for (int i = 0; i < indent; i++)
		printf("  ");
	printf("[%s]", get_ast_node_type_name(node->node_type));
	if (node->t && node->t->content && node->t->content->data)
		printf(" (token: %s)", node->t->content->data);
	if (node->identifier)
		printf(" [identifier: %s]", node->identifier->data);
	if (node->lit_value) {
		printf(" value: %s", node->lit_value->data);
	}
	printf("\n");

	if (node->left) {
		for (int i = 0; i < indent + 1; i++) printf("  ");
		printf("Left:\n");
		debug_ast_node(node->left, indent + 2);
	}
	if (node->right) {
		for (int i = 0; i < indent + 1; i++) printf("  ");
		printf("Right:\n");
		debug_ast_node(node->right, indent + 2);
	}
	if (node->children) {
		for (int i = 0; i < indent + 1; i++) printf("  ");
		printf("Children:\n");
		debug_ast_node(node->children, indent + 2);
	}
	if (node->type) {
		for (int i = 0; i < indent + 1; i++) printf("  ");
		printf("Type:\n");
		debug_ast_node(node->type, indent + 2);
	}
	if (node->next)
		debug_ast_node(node->next, indent);
}

void token_unexpected(parser_state* p, char* msg)
{
	if(!msg)
		msg = "aborting";
	if(!p->current_token)
		printf("Unexpected end of file (%s)\n", msg);
	else if(p->current_token->type == TOK_NEWLINE)
		printf("Unexpected newline at pos %lu (%s)\n", p->current_token->src_pos, msg);
	else if(!p->current_token->content)
		printf("Unexpected <%c> at pos %lu (%s)\n", p->current_token->type,
			p->current_token->src_pos, msg);
	else
		printf("Unexpected %s at pos %lu (%s)\n", 
			p->current_token->content->data, p->current_token->src_pos, msg);
	exit(1);
}

token* get_token(parser_state* p, u32 steps) {
	if(!p->current_token)
		return p->current_token;
	token* tok = p->current_token;
	if(steps == 0)
	{
		// advance to next non-whitespace token
		while(tok && (tok->type == 'S' || tok->type == 'I' || tok->type == '#' || tok->type == 'L'))
		{
			if(tok->type == 'I')
				p->current_indent = tok->content->length;
			if(tok->type == 'L')
				p->is_fresh_line = 1;
			tok = tok->next;
		}
		return tok;
	}
	u32 i = 0;
	do {
		tok = tok->next;
		if(p->current_token->type != 'S' &&
			p->current_token->type != 'I' &&
			p->current_token->type != 'L' && // not sure yet if we should ignore new lines
			p->current_token->type != '#')
			i += 1;
	} while(tok && i < steps);
	if(i < steps)
		return 0;
	return tok;
}

token* consume_whitespace(parser_state* p)
{
	p->current_token = get_token(p, 0);
	return p->current_token;
}

token* consume_token(parser_state* p)
{
	p->current_token = get_token(p, 1);
	p->is_fresh_line = 0;
	return p->current_token;
}

token* consume_token_expect(parser_state* p, u8 ttype, char* content)
{
	token* tok = p->current_token;
	p->is_fresh_line = 0;
	if(!tok)
	{
		token_unexpected(p, content);
	}
	if(ttype == tok->type)
	{
		if(!content)
			return consume_token(p);
		if(string_equals_cstr(tok->content, content, true))
			return consume_token(p);
	}
	token_unexpected(p, content);
	return 0;
}

bool match_token(parser_state* p, u32 steps, u8 ttype, char* content)
{
	token* tok = get_token(p, steps);
	if(!tok)
		return false;
	if(ttype == tok->type)
	{
		if(!content)
			return true;
		return string_equals_cstr(tok->content, content, true);
	}
	return false;
}

ast_node* parser_append_node(ast_node* parent, ast_node* new_node)
{
	if(!new_node)
		return(parent);
	parent->next = new_node;
	return new_node;
}

ast_node* parse_call(parser_state* p);
ast_node* parse_statement(parser_state* p);
void parse_block(parser_state* p);

bool is_end_of_expression(parser_state* p)
{
	return !consume_whitespace(p) || 
		match_token(p, 0, TOK_PUNCT, ")") || 
		match_token(p, 0, TOK_PUNCT, ",");
}

int get_precedence(token* tok) {
    if (!tok || tok->type != TOK_PUNCT) return 0;
    for (int i = 0; operator_table[i].op != NULL; i++) 
	{
        if (string_equals_cstr(tok->content, operator_table[i].op, false))
            return operator_table[i].prec;
    }
    return 0;
}

// I keep forgetting how this works:
// https://crockford.com/javascript/tdop/tdop.html
// https://dl.acm.org/doi/pdf/10.1145/512927.512931


ast_node* parse_expression_rbp(parser_state* p, int rbp) {
    token* tok = consume_whitespace(p);
    if (!tok)
        token_unexpected(p, "unexpected end of input");
    consume_token(p);
    
    ast_node* left = 0;

    if (tok->type == TOK_ALPHA) {
        left = parser_create_node(AST_IDENTIFIER, tok);
        left->identifier = tok->content;
    }
    else if (tok->type == TOK_NLITERAL) {
        left = parser_create_node(AST_LIT_NUMBER, tok);
        left->lit_value = tok->content;
    }
    else if (tok->type == TOK_QLITERAL) {
        left = parser_create_node(AST_LIT_STRING, tok);
        left->lit_value = tok->content;
    }
    else if (tok->type == TOK_PUNCT) {
        if (string_equals_cstr(tok->content, "(", false)) {
            left = parse_expression_rbp(p, 0);
            consume_token_expect(p, TOK_PUNCT, ")");
        }
        else if (string_equals_cstr(tok->content, "-", false) ||
                 string_equals_cstr(tok->content, "+", false)) {
            // Unary operator: use a high binding power, fixme?
            ast_node* node = parser_create_node(AST_OP_UNARY, tok);
			node->identifier = tok->content;
            node->left = parse_expression_rbp(p, 100);
            left = node;
        }
        else {
            token_unexpected(p, "unexpected token in prefix position");
        }
    }
    else {
        token_unexpected(p, "unexpected token");
    }
    
    // Led: process infix operators & function calls.
    while (p->current_token) {
        token* next = p->current_token;
        
        // Function call: if the next token is '(' then it's a call.
        if (next->type == TOK_PUNCT && string_equals_cstr(next->content, "(", false)) {
            consume_token(p);  // consume '('
            ast_node* call = parser_create_node(AST_CALL, left->t);
            call->left = left;
            // Parse argument list.
            ast_node* arg_list = NULL;
            ast_node* last_arg = NULL;
            if (!match_token(p, 0, TOK_PUNCT, ")")) {
                do {
                    ast_node* arg = parse_expression_rbp(p, 0);
                    if (!arg)
                        token_unexpected(p, "error parsing argument");
                    if (!arg_list) {
                        arg_list = arg;
                        last_arg = arg;
                    } else {
                        last_arg->next = arg;
                        last_arg = arg;
                    }
                    if (match_token(p, 0, TOK_PUNCT, ",")) {
                        consume_token(p);
                    }
                    else {
                        break;
                    }
                } while (1);
            }
            call->children = arg_list;
            consume_token_expect(p, TOK_PUNCT, ")");
            left = call;
            continue;
        }
        
        int prec = get_precedence(next);
        if (prec <= rbp)
            break;
        
        consume_token(p);
        ast_node* op_node = parser_create_node(AST_OP_BINARY, next);
        op_node->left = left;
        // For left-associative operators, pass the current precedence.
        op_node->right = parse_expression_rbp(p, prec);
        left = op_node;
    }
    
    return left;
}

ast_node* parse_expression(parser_state* p) {
    ast_node* expr = parse_expression_rbp(p, 0);
    return expr;
}

void parse_argument_list(parser_state* p, ast_node* parent)
{
	ast_node* current_arg = 0;
	while(consume_whitespace(p) && 
		!match_token(p, 0, TOK_PUNCT, ")"))
	{
		ast_node* arg = parse_expression(p);
		if(arg)
		{
			if(!current_arg)
			{
				parent->children = arg;
				current_arg = arg;
			}
			else
			{
				current_arg->next = arg;
				current_arg = arg;
			}
			if(match_token(p, 0, TOK_PUNCT, ","))
			{
				consume_token(p);
				continue;
			}
			else
			{
				break;
			}
		}
		else
		{
			token_unexpected(p, "error parsing argument");
		}
	}
}

ast_node* parse_call(parser_state* p)
{
	consume_whitespace(p);
	ast_node* n = parser_create_node(AST_CALL, p->current_token);
	n->identifier = p->current_token->content;
	consume_token(p); // consume identifier
	consume_token_expect(p, TOK_PUNCT, "(");
	parse_argument_list(p, n);
	consume_token_expect(p, TOK_PUNCT, ")");
	return n;
}

ast_node* parse_statement(parser_state* p)
{
	ast_node* stmt = parse_expression(p);
	if(!stmt)
	{
		token_unexpected(p, "error parsing statement");
	}
	return stmt;
}

void parse_block(parser_state* p)
{
	while(consume_whitespace(p))
	{
		p->current_node = parser_append_node(p->current_node, parse_statement(p));
	}
}

parser_state* parse(tokenizer_state* tok_state) {
	parser_state* p = arena_alloc(default_arena, sizeof(parser_state));
	p->tok = tok_state;
	p->current_token = tok_state->tokens;
	p->module_root = parser_create_node(AST_MODULE, tok_state->tokens);
	p->current_node = p->module_root;
	parse_block(p);
	return p;
}
