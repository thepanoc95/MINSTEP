#include "libkern_hash.h"
#include "libkern_memory.h"

#include <string.h>

int hash_table_init(struct hash_table *ht, size_t nbuckets)
{
    ht->buckets = libkern_calloc(nbuckets, sizeof(struct hash_bucket *));
    if (!ht->buckets)
        return -1;
    ht->nbuckets = nbuckets;
    ht->count = 0;
    return 0;
}

void hash_table_destroy(struct hash_table *ht)
{
    libkern_free(ht->buckets);
    ht->buckets = NULL;
    ht->nbuckets = 0;
    ht->count = 0;
}

void hash_table_insert(struct hash_table *ht, struct hash_bucket *entry, uint32_t hash)
{
    size_t idx = hash % ht->nbuckets;
    entry->hash = hash;
    entry->next = ht->buckets[idx];
    ht->buckets[idx] = entry;
    ht->count++;
}

void hash_table_remove(struct hash_table *ht, struct hash_bucket *entry)
{
    size_t idx = entry->hash % ht->nbuckets;
    struct hash_bucket **prev = &ht->buckets[idx];
    while (*prev) {
        if (*prev == entry) {
            *prev = entry->next;
            ht->count--;
            return;
        }
        prev = &(*prev)->next;
    }
}

struct hash_bucket *hash_table_lookup(const struct hash_table *ht, uint32_t hash)
{
    size_t idx = hash % ht->nbuckets;
    struct hash_bucket *bucket = ht->buckets[idx];
    while (bucket) {
        if (bucket->hash == hash)
            return bucket;
        bucket = bucket->next;
    }
    return NULL;
}

size_t hash_table_count(const struct hash_table *ht)
{
    return ht->count;
}

void hash_table_clear(struct hash_table *ht)
{
    for (size_t i = 0; i < ht->nbuckets; i++)
        ht->buckets[i] = NULL;
    ht->count = 0;
}

uint32_t hash_string(const char *str)
{
    return hash_mem(str, strlen(str));
}

uint32_t hash_mem(const void *data, size_t len)
{
    const unsigned char *p = (const unsigned char *)data;
    uint32_t hash = 5381;
    for (size_t i = 0; i < len; i++)
        hash = ((hash << 5) + hash) + p[i];
    return hash;
}

uint32_t hash_uint32(uint32_t val)
{
    val = ((val >> 16) ^ val) * 0x45d9f3b;
    val = ((val >> 16) ^ val) * 0x45d9f3b;
    return (val >> 16) ^ val;
}

uint32_t hash_uint64(uint64_t val)
{
    return hash_uint32((uint32_t)(val ^ (val >> 32)));
}

uint32_t hash_ptr(const void *ptr)
{
    return hash_uint64((uint64_t)(uintptr_t)ptr);
}
