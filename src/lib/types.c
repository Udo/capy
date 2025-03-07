#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <execinfo.h>
#include <stdlib.h>

typedef int8_t	  s8;
typedef uint8_t   u8;
typedef int16_t   s16;
typedef uint16_t  u16;
typedef int32_t   s32;
typedef uint32_t  u32;
typedef int64_t   s64;
typedef uint64_t  u64;

typedef float	  f32;
typedef double	  f64;

typedef intptr_t  isize;
typedef uintptr_t usize;

typedef struct string string;
struct string
{
	u8* data;
	u64 length;
	u64 capacity;
};

typedef struct mem_arena mem_arena;
struct mem_arena
{
	void* data;
	u64 length;
	u64 capacity;
	u32 chunk_count;
	u64 last_alloc_offset;
	u64 last_alloc_size;
};

mem_arena* default_arena = 0;

#define LOG_MSG(msg) \
    fprintf(stdout, "[%s:%d] %s\n", __FILE__, __LINE__, msg); \

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

u64 aligned_number(u64 n)
{
	return (n +7ULL) & ~7ULL;
}

// ----------------------------------------------------------------

mem_arena* arena_create(u64 capacity)
{
	mem_arena *ma = calloc(1, sizeof(mem_arena));
	if (!ma)
		fatal_error("arena_create() calloc failed");
	ma->data = calloc(1, capacity);
	ma->capacity = capacity;
	printf("[DEBUG] created arena with size %lu starting at %p\n", capacity, ma->data);
	return ma;
}

void* arena_alloc(mem_arena* arena, u64 asize)
{
	// Round requested size up to nearest 8 bytes.
	u64 aligned_asize = aligned_number(asize);
	u64 total = aligned_asize + sizeof(u64);
	if (arena->length + total > arena->capacity)
		fatal_error("arena_alloc() capacity exhausted");
	char* base = (char*)arena->data;
	u64 offset = arena->length;
	u64* header = (u64*)(base + offset);
	*header = aligned_asize;  // store aligned allocation size in header
	void* result = (void*)(header + 1);  // user pointer follows header, 8-byte aligned
	arena->length += total;
	arena->chunk_count += 1;
	arena->last_alloc_offset = offset;
	arena->last_alloc_size = aligned_asize;
	//printf("[DEBUG] arena_alloc: allocated %lu bytes at %p (header offset %lu)\n", aligned_asize, result, offset);
	return result;
}

void* arena_realloc(mem_arena* arena, void* old_addr, u64 new_size)
{
    char* base = (char*)arena->data;
    // The header is stored immediately before the returned pointer.
    u64* old_header = ((u64*)old_addr) - 1;
    u64 old_alloc_size = *old_header;
    // Round new_size up to the next multiple of 8.
    u64 aligned_new_size = aligned_number(new_size);

    // Special-case: if the new (aligned) size fits within the old allocation, do nothing.
    if (aligned_new_size <= old_alloc_size)
        return old_addr;

    if ((void*)old_header == (void*)(base + arena->last_alloc_offset)) {
        // Top block: extend in place.
        u64 total_new = aligned_new_size + sizeof(u64);
        if (arena->last_alloc_offset + total_new > arena->capacity)
            fatal_error("arena_realloc() capacity exhausted for top block");
        *old_header = aligned_new_size;  // update stored allocation size
        arena->length = arena->last_alloc_offset + total_new;
        arena->last_alloc_size = aligned_new_size;
        //printf("[DEBUG] arena_realloc: reallocated in place to %lu bytes at %p\n",
        //       aligned_new_size, old_addr);
        return old_addr;
    } else {
        // Non-top block: allocate new memory and copy the old data.
        void* new_addr = arena_alloc(arena, aligned_new_size);
        u64 copy_size = (old_alloc_size < new_size) ? old_alloc_size : new_size;
        memcpy(new_addr, old_addr, copy_size);
        //printf("[DEBUG] arena_realloc: reallocated by copy to %lu bytes at %p\n",
        //       aligned_new_size, new_addr);
        return new_addr;
    }
}

