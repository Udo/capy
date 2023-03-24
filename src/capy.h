#pragma once

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _TCC_H
#define mem_free		tcc_free
#define mem_alloc		tcc_malloc
#define mem_calloc		calloc
#define mem_realloc		tcc_realloc
#else
#define mem_free		free
#define mem_alloc		malloc
#define mem_calloc		calloc
#define mem_realloc		realloc
#endif

#define string_initial_capacity 16

#ifndef countof
#define countof(array) (sizeof(array) / sizeof(array[0]))
#endif

#ifndef u8
typedef unsigned char 	u8;
typedef char	 		s8;
typedef char	 		b8;
typedef unsigned short 	u16;
typedef short			s16;
typedef unsigned int 	u32;
typedef int			 	s32;
typedef float		 	f32;
typedef unsigned long 	u64;
typedef long			s64;
typedef double		 	f64;
#endif

void panic(char* msg)
{
	printf("panic: %s\n", msg);
	exit(-1);
}

struct string {
	s32 length;
	s32 capacity;
	u8* bytes;
	// this is a hack to make nibbling faster at the front of the string:
	// in that case we move the 'bytes' pointer to an offset, whereas the allocated memory block
	// pointer remains safely stored in 'backing_store'. this means we defer any memcpy to events
	// that need to do string_recap()
	u8* backing_store;
};

struct hashmap_item {
	u64 hash;
	void* data;
	struct hashmap_item* next;
	struct hashmap_item* next_in_order;
	struct hashmap_item* prev_in_order;
	s32 bucket_idx;
};

struct hashmap {
	s32 length;
	s32 capacity;
	u64 tag;
	b8 is_resizing;
	struct hashmap_item* buckets;
	struct hashmap_item* first_in_order;
	struct hashmap_item* last_in_order;
};

struct list_item {
	void* data;
	struct list_item* next;
	struct list_item* prev;
};

struct list {
	s32 length;
	u64 tag;
	struct list_item* first;
	struct list_item* last;
};

void string_init(struct string* s);
struct string* string_create();
struct string* string_create_with_chars(char* initial_value);
struct string* string_clone(struct string* src);
void string_recap(struct string* s, s32 new_capacity);
void string_append_bytes(struct string* s, void* addr, s32 length);
void string_assign_bytes(struct string* s, void* addr, s32 length);
void string_append_chars(struct string* s, char* s2);
void string_append_s32(struct string* s, s32 i);
void string_assign_chars(struct string* s, char* s2);
struct string* string_substr(struct string* src, s32 start, s32 count);
s32 string_pos(struct string* src, struct string* find, s32 from_pos);
u8 string_uchar(u8 c);
s32 string_ipos(struct string* src, struct string* find, s32 from_pos);
b8 string_contains(struct string* src, struct string* find, s32 from_pos);
struct string* string_trim(struct string* src);
u8 string_char_at(struct string* src, s32 pos);
struct string* string_to_upper(struct string* src);
struct string* string_to_lower(struct string* src);
struct string* string_nibble(struct string* src, struct string* find);
s32 string_compare(struct string* s1, struct string* s2);
b8 string_equals(struct string* s1, struct string* s2);
s32 string_to_s32(struct string* s1);
f32 string_to_f32(struct string* s1);
s64 string_to_s64(struct string* s1);
f64 string_to_f64(struct string* s1);
struct string* string_replace(struct string* src, struct string* find, struct string* rwith);
struct string* string_ireplace(struct string* src, struct string* find, struct string* rwith);
void string_free(struct string* s);
u64 hash_into_u64(u8* data, u32 length);
void string_append(struct string* s, struct string* a);
u64 string_hash(struct string* s);
u64 char_hash(char* s);
u64 u64_hash(u64 v);
s32 file_size(struct string* file_name) ;
struct string* file_get_contents(struct string* file_name);
b8 file_put_contents(struct string* file_name, struct string* content);

struct list* list_create();
void list_free(struct list* l);
void list_append(struct list* l, void* item_data);
void list_prepend(struct list* l, void* item_data);
void* list_delete_item(struct list* l, struct list_item* it);
void* list_pop(struct list* l);
void* list_shift(struct list* l);
void list_insert_after(struct list* l, struct list_item* after, void* item_data);
struct string* list_join(struct list* l, struct string* s);
struct list* string_split(struct string* src, struct string* delim);

