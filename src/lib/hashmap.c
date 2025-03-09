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

hashmap* hashmap_create(u32 capacity)
{
    hashmap* map = arena_alloc(default_arena, sizeof(hashmap));
    map->capacity = capacity;
    map->buckets = arena_alloc(default_arena, sizeof(hashmap_entry*) * capacity);
    for(u32 i = 0; i < capacity; i++)
    {
        map->buckets[i] = 0;
    }
    return map;
}

u32 hashmap_hash(hashmap* map, string* key)
{
    u32 hash = 0;
    for(u32 i = 0; i < key->length; i++)
    {
        hash = (hash << 5) + hash + key->data[i] + (i*3171) >> 7;
    }
    return hash % map->capacity;
}

void hashmap_set(hashmap* map, string* key, void* value)
{
    u32 hash = hashmap_hash(map, key);
    hashmap_entry* entry = map->buckets[hash];
    while(entry)
    {
        if(string_equals(entry->key, key, true))
        {
            entry->value = value;
            return;
        }
        entry = entry->next;
    }
    entry = arena_alloc(default_arena, sizeof(hashmap_entry));
    entry->key = key;
    entry->value = value;
    entry->next = map->buckets[hash];
    map->buckets[hash] = entry;
    map->size++;
}

void* hashmap_get(hashmap* map, string* key)
{
    u32 hash = hashmap_hash(map, key);
    hashmap_entry* entry = map->buckets[hash];
    while(entry)
    {
        if(string_equals(entry->key, key, true))
        {
            return entry->value;
        }
        entry = entry->next;
    }
    return 0;
}

void hashmap_remove(hashmap* map, string* key)
{
    u32 hash = hashmap_hash(map, key);
    hashmap_entry* entry = map->buckets[hash];
    hashmap_entry* prev = 0;
    while(entry)
    {
        if(string_equals(entry->key, key, true))
        {
            if(prev)
            {
                prev->next = entry->next;
            }
            else
            {
                map->buckets[hash] = entry->next;
            }
            map->size--;
            return;
        }
        prev = entry;
        entry = entry->next;
    }
}

void hashmap_print(hashmap* map)
{
    for(u32 i = 0; i < map->capacity; i++)
    {
        hashmap_entry* entry = map->buckets[i];
        while(entry)
        {
            printf("%s -> %p\n", entry->key->data, entry->value);
            entry = entry->next;
        }
    }
}

