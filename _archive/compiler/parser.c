struct Token {
	struct Token* next;
	u8 type;
	u32 line, col;
	struct string content;
};

enum char_token_types {
	CT_NONE = 0,
	CT_ALPHA,
	CT_SPACE,
	CT_CNTRL,
	CT_DIGIT,
	CT_PUNCT,
	CT_QUOTE,
	CT_DIRECTIVE,
};

char char_type_names[] = {
	'?', 'A', 'S', 'C', 'D', 'P',
	'Q', '#', '!',
};

struct Token* token_create()
{
	struct Token* t = mem_calloc(sizeof(struct Token), 1);
	t->next = 0;
	t->type = CT_NONE;
	t->line = 0;
	t->col = 0;
	string_init(&t->content);
	return t;
}

u8 parser_get_char_mode(char c)
{
	if(c == '_' || isalpha(c)) return CT_ALPHA;
	if(isspace(c)) return CT_SPACE;
	if(iscntrl(c)) return CT_CNTRL;
	if(isdigit(c)) return CT_DIGIT;
	if(ispunct(c)) return CT_PUNCT;
	return(CT_NONE);
}

struct Token* token_create_next(struct Token* tok, u32 line, u32 col)
{
	tok->next = token_create();
	tok->next->line = line;
	tok->next->col = col;
	return(tok->next);
}

struct Token* parse(struct string* src)
{
	struct Token* first = token_create();
	struct Token* current = first;
	bool escape_char_preceded = false;
	u32 line_number = 1;
	u32 col_number = 1;
	for(u32 i = 0; i < src->length; i++)
	{
		char c = src->bytes[i];
		if(c == '\n')
		{
			line_number += 1;
			col_number = 1;
		}
		else
		{
			col_number += 1;
		}
		u8 char_mode = parser_get_char_mode(c);

		if(current->type == CT_DIRECTIVE)
		{
			if(c == '\n')
			{
				current = token_create_next(current, line_number, col_number);
			}
		}
		else if(current->type == CT_QUOTE)
		{
			if(c == '\\')
			{
				escape_char_preceded = true;
			}
			else if(c == '"' && !escape_char_preceded)
			{
				current = token_create_next(current, line_number, col_number);
				continue;
			}
			else if(c == '\n')
			{
				string_append_chars(&current->content, "\\n");
			}
			else
			{
				escape_char_preceded = false;
			}
		}
		else if(char_mode != current->type || current->type == CT_NONE || char_mode == CT_PUNCT)
		{
			if(current->type != CT_NONE)
			{
				current = token_create_next(current, line_number, col_number);
			}
			if(c == '"')
			{
				current->type = CT_QUOTE;
				continue;
			}
			else if(c == '#')
			{
				current->type = CT_DIRECTIVE;
			}
			else
			{
				current->type = char_mode;
			}
		}

		string_append_bytes(&current->content, &c, 1);
	}
	return first;
}

void print_tokens(struct Token* t)
{
	printf("\e[0m");
	while(t)
	{
		if(t->type == CT_QUOTE)
			printf("\"\e[0;31m%s\e[0m\"", t->content.bytes);
		if(t->type == CT_DIRECTIVE)
			printf("\e[0;35m%s\e[0m", t->content.bytes);
		else
		//if(t->type != CT_SPACE)
			printf("%s", t->content.bytes);
		t = t->next;
	}
	printf("\e[0m");
}
