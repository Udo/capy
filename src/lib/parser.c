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
	symbol* _last_symbol; // makes it easier to append
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

/* Return a string representation of an ast_node_type */
char* get_ast_node_type_name(ast_node_type type) {
	switch (type) {
		case AST_KEYWORD:                  return "KEYWORD";
		case AST_DIRECTIVE:                return "DIRECTIVE";
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

void debug_ast_node(ast_node* node, int indent, symbol_table* st) {
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
	if (node->type) {
		for (int i = 0; i < indent + 1; i++) printf("  ");
		printf("Type:\n");
		debug_ast_node(node->type, indent + 2, node->symbol_table);
	}
	if (node->symbol_table && st != node->symbol_table && 
		(node->node_type == AST_MODULE || node->node_type == AST_BLOCK)) {
		for (int i = 0; i < indent + 1; i++) printf("  ");
		printf("Symbol table:\n");
		symbol* sym = node->symbol_table->symbols;
		while (sym) {
			for (int i = 0; i < indent + 2; i++) printf("  ");
			printf("Symbol: %s\n", sym->name->data);
			// show symbol type
			if (sym->type) {
				debug_ast_node(sym->type, indent + 4, node->symbol_table);
			}
			sym = sym->next;
		}
	}
	if (node->left) {
		for (int i = 0; i < indent + 1; i++) printf("  ");
		printf("Left:\n");
		debug_ast_node(node->left, indent + 2, node->symbol_table);
	}
	if (node->right) {
		for (int i = 0; i < indent + 1; i++) printf("  ");
		printf("Right:\n");
		debug_ast_node(node->right, indent + 2, node->symbol_table);
	}
	if (node->children) {
		for (int i = 0; i < indent + 1; i++) printf("  ");
		printf("Children:\n");
		debug_ast_node(node->children, indent + 2, node->symbol_table);
	}
	if (node->next)
		debug_ast_node(node->next, indent, node->symbol_table);
}

void token_unexpected(parser_state* p, char* msg)
{
	if(!msg)
		msg = "aborting";
	if(!p->current_token)
	{
		printf("Unexpected end of file (%s)\n", msg);
		exit(1);
	}
	pretty_print_lineatpos(p->tok->src, p->current_token->src_pos);
	printf(ANSI_COLOR_BLUE "      🮐 " ANSI_COLOR_RESET);
	if(p->current_token->type == TOK_NEWLINE)
		printf("Unexpected newline(%s)\n", msg);
	else if(p->current_token->type == TOK_SPACE || p->current_token->type == TOK_INDENT)
		printf("Unexpected whitespace(%s)\n", msg);
	else if(!p->current_token->content)
		printf("Unexpected <%c> (%s)\n", p->current_token->type, msg);
	else
		printf("Unexpected %s (%s)\n", 
			p->current_token->content->data, msg);
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
			tok = tok->next;
		}
		return tok;
	}
	u32 i = 0;
	do {
		tok = tok->next;
		if(tok && tok->type != 'S' &&
			tok->type != 'I' &&
			tok->type != 'L' &&
			tok->type != '#')
			i += 1;
	} while(tok && i < steps);
	if(i < steps)
		return 0;
	return tok;
}

token* consume_whitespace(parser_state* p)
{
	p->last_token = p->current_token;
	p->current_token = get_token(p, 0);
	return p->current_token;
}

token* consume_token(parser_state* p)
{
	p->last_token = p->current_token;
	p->current_token = get_token(p, 1);
	return p->current_token;
}

