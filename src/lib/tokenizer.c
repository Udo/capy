#include "types.h"

typedef enum {
	TOK_NEWLINE = 'L', // a new line
	TOK_INDENT = 'I', // indenting space after a new line before any code, indent level is in token->content->length
	TOK_SPACE = 'S', // whitespace, can mostly be ignored
	TOK_PUNCT = 'P', // every punctuation character gets put into its own token (in token->content)
	TOK_ALPHA = 'A', // alphanumerical sequence (in token->content)
	TOK_COMMENT = '#', // comment text (in token->content)
	TOK_NLITERAL = 'N', // number literal (in token->content)
	TOK_QLITERAL = 'Q', // quoted string literal (in token->content)
	TOK_UNKNOWN = '?',
} token_type;

typedef struct {
    char* op;
    int prec;
} operator_table_def;

operator_table_def operator_table[] = {
    {",",    1},    // Comma operator (lowest precedence)
    
    // Assignment operators (right-associative, low precedence)
    {"=",    5},
    {"+=",   5},
    {"-=",   5},
    {"*=",   5},
    {"/=",   5},
    {"%=",   5},
    {"<<=",  5},
    {">>=",  5},
    {"&=",   5},
    {"^=",   5},
    {"|=",   5},

    // Logical OR and AND
    {"||",   8},
    {"&&",   10},

    // Equality and relational operators
    {"==",   18},
    {"!=",   18},
    {"<",    20},
    {"<=",   20},
    {">",    20},
    {">=",   20},

    // Additive operators
    {"+",    24},
    {"-",    24},

    // Multiplicative operators
    {"*",    26},
    {"/",    26},
    {"%",    26},

	{".",    30}, // Member access operator (highest precedence)
	{":",    30}, // Member access operator (highest precedence)
	{"[",    30}, // Array subscript operator]"}
	{"(",    30}, // Function call operator

    {0,  0} 
};

typedef struct token token;
struct token
{
	u8 type;
	string* content;
	u32 hblockid; // reserved, maybe unneeded
	u64 src_pos;
	token* next;
};

typedef struct tokenizer_state tokenizer_state;
struct tokenizer_state
{
	token* tokens;
	string* src;
	string* filename;
};

int is_operator_prefix(const char* candidate) {
    int cand_len = strlen(candidate);
    for (int j = 0; operator_table[j].op != 0; j++) {
        if (strncmp(candidate, operator_table[j].op, cand_len) == 0) {
            return 1;
        }
    }
    return 0;
}

u8 tok_get_char_type(u8 c)
{
	if(is_linebreak(c)) return 'L';
	if(is_alpha(c) || is_digit(c) || c == '_') return 'A';
	if(is_space(c)) return 'S';
	if(is_punct(c)) return 'P';
	return '?';
}

token* new_token(u8 ctype, token* prev, u64 pos)
{
	token* nt = arena_alloc(default_arena, sizeof(token));
	nt->content = string_create(8);
	nt->type = ctype;
	nt->src_pos = pos;
	prev->next = nt;
	return nt;
}

void tok_print_repeat(char* str, u32 times)
{
	for (u32 i = 0; i < times; i++)
	{
		printf("%s", str);
	}
}

void tok_print_single(token* token_current)
{
	if(token_current)
	{
		if(token_current->type == 'L') // line break
			printf("\n%05lu: ", token_current->src_pos);
		else if(token_current->type == 'P')
			printf("%s ", token_current->content->data);
		else if(token_current->type == 'H')
			printf(" H%i'%s'H%i ", token_current->hblockid, token_current->content->data, token_current->hblockid);
		else if(token_current->type == '#')
			printf("/* %s */", token_current->content->data);
		else if(token_current->type == 'Q')
			printf("«%s» ", token_current->content->data);
		else if(token_current->type == 'S')
		{
		//	tok_print_repeat(" ", (u32)token_current->content->length);
		}
		else if(token_current->type == 'I')
			tok_print_repeat("    ", (u32)token_current->content->length);
		else if(token_current->type == 'A')
			printf("<|%s|> ", token_current->content->data);
		else
			printf("<%c %s> ", token_current->type, token_current->content->data);
	}
}

void tok_print(token* token_current)
{
	while (token_current)
	{
		tok_print_single(token_current);
		token_current = token_current->next;
	}
}

