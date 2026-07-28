#ifndef MXWL_LIBKERN_HASH_H
#define MXWL_LIBKERN_HASH_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct hash_bucket {
    struct hash_bucket *next;
    uint32_t            hash;
};

struct hash_table {
    struct hash_bucket **buckets;
    size_t               nbuckets;
    size_t               count;
};

#define HASH_TABLE_INIT { NULL, 0, 0 }

int   hash_table_init(struct hash_table *ht, size_t nbuckets);
void  hash_table_destroy(struct hash_table *ht);

void  hash_table_insert(struct hash_table *ht, struct hash_bucket *entry, uint32_t hash);
void  hash_table_remove(struct hash_table *ht, struct hash_bucket *entry);

struct hash_bucket *hash_table_lookup(const struct hash_table *ht, uint32_t hash);

size_t hash_table_count(const struct hash_table *ht);
void   hash_table_clear(struct hash_table *ht);

#define hash_table_entry(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))

#define hash_table_for_each(bucket, ht) \
    for (size_t _i = 0; _i < (ht)->nbuckets; _i++) \
        for (bucket = (ht)->buckets[_i]; bucket; bucket = bucket->next)

uint32_t hash_string(const char *str);
uint32_t hash_mem(const void *data, size_t len);
uint32_t hash_uint32(uint32_t val);
uint32_t hash_uint64(uint64_t val);
uint32_t hash_ptr(const void *ptr);

#ifdef __cplusplus
}
#endif

#endif /* MXWL_LIBKERN_HASH_H */