token* consume_token_expect(parser_state* p, u8 ttype, char* content)
{
	p->last_token = p->current_token;
	token* tok = p->current_token;
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

void token_expect(parser_state* p, u8 ttype, char* content)
{
	if(!match_token(p, 0, ttype, content))
		token_unexpected(p, content);
}

ast_node* parser_create_node(parser_state* p, ast_node_type type, token* t)
{
	ast_node* n = arena_alloc(default_arena, sizeof(ast_node));
	n->node_type = type;
	n->symbol_table = p->current_symbol_table;
	n->t = t;
	return n;
}

symbol_table* parser_create_symbol_table(parser_state* p)
{
	symbol_table* st = arena_alloc(default_arena, sizeof(symbol_table));
	st->scope_start = get_token(p, 0);
	if(p->current_symbol_table)
		st->parent = p->current_symbol_table;
	p->current_symbol_table = st;
	return st;
}

symbol* parser_find_symbol_by_name(parser_state* p, string* name, bool recursive)
{
	symbol_table* st = p->current_symbol_table;
	while(st)
	{
		symbol* sym = st->symbols;
		while(sym)
		{
			if(string_equals(sym->name, name, true))
				return sym;
			sym = sym->next;
		}
		if(!recursive)
			break;
		st = st->parent;
	}
	return 0;
}

symbol* parser_create_symbol(parser_state* p, string* name, ast_node* type)
{
	symbol* existing = parser_find_symbol_by_name(p, name, false);
	if(existing)
	{
		pretty_print_lineatpos(p->tok->src, existing->type->t->src_pos);
		printf(ANSI_COLOR_BLUE "      🮐 " ANSI_COLOR_RESET "original definition of %s\n", name->data);
		pretty_print_lineatpos(p->tok->src, p->last_token->src_pos);
		printf(ANSI_COLOR_BLUE "      🮐 " ANSI_COLOR_RESET);
		printf("cannot have another %s in the same block\n", name->data);
		exit(1);
	}
	symbol* sym = arena_alloc(default_arena, sizeof(symbol));
	sym->name = name;
	sym->type = type;
	if(p->current_symbol_table->_last_symbol)
		p->current_symbol_table->_last_symbol->next = sym;
	else
		p->current_symbol_table->symbols = sym;
	p->current_symbol_table->_last_symbol = sym;
	return sym;
}

ast_node* parse_call(parser_state* p);
ast_node* parse_statement(parser_state* p);
ast_node* parse_block(parser_state* p);
ast_node* parse_declaration(parser_state* p);

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

void check_identifier(parser_state* p, string* id)
{
	symbol* sym = parser_find_symbol_by_name(p, id, true);
	if(!sym)
	{
		pretty_print_lineatpos(p->tok->src, p->last_token->src_pos);
		printf(ANSI_COLOR_BLUE "      🮐 " ANSI_COLOR_RESET);
		printf("undeclared identifier %s\n", id->data);
		exit(1);
	}
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

    if (tok->type == TOK_ALPHA) 
	{
		check_identifier(p, tok->content);
        left = parser_create_node(p, AST_IDENTIFIER, tok);
        left->identifier = tok->content;
    }
    else if (tok->type == TOK_NUMBER) 
	{
        left = parser_create_node(p, AST_LIT_NUMBER, tok);
        left->lit_value = tok->content;
    }
    else if (tok->type == TOK_QLITERAL) 
	{
        left = parser_create_node(p, AST_LIT_STRING, tok);
        left->lit_value = tok->content;
    }
    else if (tok->type == TOK_PUNCT) 
	{
        if (string_equals_cstr(tok->content, "(", false)) 
		{
            left = parse_expression_rbp(p, 0);
            consume_token_expect(p, TOK_PUNCT, ")");
        }
        else if (string_equals_cstr(tok->content, "-", false) ||
                 string_equals_cstr(tok->content, "+", false)) 
		{
            // Unary operator: use a high binding power, fixme?
            ast_node* node = parser_create_node(p, AST_OP_UNARY, tok);
			node->identifier = tok->content;
            node->left = parse_expression_rbp(p, 100);
            left = node;
        }
        else 
		{
			// rewind because we already consumed the token
			p->current_token = p->last_token;
            token_unexpected(p, "unexpected token in prefix position");
        }
    }
    else 
	{
        token_unexpected(p, "unexpected token");
    }
    
    // Led: process infix operators & function calls.
    while (p->current_token) {
        token* next = p->current_token;
        
        // Function call: if the next token is '(' then it's a call.
        if (next->type == TOK_PUNCT && string_equals_cstr(next->content, "(", false)) {
            consume_token(p);  // consume '('
            ast_node* call = parser_create_node(p, AST_CALL, left->t);
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
        ast_node* op_node = parser_create_node(p, AST_OP_BINARY, next);
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
	ast_node* n = parser_create_node(p, AST_CALL, p->current_token);
	n->identifier = p->current_token->content;
	consume_token(p); // consume identifier
	consume_token_expect(p, TOK_PUNCT, "(");
	parse_argument_list(p, n);
	consume_token_expect(p, TOK_PUNCT, ")");
	return n;
}

ast_node* parse_directive(parser_state* p)
{
	consume_whitespace(p);
	consume_token_expect(p, TOK_PUNCT, "@");
	token_expect(p, TOK_ALPHA, 0);
	ast_node* n = parser_create_node(p, AST_DIRECTIVE, p->current_token);
	n->identifier = p->current_token->content;
	ast_node* current_arg = 0;
	consume_token(p);
	consume_whitespace(p);
	while(p->current_token && !p->current_token->is_first_on_line)
	{
		ast_node *arg = parser_create_node(p, AST_KEYWORD, p->current_token);
		arg->identifier = p->current_token->content;
		if(!current_arg)
		{
			n->children = arg;
			current_arg = arg;
		}
		else
		{
			current_arg->next = arg;
			current_arg = arg;
		}
		consume_token(p);
		consume_whitespace(p);
	}
	return n;
}

// Forward declarations for the helper functions.
ast_node* parse_primitive_type(parser_state* p);
ast_node* parse_struct_type(parser_state* p);
ast_node* parse_enum_type(parser_state* p);
ast_node* parse_function_or_grouped_type(parser_state* p);

ast_node* parse_type_expression(parser_state* p, char* stop_at)
{
    consume_whitespace(p);
    token* tok = p->current_token;
    if (!tok)
	{
        token_unexpected(p, "expected type expression");
		return 0;
	}

	if (tok->type == TOK_PUNCT && stop_at && string_equals_cstr(tok->content, stop_at, false))
	{
        ast_node* n = parser_create_node(p, AST_IDENTIFIER, tok);
		n->identifier = string_create_from_cstr("auto");
		return n;
	}
    else if (tok->type == TOK_ALPHA && string_equals_cstr(tok->content, "struct", false))
        return parse_struct_type(p);
    else if (tok->type == TOK_ALPHA && string_equals_cstr(tok->content, "enum", false))
        return parse_enum_type(p);
    else if (tok->type == TOK_PUNCT && string_equals_cstr(tok->content, "(", false))
        return parse_function_or_grouped_type(p);
    else if (tok->type == TOK_ALPHA)
        return parse_primitive_type(p);

    token_unexpected(p, "invalid type expression");
    return 0;
}

ast_node* parse_primitive_type(parser_state* p)
{
    token* tok = p->current_token;
    ast_node* prim = parser_create_node(p, AST_DECL_TYPE_PRIMITIVE, tok);
    prim->identifier = tok->content;
    consume_token(p);
    return prim;
}

ast_node* parse_struct_type(parser_state* p)
{
    consume_token(p); // consume 'struct'
    ast_node* struct_node = parser_create_node(p, AST_DECL_TYPE_STRUCT, p->last_token);
    consume_whitespace(p);
    consume_token_expect(p, TOK_PUNCT, "{");
    ast_node* field_list = NULL;
    ast_node* last_field = NULL;
    while (consume_whitespace(p) && !match_token(p, 0, TOK_PUNCT, "}"))
    {
        ast_node* field = parse_declaration(p);
        if (!field)
            token_unexpected(p, "error parsing struct field");
        if (!field_list)
        {
            field_list = field;
            last_field = field;
        }
        else
        {
            last_field->next = field;
            last_field = field;
        }
        if (match_token(p, 0, TOK_PUNCT, ";"))
            consume_token(p);
    }
    struct_node->children = field_list;
    consume_token_expect(p, TOK_PUNCT, CLOSING_BRACE);
    return struct_node;
}

ast_node* parse_enum_type(parser_state* p)
{
    consume_token(p); // consume 'enum'
    ast_node* enum_node = parser_create_node(p, AST_DECL_TYPE_ENUM, p->last_token);
    consume_whitespace(p);
    consume_token_expect(p, TOK_PUNCT, "{");
    ast_node* enumerators = NULL;
    ast_node* last_enum = NULL;
    while (consume_whitespace(p) && !match_token(p, 0, TOK_PUNCT, "}"))
    {
        token* id_tok = p->current_token;
        if (id_tok->type != TOK_ALPHA)
            token_unexpected(p, "expected enumerator identifier");
        ast_node* enumerator = parser_create_node(p, AST_IDENTIFIER, id_tok);
        enumerator->identifier = id_tok->content;
        if (!enumerators)
        {
            enumerators = enumerator;
            last_enum = enumerator;
        }
        else
        {
            last_enum->next = enumerator;
            last_enum = enumerator;
        }
        consume_token(p);
        if (match_token(p, 0, TOK_PUNCT, ",") || match_token(p, 0, TOK_PUNCT, ";"))
            consume_token(p);
    }
    enum_node->children = enumerators;
    consume_token_expect(p, TOK_PUNCT, "}");
    return enum_node;
}

ast_node* parse_function_or_grouped_type(parser_state* p)
{
    consume_token(p); // consume '('
    ast_node* param_list = NULL;
    ast_node* last_param = NULL;
    if (!match_token(p, 0, TOK_PUNCT, ")"))
    {
        do {
            ast_node* param = parse_type_expression(p, ")");
            if (!param)
                token_unexpected(p, "error parsing parameter type");
            if (!param_list)
            {
                param_list = param;
                last_param = param;
            }
            else
            {
                last_param->next = param;
                last_param = param;
            }
            consume_whitespace(p);
            if (match_token(p, 0, TOK_PUNCT, ","))
                consume_token(p);
            else
                break;
        } while (1);
    }
    consume_token_expect(p, TOK_PUNCT, ")");
    consume_whitespace(p);

    if (match_token(p, 0, TOK_PUNCT, "->"))
    {
        consume_token(p); // consume '->'
        consume_whitespace(p);
        ast_node* return_type = parse_type_expression(p, 0);
        ast_node* func_node = parser_create_node(p, AST_DECL_TYPE_FUNCTION, p->current_token);
        func_node->left = param_list;
        func_node->type = return_type;
        return func_node;
    }
    else
    {
        // Grouped or tuple type.
        if (param_list && param_list->next == NULL)
            return param_list;
        else
        {
            ast_node* tuple_node = parser_create_node(p, AST_TYPE, p->last_token);
            tuple_node->children = param_list;
            return tuple_node;
        }
    }
}

ast_node* parse_declaration(parser_state* p)
{
	consume_whitespace(p);
	ast_node* n = parser_create_node(p, AST_DECL_VAR, p->current_token);
	n->identifier = p->current_token->content;
	consume_token(p); // consume identifier
	consume_whitespace(p);
	if(match_token(p, 0, TOK_PUNCT, ":"))
	{
		consume_token(p);
		consume_whitespace(p);
		ast_node* type = parse_type_expression(p, "=");
		if(!type)
			token_unexpected(p, "error parsing type expression");
		parser_create_symbol(p, n->identifier, type);
		n->type = type;
	}
	consume_whitespace(p);
	if(match_token(p, 0, TOK_PUNCT, "="))
	{
		consume_token_expect(p, TOK_PUNCT, "=");
		consume_whitespace(p);
		ast_node* value = parse_expression(p);
		if(!value)
			token_unexpected(p, "error parsing value");
		n->children = value;
	}
	return n;
}

ast_node* parse_statement(parser_state* p)
{
	ast_node* stmt = 0;
	consume_whitespace(p);
	if(match_token(p, 0, TOK_PUNCT, "@"))
	{
		stmt = parse_directive(p);
	}
	else if(match_token(p, 0, TOK_ALPHA, "new"))
	{
		consume_token(p); // swallow "new"
		stmt = parse_declaration(p);
	}
	else if(match_token(p, 0, TOK_ALPHA, "block")) 
	{
		consume_token(p); // swallow "block"
		consume_whitespace(p);
		stmt = parse_block(p);
	}
	else
	{
		stmt = parse_expression(p);
	}
	if(!stmt)
	{
		token_unexpected(p, "error parsing statement");
	}
	return stmt;
}

ast_node* parse_block(parser_state* p)
{
	consume_whitespace(p);
	ast_node* block = parser_create_node(p, AST_BLOCK, p->current_token);
	// blocks have their own symbol table
	block->symbol_table = parser_create_symbol_table(p);
	ast_node* current_stmt = 0;
	u32 block_indent = p->current_token->indent;
	while(consume_whitespace(p) && block_indent <= p->current_token->indent)
	{
		ast_node* stmt = 0;
		stmt = parse_statement(p);
		if(!current_stmt)
		{
			block->children = stmt;
			current_stmt = stmt;
		}
		else
		{
			current_stmt->next = stmt;
			current_stmt = stmt;
		}
	}
	p->current_symbol_table = p->current_symbol_table->parent;
	return block;
}

void declare_symbol(parser_state* p, string* name, ast_node* type)
{
	symbol* sym = parser_find_symbol_by_name(p, name, true);
	if(sym)
	{
		pretty_print_lineatpos(p->tok->src, sym->type->t->src_pos);
		printf(ANSI_COLOR_BLUE "      🮐 " ANSI_COLOR_RESET);
		printf("redeclaration of %s\n", name->data);
		exit(1);
	}
	parser_create_symbol(p, name, type);
}

parser_state* parse(tokenizer_state* tok_state) {
	parser_state* p = arena_alloc(default_arena, sizeof(parser_state));
	p->tok = tok_state;
	p->current_token = tok_state->tokens;
	p->module_root = parser_create_node(p, AST_MODULE, tok_state->tokens);
	p->current_node = p->module_root;
	p->current_node->symbol_table = parser_create_symbol_table(p);
	declare_symbol(p, string_create_from_cstr("print"), 
		parser_create_node(p, AST_DECL_TYPE_FUNCTION, tok_state->tokens));
	p->module_root->children = parse_block(p);
	return p;
}
