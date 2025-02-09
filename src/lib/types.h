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

typedef int8_t s8;
typedef uint8_t u8;
typedef int16_t s16;
typedef uint16_t u16;
typedef int32_t s32;
typedef uint32_t u32;
typedef int64_t s64;
typedef uint64_t u64;
typedef float f32;
typedef double f64;
typedef intptr_t isize;
typedef uintptr_t usize;
typedef struct string string;
struct string {
 void* data;
 u64 length;
 u64 capacity;
}
;
void print_stack_trace(void);
void fatal_error(const char *msg);
void _string_zero_guard(string *s);
void string_set_capacity(string *s, u64 new_cap);
string *string_create(u64 initial_capacity);
string *string_create_from_c_str(const char *cstr);
string *string_create_from_float(double value);
string *string_read_entire_file(const char *filename);
bool string_write_entire_file(const string *s, const char *filename);
string *string_substr(const string *s, s64 start, s64 length);
char *string_c_str(string *s);
void string_append_buffer(string *dest, void *src, u64 src_length);
void string_append_string(string *dest, string *src);
void string_append_c_str(string *dest, char *cstr);
void string_clear(string *s);
void string_free(string *s);

#endif /* SRC_LIB_TYPES_H */