void arena_print_stats(mem_arena* arena)
{
	printf("[DEBUG] arena_stats: allocated %lu of %lu bytes in %i chunks at %p\n", arena->length, arena->capacity, arena->chunk_count, arena->data);
}

void* arena_free(mem_arena* arena, void* chunk_addr)
{
	// a null-op
	(void)arena;
	(void)chunk_addr;
	return NULL;
}

bool is_linebreak(u8 c)
{
	return (c == '\n');
}

bool is_space(u8 c)
{
	return (c == ' ')  || (c == '\t') || (c == '\n') ||
		   (c == '\v') || (c == '\f') || (c == '\r');
}

bool is_punct(u8 c)
{
	return ((c >= 33 && c <= 47)  ||
			(c >= 58 && c <= 64)  ||
			(c >= 91 && c <= 96)  ||
			(c >= 123 && c <= 126));
}

bool is_alpha(u8 c)
{
	return ((c >= 'A' && c <= 'Z') ||
			(c >= 'a' && c <= 'z'));
}

bool is_digit(u8 c)
{
	return (c >= '0' && c <= '9');
}

void _string_zero_guard(string *s)
{
	if(s->data)
		((char *)s->data)[s->length] = '\0';
}

u64 string_growsize(string* dest, u64 min_new_length)
{
	min_new_length = aligned_number(min_new_length+1)-1;
	u64 natural_growth = aligned_number((u64)((f64)dest->capacity*1.5))-1;
	//printf("[STRING] string_growsize %lu ask:%lu nat:%lu\n", dest->capacity, min_new_length, natural_growth);
	if(natural_growth < min_new_length)
		return min_new_length;
	else
		return natural_growth;
}

void string_set_capacity(string *s, u64 new_cap)
{
	if (new_cap <= s->length) return;
	u64 msize = aligned_number(new_cap +1);
	s->capacity = msize-1;
	if(s->data)
	{
		s->data = arena_realloc(default_arena, s->data, msize);
	}
	else
	{
		s->data = arena_alloc(default_arena, msize);
	}
	//printf("[STRING] string_set_capacity ask:%lu mem:%lu cap:%lu\n", new_cap, msize, s->capacity);
	if (!s->data)
		fatal_error("alloc failed");
	_string_zero_guard(s);
}

string *string_create(u64 initial_capacity)
{
	string *s = arena_alloc(default_arena, sizeof(string));
	if (!s)
		fatal_error("malloc failed");
	s->length = 0;
	string_set_capacity(s, initial_capacity);
	return s;
}

string *string_create_from_cstr(const char *cstr)
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
	return string_create_from_cstr(buf);
}

string *string_read_file(const char *filename)
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

bool string_write_file(const string *s, const char *filename)
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

void string_set_length(string *s, s64 new_len)
{
	if(new_len < 0)
		new_len = s->length + new_len;
	s->length = new_len;
	_string_zero_guard(s);
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
	sub->length = (u64)eff_len;
	memcpy(sub->data, ((char *)s->data) + start, (size_t)eff_len);
	_string_zero_guard(sub);
	return sub;
}

char *string_cstr(string *s)
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
		string_set_capacity(dest, string_growsize(dest, new_len));
	memcpy(((char *)dest->data) + dest->length, src, src_length);
	dest->length = new_len;
	_string_zero_guard(dest);
}

void string_append_string(string *dest, string *src)
{
	string_append_buffer(dest, src->data, src->length);
}

void string_append_cstr(string *dest, char *cstr)
{
	string_append_buffer(dest, cstr, strlen(cstr));
}

void string_append_char(string *dest, char ch)
{
	u64 new_len = dest->length + 1;
	if (dest->capacity < new_len)
		string_set_capacity(dest, string_growsize(dest, new_len));
	dest->data[dest->length] = ch;
	dest->length += 1;
	_string_zero_guard(dest);
}