struct hashmap* hashmap_create();
void hashmap_free_buckets(struct hashmap_item* buckets, s32 count);
void hashmap_free(struct hashmap* h);
void hashmap_order_add(struct hashmap* h, struct hashmap_item* it);
void hashmap_order_delete(struct hashmap* h, struct hashmap_item* it);
void hashmap_recap(struct hashmap* h, s32 new_capacity);
struct hashmap_item* hashmap_get_entry(struct hashmap* h, u64 hash);
void* hashmap_get(struct hashmap* h, u64 hash);
s32 next_power_of_2(s32 n);
void hashmap_insert(struct hashmap* h, u64 hash, void* item_data);
struct hashmap_item* hashmap_first_entry(struct hashmap* h);
struct hashmap_item* hashmap_next_entry(struct hashmap* h, struct hashmap_item* it);
void* hashmap_delete_entry(struct hashmap* h, struct hashmap_item* it);
void* hashmap_delete(struct hashmap* h, u64 hash);
struct list* hashmap_values(struct hashmap* h);
struct list* hashmap_hashkeys(struct hashmap* h);

void string_init(struct string* s)
{
	s->capacity = string_initial_capacity;
	s->bytes = s->backing_store = mem_calloc(1, s->capacity+1);
}

struct string* string_create()
{
	struct string* result;
	result = mem_calloc(sizeof(struct string), 1);
	if(!result)
		panic("string_create() alloc failed");
	string_init(result);
	return result;
}

struct string* string_create_with_chars(char* initial_value)
{
	struct string* result;
	result = string_create();
	string_assign_chars(result, initial_value);
	return result;
}

struct string* string_clone(struct string* src)
{
	struct string* result;
	result = string_create();
	if(src && src->length > 0)
		string_assign_bytes(result, src->bytes, src->length);
	return result;
}

void string_recap(struct string* s, s32 new_capacity)
{
	if(new_capacity <= string_initial_capacity)
		new_capacity = string_initial_capacity;
	if(s->backing_store != s->bytes)
	{
		//printf("string_recap: reset view, %i chars copied \n", s->length+1);
		memcpy(s->backing_store, s->bytes, s->length+1);
	}
	if(s->backing_store == 0)
		s->backing_store = mem_alloc(new_capacity+1);
	else
		s->backing_store = mem_realloc(s->backing_store, new_capacity+1);
	if(!s->backing_store)
		panic("string_recap() realloc failed");
	if(new_capacity > s->capacity)
		memset(s->backing_store + s->capacity, 0, new_capacity - s->capacity);
	s->bytes = s->backing_store;
	s->capacity = new_capacity;
	//printf("- recap %p to %i\n", s, new_capacity);
}

void string_append_bytes(struct string* s, void* addr, s32 length)
{
	s32 new_length;
	if(length < 0 || addr == 0)
		return;
	//printf("- append to %p with length +%i\n", s, length);
	new_length = length + s->length;
	if(s->capacity < new_length || s->capacity < 8)
		string_recap(s, new_length);
	memcpy(s->bytes + s->length, addr, length);
	s->length = new_length;
	memset(s->bytes + s->length, 0, 1);
}

void string_assign_bytes(struct string* s, void* addr, s32 length)
{
	s->length = 0;
	string_append_bytes(s, addr, length);
}

void string_append_s32(struct string* s, s32 i)
{
	char sbuf[12];
	sprintf(sbuf, "%i", i);
	string_append_chars(s, sbuf);
}

void string_append_chars(struct string* s, char* s2)
{
	string_append_bytes(s, s2, strlen(s2));
}

void string_assign_chars(struct string* s, char* s2)
{
	string_assign_bytes(s, s2, strlen(s2));
}

struct string* string_substr(struct string* src, s32 start, s32 count)
{
	struct string* result;

	result = string_create();
	if(start < 0)
	{
		// start < 0 means: count from the end of the string
		start = src->length + start;
		if(start < 0)
			start = 0;
	}
	if(count == 0)
	{
		return result;
	}
	if(count < 0)
	{
		// if count < 0: count from the end of the string
		count = (src->length + count) - start;
		if(count < 0)
			count = 0;
	}
	if(start+count > src->length)
	{
		count = src->length - start;
	}
	if(count > 0 && start < src->length)
	{
		string_assign_bytes(result, src->bytes + start, count);
	}
	return result;
}

