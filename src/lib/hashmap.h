/* Auto-generated header from hashmap.c */
/* DO NOT EDIT MANUALLY */
#ifndef SRC_LIB_HASHMAP_H
#define SRC_LIB_HASHMAP_H

#include "types.h"

typedef struct hashmap_entry hashmap_entry;
struct hashmap_entry
{
    string* key;
    void* value;
    u32 type_info;
    hashmap_entry* next;
};

typedef struct hashmap hashmap;
struct hashmap
{
    hashmap_entry** buckets;
    u32 capacity;
    u32 size;
};

hashmap* hashmap_create(u32 capacity);

u32 hashmap_hash(hashmap* map, string* key);

void hashmap_set(hashmap* map, string* key, void* value);

void* hashmap_get(hashmap* map, string* key);

void hashmap_remove(hashmap* map, string* key);

void hashmap_print(hashmap* map);


#endif /* SRC_LIB_HASHMAP_H */
