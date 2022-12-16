typedef unsigned char 	u8;
typedef char	 		s8;
typedef unsigned short 	u16;
typedef short			s16;
typedef unsigned int 	u32;
typedef int			 	s32;
typedef float		 	f32;
typedef unsigned long 	u64;
typedef long			s64;
typedef double		 	f64;

#define mem_alloc(sz)			malloc(sz)
#define mem_calloc(sz, esz)		calloc(sz, esz)
#define mem_realloc(ptr, sz)	realloc(ptr, sz)

struct string {
	u32 length;
	u32 capacity;
	u8* bytes;
};

void panic(char* msg)
{
	printf("panic: %s\n", msg);
	exit(-1);
}

struct string* string_create()
{
	struct string* result = mem_calloc(sizeof(struct string), 1);
	if(!result)
		panic("string_create() alloc failed");
	return result;
}

void recap_string(struct string* s, u32 new_capacity)
{
	printf("- recap string %p to %i\n", s, new_capacity);
	if(new_capacity == 0)
		return;
	if(s->bytes == 0)
		s->bytes = mem_alloc(new_capacity+1);
	else
		s->bytes = mem_realloc(s->bytes, new_capacity+1);
	if(!s->bytes)
		panic("recap_string() realloc failed");
	if(new_capacity > s->capacity)
		// do we need this?
		memset(s->bytes + s->capacity, 0, new_capacity - s->capacity);
	s->capacity = new_capacity;
}

void append_string_bytes(struct string* s, void* addr, u32 length)
{
	printf("- append to string %p :+ %i\n", s, length);
	if(length == 0 || addr == 0)
		return;
	u32 new_length = length + s->length;
	if(s->capacity < new_length)
		recap_string(s, new_length);
	memcpy(s->bytes + s->length, addr, length);
	s->length = new_length;
	memset(s->bytes + s->length, 0, 1);
}

void assign_string_bytes(struct string* s, void* addr, u32 length)
{
	s->length = 0;
	append_string_bytes(s, addr, length);
}

void append_string_chars(struct string* s, char* s2)
{
	append_string_bytes(s, s2, strlen(s2));
}

void assign_string_chars(struct string* s, char* s2)
{
	assign_string_bytes(s, s2, strlen(s2));
}

void free_string(struct string* s)
{
	if(s == 0)
		return;
	if(s->bytes)
		free(s->bytes);
	free(s);
}