s32 string_pos(struct string* src, struct string* find, s32 from_pos)
{
	s32 found_chars;
	if(find->length == 0 || src->length == 0 || from_pos < 0 ||
		from_pos > src->length || find->length > src->length-from_pos)
		return(-1);
	found_chars = 0;
	for(s32 i = from_pos; i < src->length+1; i++)
	{
		if(*(src->bytes+i) == *(find->bytes+found_chars))
		{
			found_chars += 1;
			if(found_chars >= find->length)
				return(1 + i - found_chars);
		}
		else
		{
			found_chars = 0;
		}
	}
	return(-1);
}

u8 string_uchar(u8 c)
{
	u8 result;
	result = c;
	switch(c)
	{
		case 65 ... 90:
		{
			result += 32;
			break;
		}
	}
	return result;
}

s32 string_ipos(struct string* src, struct string* find, s32 from_pos)
{
	s32 found_chars;
	if(find->length == 0 || src->length == 0 || from_pos < 0 ||
		from_pos > src->length || find->length > src->length-from_pos)
		return(-1);
	found_chars = 0;
	for(s32 i = from_pos; i < src->length+1; i++)
	{
		if(string_uchar(*(src->bytes+i)) == string_uchar(*(find->bytes+found_chars)))
		{
			found_chars += 1;
			if(found_chars >= find->length)
				return(1 + i - found_chars);
		}
		else
		{
			found_chars = 0;
		}
	}
	return(-1);
}

b8 string_contains(struct string* src, struct string* find, s32 from_pos)
{
	return string_pos(src, find, from_pos) != -1;
}

struct string* string_trim(struct string* src)
{
	struct string* result;
	s32 i0, i1;

	result = string_create();
	for(i0 = 0; i0 < src->length+1; i0++)
	{
		if(!isspace(*(src->bytes+i0)))
			break;
	}
	if(i0 >= src->length-1)
		return result;
	for(i1 = src->length-1; i1 >= 0; i1--)
	{
		if(!isspace(*(src->bytes+i1)))
			break;
	}
	if(i1 < i0)
		return result;
	string_assign_bytes(result, src->bytes+i0, 1+i1-i0);
	return result;
}

u8 string_char_at(struct string* src, s32 pos)
{
	if(pos < 0)
		pos = src->length + pos;
	if(pos < 0 || pos >= src->length)
		return(0);
	return(*(src->bytes + pos));
}

struct string* string_to_lower(struct string* src)
{
	struct string* result;
	result = string_create();
	if(src->length == 0)
		return result;
	string_recap(result, src->length);
	for(s32 i = 0; i < src->length+1; i++)
	{
		u8 c;
		c = *(src->bytes + i);
		switch(c)
		{
			case 65 ... 90:
			{
				c += 32;
				break;
			}
		}
		*(result->bytes+i) = c;
	}
	result->length = src->length;
	return(result);
}

struct string* string_to_upper(struct string* src)
{
	struct string* result = string_create();
	if(src->length == 0)
		return result;
	string_recap(result, src->length);
	for(s32 i = 0; i < src->length+1; i++)
	{
		u8 c;
		c = *(src->bytes + i);
		switch(c)
		{
			case 97 ... 122:
			{
				c -= 32;
				break;
			}
		}
		*(result->bytes+i) = c;
	}
	result->length = src->length;
	return(result);
}

struct string* string_nibble(struct string* src, struct string* find)
{
	s32 p;
	struct string* result;

	result = string_create();
	p = string_pos(src, find, 0);
	if(p == -1)
	{
		string_assign_bytes(result, src->bytes, src->length);
		string_assign_chars(src, "");
	}
	else
	{
		string_assign_bytes(result, src->bytes, p);
		src->bytes += p + find->length;
		src->capacity -= p + find->length;
		src->length -= p + find->length;
	}
	return result;
}

s32 string_compare(struct string* s1, struct string* s2)
{
	s32 len;
	len = s1->length < s2->length ? s1->length : s2->length;
	for(s32 i = 0; i < len; i++)
	{
		u8 c1;
		u8 c2;
		c1 = *(s1->bytes + i);
		c2 = *(s2->bytes + i);
		if(c1 != c2)
			return c1 - c2;
	}
	if(s1->length == s2->length)
		return 0;
	return(s1->length - s2->length);
}

b8 string_equals(struct string* s1, struct string* s2)
{
	if(s1->length != s2->length)
		return false;
	return string_compare(s1, s2) == 0;
}

s64 string_to_s64(struct string* s1)
{
	return(atol(s1->bytes));
}

s32 string_to_s32(struct string* s1)
{
	return(atoi(s1->bytes));
}

