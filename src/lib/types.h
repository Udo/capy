/* Auto-generated header from types.c */
/* DO NOT EDIT MANUALLY */
#ifndef SRC_LIB_TYPES_H
#define SRC_LIB_TYPES_H

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

extern mem_arena* default_arena ;

#define LOG_MSG(msg) \
    fprintf(stdout, "[%s:%d] %s\n", __FILE__, __LINE__, msg); \

void print_stack_trace(void);

void fatal_error(const char *msg);

u64 aligned_number(u64 n);



mem_arena* arena_create(u64 capacity);

void* arena_alloc(mem_arena* arena, u64 asize);

void* arena_realloc(mem_arena* arena, void* old_addr, u64 new_size);

void arena_print_stats(mem_arena* arena);

void* arena_free(mem_arena* arena, void* chunk_addr);

bool is_linebreak(u8 c);

bool is_space(u8 c);

bool is_punct(u8 c);

bool is_alpha(u8 c);

bool is_digit(u8 c);

void _string_zero_guard(string *s);

u64 string_growsize(string* dest, u64 min_new_length);

void string_set_capacity(string *s, u64 new_cap);

string *string_create(u64 initial_capacity);

string *string_create_from_cstr(const char *cstr);

string *string_create_from_float(double value);

string *string_read_file(const char *filename);

bool string_write_file(const string *s, const char *filename);

void string_set_length(string *s, s64 new_len);

string *string_substr(const string *s, s64 start, s64 length);

char *string_cstr(string *s);

void string_append_buffer(string *dest, void *src, u64 src_length);

void string_append_string(string *dest, string *src);

void string_append_cstr(string *dest, char *cstr);

void string_append_char(string *dest, char ch);

void string_clear(string *s);

void string_free(string *s);

bool string_equals(string* s1, string* s2, bool case_sensitive);

bool string_equals_cstr(string* s1, char* s2, bool case_sensitive);

u64 string_pos(string* haystack, string* needle, bool case_sensitive);

string* string_to_lowercase(string* s);

string* string_to_uppercase(string* s);

string* string_trim(string* s);

#endif /* SRC_LIB_TYPES_H */
