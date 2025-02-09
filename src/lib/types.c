#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <execinfo.h>
#include <stdlib.h>

typedef int8_t    s8;
typedef uint8_t   u8;
typedef int16_t   s16;
typedef uint16_t  u16;
typedef int32_t   s32;
typedef uint32_t  u32;
typedef int64_t   s64;
typedef uint64_t  u64;

typedef float     f32;
typedef double    f64;

typedef intptr_t  isize;
typedef uintptr_t usize;

typedef struct string string;
struct string
{
	void* data;
	u64 length;
	u64 capacity;
};

void print_stack_trace(void)
{
	void* buffer[50];
	int nptrs = backtrace(buffer, 50);
	char **symbols = backtrace_symbols(buffer, nptrs);
	if (!symbols) {
		perror("backtrace_symbols");
		return;
	}
	fprintf(stderr, "Stack trace:\n");
	for (int i = 0; i < nptrs; i++) {
		fprintf(stderr, "%s\n", symbols[i]);
	}
	free(symbols);
}

void fatal_error(const char *msg)
{
	fprintf(stderr, "Fatal error: %s\n", msg);
	print_stack_trace();
	exit(EXIT_FAILURE);
}

void _string_zero_guard(string *s)
{
	if(s->data)
		((char *)s->data)[s->length] = '\0';
}

void string_set_capacity(string *s, u64 new_cap)
{
	if (new_cap <= s->length) return;
	s->capacity = new_cap;
	if(s->data)
	{
		s->data = realloc(s->data, new_cap+1);
	}
	else
	{
		s->data = malloc(new_cap+1);
	}
	if (!s->data)
		fatal_error("alloc failed");
	_string_zero_guard(s);
}

string *string_create(u64 initial_capacity)
{
	string *s = malloc(sizeof(string));
	if (!s)
		fatal_error("malloc failed");
	s->length = 0;
	string_set_capacity(s, initial_capacity);
	return s;
}

string *string_create_from_c_str(const char *cstr)
{
	u64 len = strlen(cstr);
	string *s = string_create(len);
	s->length = len;
	memcpy(s->data, cstr, len);
	_string_zero_guard(s);
	return s;
}

string *string_create_from_float(double value)
{
	char buf[128];
	int n = snprintf(buf, sizeof(buf)-1, "%f", value);
	if (n < 0)
		fatal_error("snprintf failed");
	return string_create_from_c_str(buf);
}

string *string_read_entire_file(const char *filename)
{
	FILE *fp = fopen(filename, "rb");
	if (fp)
	{
		if (fseek(fp, 0, SEEK_END) != 0) {
			fclose(fp);
			return string_create(0);
		}
		long fsize = ftell(fp);
		if (fsize < 0) {
			fclose(fp);
			return string_create(0);
		}
		rewind(fp);
		string *s = string_create(fsize);
		s->length = fsize;
		if (fread(s->data, 1, s->length, fp) != s->length)
		{
			fclose(fp);
			_string_zero_guard(s);
			return s;
		}
		fclose(fp);
		return s;
	}
	return string_create(0);
}

bool string_write_entire_file(const string *s, const char *filename)
{
	FILE *fp = fopen(filename, "wb");
	if (!fp)
		return false;
	size_t written = fwrite(s->data, 1, s->length, fp);
	if (written != s->length) {
		fclose(fp);
		return false;
	}
	fclose(fp);
	return true;
}

string *string_substr(const string *s, s64 start, s64 length)
{
	/* - If start is negative, start from (s->length + start)
	 * - If length is negative, omit -length bytes from the end.
	 */
	s64 total = (s64)s->length;
	if (start < 0)
		start = total + start;
	if (start < 0)
		start = 0;
	if (start > total)
		start = total;

	s64 eff_len;
	if (length >= 0) {
		eff_len = length;
		if (start + eff_len > total)
			eff_len = total - start;
	} else { /* negative length: omit (-length) bytes from the end */
		eff_len = total - start + length;
		if (eff_len < 0)
			eff_len = 0;
	}

	string *sub = string_create((u64)eff_len);
	sub->length = sub->capacity;
	memcpy(sub->data, ((char *)s->data) + start, (size_t)eff_len);
	_string_zero_guard(sub);
	return sub;
}

char *string_c_str(string *s)
{
	if(!s->data)
		return "";
	_string_zero_guard(s);
	return (char *)s->data;
}

void string_append_buffer(string *dest, void *src, u64 src_length)
{
	if(!src)
		return;
	u64 new_len = dest->length + src_length;
	if (dest->capacity < new_len)
		string_set_capacity(dest, new_len);
	memcpy(((char *)dest->data) + dest->length, src, src_length);
	dest->length = new_len;
	_string_zero_guard(dest);
}

void string_append_string(string *dest, string *src)
{
	string_append_buffer(dest, src->data, src->length);
}

void string_append_c_str(string *dest, char *cstr)
{
	string_append_buffer(dest, cstr, strlen(cstr));
}

void string_clear(string *s)
{
	s->length = 0;
	_string_zero_guard(s);
}

void string_free(string *s)
{
	if (s) {
		free(s->data);
		free(s);
	}
}