f64 string_to_f64(struct string* s1)
{
	return(atof(s1->bytes));
}

f32 string_to_f32(struct string* s1)
{
	return(strtof(s1->bytes, 0));
}

struct string* string_replace(struct string* src, struct string* find, struct string* rwith)
{
	struct string* result;
	s32 offset;

	result = string_create();
	if(find->length == 0)
	{
		string_assign_bytes(result, src->bytes, src->length);
		return result;
	}
	offset = 0;
	do {
		s32 p = string_pos(src, find, offset);
		if(p == -1)
		{
			string_append_bytes(result, src->bytes + offset, src->length - offset);
			return result;
		}
		if(p > 0)
			string_append_bytes(result, src->bytes + offset, p - offset);
		if(rwith->length > 0)
			string_append_bytes(result, rwith->bytes, rwith->length);
		offset += p + find->length;
	} while(offset < src->length);
	return result;
}

struct string* string_ireplace(struct string* src, struct string* find, struct string* rwith)
{
	struct string* result;
	s32 offset;
	result = string_create();
	if(find->length == 0)
	{
		string_assign_bytes(result, src->bytes, src->length);
		return result;
	}
	offset = 0;
	do {
		s32 p;
		p = string_ipos(src, find, offset);
		if(p == -1)
		{
			string_append_bytes(result, src->bytes + offset, src->length - offset);
			return result;
		}
		if(p > 0)
			string_append_bytes(result, src->bytes + offset, p - offset);
		if(rwith->length > 0)
			string_append_bytes(result, rwith->bytes, rwith->length);
		offset += p + find->length;
	} while(offset < src->length);
	return result;
}

u64 hash_into_u64(u8* data, u32 length)
{
	u64 hash = 0xfb54e02cf3c0a69;
	u64 salt = 0x22a11c9087322ab3;
	u8 last = 0xc1;

	for (u32 i = 0; i < length; i++) {
		hash += ((i + salt + data[i]*last) ^ (data[i]*hash)) + (hash << (i % 31));
		last = data[i];
	}

	return hash;
}

void string_free(struct string* s)
{
	if(s == 0)
		return;
	if(s->backing_store)
		mem_free(s->backing_store);
	mem_free(s);
}

void string_append(struct string* s, struct string* a)
{
	if(!a)
		return;
	string_append_bytes(s, a->bytes, a->length);
}

u64 string_hash(struct string* s)
{
	return(hash_into_u64(s->bytes, s->length));
}

u64 char_hash(char* s)
{
	return(hash_into_u64(s, strlen(s)));
}

u64 u64_hash(u64 v)
{
	return(hash_into_u64((void*)&v, 8));
}

#include <sys/stat.h>

s32 file_size(struct string* file_name) {
    struct stat st;
    if(stat(file_name->bytes, &st) != 0)
        return 0;
    return st.st_size;
}

struct string* file_get_contents(struct string* file_name)
{
	struct string* result;
	FILE *file;
    s32 fsize;
    s32 read_size;

    result = string_create();
    file = fopen(file_name->bytes, "rb");
    if (!file) return result;

	fsize = file_size(file_name);
	if(fsize == 0) return result;

	string_recap(result, fsize);
	read_size = fread(result->bytes, 1, fsize, file);
    fclose(file);
    result->length = read_size;

    return result;
}

b8 file_put_contents(struct string* file_name, struct string* content)
{
    FILE *fp;
    fp = fopen(file_name->bytes, "wb");
    if(!fp)
		return false;
	fwrite(content->bytes, 1, content->length, fp);
	fclose(fp);
	return(true);
}

struct list* list_create()
{
	struct list* result;
	result = mem_calloc(sizeof(struct list), 1);
	if(!result)
		panic("list_create() alloc failed");
	return result;
}

void list_free(struct list* l)
{
	struct list_item* i;
	if(l == 0)
		return;
	i = l->first;
	while(i)
	{
		struct list_item* n;
		n = i->next;
		mem_free(i);
		i = n;
	}
	mem_free(l);
}

void list_append(struct list* l, void* item_data)
{
	struct list_item* new_item;

	l->length += 1;
	new_item = mem_calloc(sizeof(struct list_item), 1);
	new_item->data = item_data;
	new_item->prev = l->last;
	if(!l->first)
	{
		l->first = new_item;
		l->last = new_item;
	}
	else
	{
		l->last->next = new_item;
		l->last = new_item;
	}
}