void string_clear(string *s)
{
	s->length = 0;
	_string_zero_guard(s);
}

void string_free(string *s)
{
	if (s) {
		arena_free(default_arena, s->data);
		arena_free(default_arena, s);
	}
}

bool string_equals(string* s1, string* s2, bool case_sensitive)
{
	if (s1->length != s2->length)
		return false;
	if (case_sensitive)
		return (memcmp(s1->data, s2->data, s1->length) == 0);

	for (u64 i = 0; i < s1->length; i++)
	{
		u8 c1 = s1->data[i];
		u8 c2 = s2->data[i];
		if (c1 >= 'A' && c1 <= 'Z') c1 += 'a' - 'A';
		if (c2 >= 'A' && c2 <= 'Z') c2 += 'a' - 'A';
		if (c1 != c2)
			return false;
	}
	return true;
}

bool string_equals_cstr(string* s1, char* s2, bool case_sensitive)
{
    u64 len2 = strlen(s2);
    if (s1->length != len2)
        return false;
    if (case_sensitive)
        return (memcmp(s1->data, s2, s1->length) == 0);

    for (u64 i = 0; i < s1->length; i++)
    {
        u8 c1 = s1->data[i];
        u8 c2 = s2[i];
        if (c1 >= 'A' && c1 <= 'Z') c1 += 'a' - 'A';
        if (c2 >= 'A' && c2 <= 'Z') c2 += 'a' - 'A';
        if (c1 != c2)
            return false;
    }
    return true;
}

u64 string_pos(string* haystack, string* needle, bool case_sensitive)
{
	if (needle->length == 0)
		return 0;
	if (haystack->length < needle->length)
		return (u64)-1;

	for (u64 i = 0; i <= haystack->length - needle->length; i++)
	{
		bool found = true;
		if (case_sensitive)
		{
			if (memcmp(haystack->data + i, needle->data, needle->length) == 0)
				return i;
			continue;
		}
		for (u64 j = 0; j < needle->length; j++)
		{
			u8 c1 = haystack->data[i + j];
			u8 c2 = needle->data[j];
			if (c1 >= 'A' && c1 <= 'Z') c1 += 'a' - 'A';
			if (c2 >= 'A' && c2 <= 'Z') c2 += 'a' - 'A';
			if (c1 != c2) {
				found = false;
				break;
			}
		}
		if (found)
			return i;
	}
	return (u64)-1;
}

string* string_to_lowercase(string* s)
{
	string* result = string_create(s->length);
	result->length = s->length;
	for (u64 i = 0; i < s->length; i++) {
		u8 c = s->data[i];
		if (c >= 'A' && c <= 'Z')
			result->data[i] = c + ('a' - 'A');
		else
			result->data[i] = c;
	}
	_string_zero_guard(result);
	return result;
}

string* string_to_uppercase(string* s)
{
	string* result = string_create(s->length);
	result->length = s->length;
	for (u64 i = 0; i < s->length; i++) {
		u8 c = s->data[i];
		if (c >= 'a' && c <= 'z')
			result->data[i] = c - ('a' - 'A');
		else
			result->data[i] = c;
	}
	_string_zero_guard(result);
	return result;
}

string* string_trim(string* s)
{
	if (s->length == 0)
		return string_create(0);

	u64 start = 0;
	while (start < s->length && is_space(s->data[start]))
		start++;

	// If the entire string is whitespace, return an empty string.
	if (start == s->length)
		return string_create(0);

	u64 end = s->length - 1;
	while (end > start && is_space(s->data[end]))
		end--;

	u64 new_len = end - start + 1;
	string* result = string_create(new_len);
	result->length = new_len;
	memcpy(result->data, s->data + start, new_len);
	_string_zero_guard(result);
	return result;
}
