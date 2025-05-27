#include "types.h"

typedef enum {
	TOK_NEWLINE = 'L', // a new line
	TOK_INDENT = 'I', // indenting space after a new line before any code, indent level is in token->content->length
	TOK_SPACE = 'S', // whitespace, can mostly be ignored
	TOK_PUNCT = 'P', // every punctuation character gets put into its own token (in token->content)
	TOK_ALPHA = 'A', // alphanumerical sequence (in token->content)
	TOK_COMMENT = '#', // comment text (in token->content)
	TOK_NUMBER = 'N', // number literal (in token->content)
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
	u32 indent;
	u8 is_first_on_line;
	u8 is_operator;
	u32 hblockid;
	u64 src_pos;
	string* src; 
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

typedef struct code_position code_position;
struct code_position
{
	u64 line;
	u64 column;
};

code_position get_code_position(string* content, u64 pos)
{
	code_position cp = {1, 1};
	for (u64 i = 0; i < pos; i++)
	{
		if(content->data[i] == '\n')
		{
			cp.line += 1;
			cp.column = 1;
		}
		else
		{
			cp.column += 1;
		}
	}
	return cp;
}

#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_LIGHTBLUE "\x1b[94m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_LIGHTRED "\x1b[91m"
#define ANSI_COLOR_GRAY    "\x1b[90m"
#define ANSI_COLOR_WHITE   "\x1b[97m"
#define ANSI_COLOR_BOLD    "\x1b[1m"
#define ANSI_COLOR_DIM     "\x1b[2m"
#define ANSI_COLOR_UNDER   "\x1b[4m"
#define ANSI_COLOR_BLINK   "\x1b[5m"
#define ANSI_COLOR_INV     "\x1b[7m"
#define ANSI_COLOR_RESET   "\x1b[0m"

void pretty_print_lineatpos(string* content, u64 pos)
{
	code_position cp = get_code_position(content, pos);
	printf(ANSI_COLOR_LIGHTBLUE "%05lu " ANSI_COLOR_BLUE "🮐 " ANSI_COLOR_RESET, cp.line);
	u64 start = pos;
	while(start > 0 && content->data[start-1] != '\n')
		start -= 1;
	u64 end = pos;
	while(end < content->length && content->data[end] != '\n')
		end += 1;
	u64 len = end - start;
	char* line = arena_alloc(default_arena, len + 1);
	memcpy(line, content->data + start, len);
	line[len] = '\0';
	printf("%s\n      " ANSI_COLOR_BLUE "🮐" ANSI_COLOR_LIGHTRED "🮣", line);
	u64 pointer = pos - start;
	for (u64 i = 0; i < pointer; i++)
		printf("🭹");
	printf("🮧🭹🭹🭹🭹🭹\n" ANSI_COLOR_RESET);
}

token* new_token(u8 ctype, token* prev, u64 pos, string* src)
{
	if(prev->type == 'A')
	{
		// check if token content could be a numeric literal and change type if so
		u8 is_number = 1;
		u8 has_dot = 0;
		for (u64 i = 0; i < prev->content->length; i++)
		{
			if(prev->content->data[i] == '.')
			{
				if(has_dot)
				{
					is_number = 0;
					break;
				}
				has_dot = 1;
			}
			else if(!is_digit(prev->content->data[i]))
			{
				is_number = 0;
				break;
			}
		}
		if(is_number)
			prev->type = 'N';
	}
	token* nt = arena_alloc(default_arena, sizeof(token));
	nt->content = string_create(8);
	nt->type = ctype;
	nt->src_pos = pos;
	nt->src = src;
	nt->indent = prev->indent;
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
		if(token_current->is_first_on_line) // line break
			printf("\n%05lu: ", token_current->src_pos);
		printf("%c%i", token_current->type, token_current->indent);
		if(token_current->type == 'P') // punctuation
			printf("%s ", token_current->content->data);
		else if(token_current->type == 'H') // HTML block
			printf(" H%i'%s'H%i ", token_current->hblockid, token_current->content->data, token_current->hblockid);
		else if(token_current->type == '#') // comment
			printf("/* %s */", token_current->content->data);
		else if(token_current->type == 'Q') // quoted string
			printf("«%s» ", token_current->content->data);
		else if(token_current->type == 'S') 
		{
			printf("_");
		}
		else if(token_current->type == 'I') // indent
			tok_print_repeat("﹍", (u32)token_current->content->length);
		else if(token_current->type == 'A') // alphanumerical sequence
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
	printf("\n");
}

token* tokenizer_cleanup(token* token_list)
{
	token* first = 0;
	token* current = 0;
	while(token_list)
	{
		if(token_list->type == 'P' && !string_equals_cstr(token_list->content, ")", true) && !string_equals_cstr(token_list->content, "(", true))
		{
			token_list->is_operator = true;
		}
		if(token_list->type == 'L' || token_list->type == 'S' || token_list->type == 'I' || token_list->type == '#')
		{
			token_list = token_list->next;
			continue;
		}
		if(!first)
		{
			first = token_list;
			current = token_list;
		}
		else
		{
			current->next = token_list;
			current = current->next;
		}
		token_list = token_list->next;
	}
	if(current->next && (current->next->type == 'L' || current->next->type == 'S' || current->next->type == 'I' || current->next->type == '#'))
		current->next = 0;
	return first;
}

tokenizer_state* tokenize(string* content, char* filename)
{
	token* token_list = arena_alloc(default_arena, sizeof(token));
	token_list->content = string_create(8);
	token_list->type = 'L';
	token_list->src_pos = 1;
	token_list->src = content;
	token* token_current = token_list;
	u8 pmode = '.';
	u8 lastc = 0;
	//u8 lastctype = 0;
	u8 omit_lasttag = 0;
	u8 is_new_line = 1;
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
					token_current = new_token(ctype, token_current, i, content);
				}
			}
			else if(ctype != token_current->type)
			{
				if(is_new_line && token_current->type == 'S')
				{
					token_current->type = 'I';
				}
				token_current = new_token(ctype, token_current, i, content);
				if(is_new_line)
					token_current->is_first_on_line = 1;
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
				token_current = new_token(0, token_current, i, content);
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
				token_current = new_token('L', token_current, i+1, content);
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
		{
			is_new_line = 1;
			token_current->indent = 0;
		}
		else if(token_current->type != 'S')
			is_new_line = 0;
		if(is_new_line && token_current->type == 'S')
		{
			token_current->indent = token_current->content->length;
		}
	}
	token_current = new_token('L', token_current, content->length, content);
	tokenizer_state* t = arena_alloc(default_arena, sizeof(tokenizer_state));
	t->tokens = tokenizer_cleanup(token_list);
	t->src = content;
	t->filename = string_create_from_cstr(filename);
	return t;
}