void list_prepend(struct list* l, void* item_data)
{
	struct list_item* new_item;

	l->length += 1;
	new_item = mem_calloc(sizeof(struct list_item), 1);
	new_item->data = item_data;
	new_item->next = l->first;
	if(!l->first)
	{
		l->first = new_item;
		l->last = new_item;
	}
	else
	{
		l->first->prev = new_item;
		l->first = new_item;
	}
}

void* list_delete_item(struct list* l, struct list_item* it)
{
	struct list_item* pi;
	void* r;

	if(!l->last)
		return 0;
	if(it->prev)
	{
		pi = it->prev;
		pi->next = it->next;
	}
	if(it->next)
	{
		pi = it->next;
		pi->prev = it->prev;
	}
	if(l->first == it)
		l->first = it->next;
	if(l->last == it)
		l->last = it->prev;
	r = it->data;
	mem_free(it);
	l->length -= 1;
	return r;
}

void* list_pop(struct list* l)
{
	return list_delete_item(l, l->last);
}

void* list_shift(struct list* l)
{
	return list_delete_item(l, l->first);
}

void list_insert_after(struct list* l, struct list_item* after, void* item_data)
{
	struct list_item* new_item;

	if(!after)
		return list_append(l, item_data);
	l->length += 1;
	new_item = mem_calloc(sizeof(struct list_item), 1);
	new_item->data = item_data;
	new_item->next = after->next;
	new_item->prev = after;
	if(after->next)
		after->next->prev = new_item;
	after->next = new_item;
	if(l->last == after)
		l->last = new_item;
}

struct string* list_join(struct list* l, struct string* s)
{
	struct string* result;
	struct list_item* it;

	result = string_create();
	it = l->first;
	while(it)
	{
		string_append(result, it->data);
		if(s && s->length > 0)
			string_append(result, it->data);
		it = it->next;
	}
	return result;
}

struct list* string_split(struct string* src, struct string* delim)
{
	struct list* result;
	s32 start;
	s32 pos;

	result = list_create();
	pos = string_pos(src, delim, 0);
	start = 0;
	if(pos == -1)
	{
		list_append(result, string_clone(src));
		return result;
	}
	while(pos != -1)
	{
		list_append(result, string_substr(src, start, pos-start));
		start = pos + delim->length;
		pos = string_pos(src, delim, start);
	}
	if(start < src->length-1)
		list_append(result, string_substr(src, start, src->length));
	return result;
}

s32 next_power_of_2(s32 n)
{
    int next;
    next = 4;
    while (next <= n)
        next <<= 1;
    return next;
}


struct hashmap* hashmap_create()
{
	struct hashmap* result;
	result = mem_calloc(sizeof(struct hashmap), 1);
	if(!result)
		panic("hashmap_create() alloc failed");
	return result;
}

void hashmap_free_buckets(struct hashmap_item* buckets, s32 count)
{
	struct hashmap_item* item;
	for(s32 i = 0; i < count; i++)
	{
		item = &buckets[i];
		if(item->hash)
		{
			item = item->next;
			while(item)
			{
				struct hashmap_item* item_next = item->next;
				mem_free(item);
				item = item_next;
			}
		}
	}
	mem_free(buckets);
}

void hashmap_free(struct hashmap* h)
{
	hashmap_free_buckets(h->buckets, h->capacity);
	mem_free(h);
}

void hashmap_order_add(struct hashmap* h, struct hashmap_item* it)
{
	if(!h->first_in_order)
	{
		h->first_in_order = it;
		h->last_in_order = it;
		it->next_in_order = 0;
		it->prev_in_order = 0;
	}
	else
	{
		h->last_in_order->next_in_order = it;
		it->prev_in_order = h->last_in_order;
		it->next_in_order = 0;
		h->last_in_order = it;
	}
}

void hashmap_order_delete(struct hashmap* h, struct hashmap_item* it)
{
	if(it == h->first_in_order)
		h->first_in_order = it->next_in_order;
	if(it == h->last_in_order)
		h->last_in_order = it->prev_in_order;
	if(it->prev_in_order)
		it->prev_in_order->next_in_order = it->next_in_order;
	if(it->next_in_order)
		it->next_in_order->prev_in_order = it->prev_in_order;
}