tokenizer_state* tokenize(string* content, char* filename)
{
	token* token_list = arena_alloc(default_arena, sizeof(token));
	token_list->content = string_create(8);
	token_list->type = 'L';
	token* token_current = token_list;
	u8 pmode = '.';
	u8 lastc = 0;
	//u8 lastctype = 0;
	u8 omit_lasttag = 0;
	u8 is_new_line = 0;
	s32 xml_level = 0;
	u32 hblock_count = 0;
	for (u64 i = 0; i < content->length; i++)
	{
		u8 c = content->data[i];
		u8 nextc = (i+1 < content->length) ? content->data[i+1] : 0;
		u8 ctype = tok_get_char_type(c);
		if(pmode == '.') // 'normal' mode
		{
			if(token_current->type == 0)
			{
				// If the current token is empty, set its type.
				token_current->type = ctype;
			}
			else if(ctype == 'P' && token_current->type == 'P')
			{
				// When in punctuation mode, try to extend the operator token.
				// Build a candidate string = current token content + current char.
				int cand_len = token_current->content->length + 1;
				char candidate[cand_len + 1];
				memcpy(candidate, token_current->content->data, token_current->content->length);
				candidate[token_current->content->length] = c;
				candidate[cand_len] = '\0';

				if(is_operator_prefix(candidate))
				{
					// Candidate is a valid operator prefix, so append and continue.
					string_append_char(token_current->content, c);
					continue;
				}
				else
				{
					// Not a valid extension: finish current token and start a new punctuation token.
					token_current = new_token(ctype, token_current, i);
				}
			}
			else if(ctype != token_current->type)
			{
				token_current = new_token(ctype, token_current, i);
			}
			
			// Process special cases for quotes or comments.
			if(c == '"' || c == '\'')
			{
				pmode = c;
				token_current->type = 'Q';
			}
			else if (c == '/' && nextc == '*')
			{
				pmode = 'C';
				token_current->type = '#';
				string_clear(token_current->content);
				i += 1;
			}
			else if ((c == '/' && nextc == '/') || (c == '#'))
			{
				pmode = '/';
				token_current->type = '#';
				string_clear(token_current->content);
				i += 1;
			}
			else if (c == '<' && lastc == '(')
			{
				pmode = 'H';
				omit_lasttag = 0;
				if(nextc == '>')
				{
					omit_lasttag = 1;
					i += 1;
				}
				token_current->type = 'Q';
				xml_level = 1;
				hblock_count += 1;
				token_current->hblockid = hblock_count;
				if(!omit_lasttag)
					string_append_char(token_current->content, c);
			}
			else
			{
				// Append the current character normally.
				string_append_char(token_current->content, c);
			}
		}
		else if(pmode == 'C') // multi line comment
		{
			if (c == '*' && nextc == '/')
			{
				pmode = '.';
				i += 1;
				token_current = new_token(0, token_current, i);
			}
			else
			{
				string_append_char(token_current->content, c);
			}
		}
		else if(pmode == '/') // single line comment
		{
			if (c == '\n')
			{
				pmode = '.';
				token_current = new_token('L', token_current, i+1);
			}
			else
			{
				string_append_char(token_current->content, c);
			}
		}
		else if(pmode == 'H') // 'HTML' mode
		{
			u8 do_append = 1;
			if(c == '?' && lastc == '<')
				xml_level -= 1;
			else if(c == '<')
				xml_level += 1;
			else if(c == '/' && lastc == '<')
			{
				xml_level -= 2;
				if(xml_level == 0 && nextc == '>' && omit_lasttag)
				{
					pmode = '.';
					i += 1;
					do_append = 0;
					string_set_length(token_current->content, -1);
				}
			}
			else if(c == '>' && xml_level == 0)
				pmode = '.';
			if(do_append)
				string_append_char(token_current->content, c);
		}
		else if(pmode == '"' || pmode == '\'') // quote mode
		{
			if(c == pmode)
			{
				if(lastc == '\\')
				{
					// if previous char was \ then this current quote char is part of token
					token_current->content->data[token_current->content->length-1] = c;
				}
				else
				{
					// end quote mode, swallow current char
					pmode = '.';
				}
			}
			else
			{
				string_append_char(token_current->content, c);
			}
		}
		lastc = c;
		//lastctype = ctype;
		if(token_current->type == 'L')
			is_new_line = 1;
		else if(token_current->type != 'S')
			is_new_line = 0;
		if(is_new_line && token_current->type == 'S')
			token_current->type = 'I';
	}
	tokenizer_state* t = arena_alloc(default_arena, sizeof(tokenizer_state));
	t->tokens = token_list;
	t->src = content;
	t->filename = string_create_from_cstr(filename);
	return t;
}