void hashmap_recap(struct hashmap* h, s32 new_capacity)
{
	struct hashmap_item* old_buckets;
	struct hashmap_item* old_first_in_order;
	//struct hashmap_item* old_last_in_order;
	s32 old_capacity;

	old_buckets = h->buckets;
	old_first_in_order = h->first_in_order;
	//old_last_in_order = h->last_in_order;
	h->first_in_order = 0;
	h->last_in_order = 0;
	old_capacity = h->capacity;
	h->is_resizing = true;
	h->capacity = new_capacity;
	h->buckets = mem_calloc(sizeof(struct hashmap_item), new_capacity);
	if(old_buckets)
	{
		struct hashmap_item* it;
		it = old_first_in_order;
		while(it)
		{
			hashmap_insert(h, it->hash, it->data);
			it = it->next_in_order;
		}
		hashmap_free_buckets(old_buckets, old_capacity);
	}
	h->is_resizing = false;
}

struct hashmap_item* hashmap_get_entry(struct hashmap* h, u64 hash)
{
	s32 idx;
	struct hashmap_item* bucket;

	if(!h->capacity)
		return 0;

	idx = hash % h->capacity;
	bucket = &h->buckets[idx];
	if(!bucket || !bucket->hash)
		return 0;
	while(bucket)
	{
		if(bucket->hash == hash)
			return bucket;
		bucket = bucket->next;
	}
	return 0;
}

void* hashmap_get(struct hashmap* h, u64 hash)
{
	struct hashmap_item* entry;
	entry = hashmap_get_entry(h, hash);
	if(entry)
		return entry->data;
	return 0;
}

void hashmap_insert(struct hashmap* h, u64 hash, void* item_data)
{
	s32 idx;
	struct hashmap_item* bucket;

	if(h->length == 0)
		hashmap_recap(h, 4);
	else if(h->length > h->capacity*2)
		hashmap_recap(h, next_power_of_2(h->length));
	idx = hash % h->capacity;
	bucket = &h->buckets[idx];
	if(bucket->hash) // already occupied?
	{
		struct hashmap_item* dupe;
		dupe = hashmap_get_entry(h, hash);
		if(dupe)
		{
			dupe->data = item_data;
		}
		else
		{
			struct hashmap_item* new_item;
			new_item = mem_calloc(sizeof(struct hashmap_item), 1);
			new_item->hash = hash;
			new_item->data = item_data;
			new_item->next = bucket->next;
			new_item->bucket_idx = idx;
			hashmap_order_add(h, new_item);
			bucket->next = new_item;
			if(!h->is_resizing)
				h->length += 1;
		}
	}
	else
	{
		bucket->hash = hash;
		bucket->data = item_data;
		bucket->bucket_idx = idx;
		hashmap_order_add(h, bucket);
		if(!h->is_resizing)
			h->length += 1;
	}
}

struct hashmap_item* hashmap_first_entry(struct hashmap* h)
{
	return h->first_in_order;
}

struct hashmap_item* hashmap_next_entry(struct hashmap* h, struct hashmap_item* it)
{
	return it->next_in_order;
}

void* hashmap_delete_entry(struct hashmap* h, struct hashmap_item* it)
{
	s32 idx;
	void* r;
	struct hashmap_item* bucket;

	if(!it)
		return 0;
	idx = it->bucket_idx;
	r = it->data;
	hashmap_order_delete(h, it);
	bucket = &h->buckets[idx];
	if(bucket == it)
	{
		if(it->next)
		{
			bucket->hash = it->next->hash;
			bucket->data = it->next->data;
			mem_free(it);
		}
		else
		{
			memset(bucket, 0, sizeof(struct hashmap_item));
		}
	}
	else
	{
		while(bucket->next != it)
			bucket = bucket->next;
		bucket->next = it->next;
		mem_free(it);
	}
	return r;
}


void* hashmap_delete(struct hashmap* h, u64 hash)
{
	struct hashmap_item* it;
	it = hashmap_get_entry(h, hash);
	if(!it)
		return 0;
	return hashmap_delete_entry(h, it);
}

struct list* hashmap_values(struct hashmap* h)
{
	struct list* result;
	struct hashmap_item* it;

	result = list_create();
	it = hashmap_first_entry(h);
	while(it)
	{
		list_append(result, it->data);
		it = it->next_in_order;
	}
	return result;
}

struct list* hashmap_hashkeys(struct hashmap* h)
{
	struct list* result;
	struct hashmap_item* it;

	result = list_create();
	it = hashmap_first_entry(h);
	while(it)
	{
		list_append(result, (void*)it->hash);
		it = it->next_in_order;
	}
	return result;
}


