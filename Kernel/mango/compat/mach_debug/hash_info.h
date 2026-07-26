/*
 * compat/mach_debug/hash_info.h
 *
 * Stub for hash_info_bucket_t, used by ipc_hash_info()
 * in debug builds.  Not needed for the data structure porting phase.
 */

#ifndef MANGO_COMPAT_MACH_DEBUG_HASH_INFO_H
#define MANGO_COMPAT_MACH_DEBUG_HASH_INFO_H

typedef struct hash_info_bucket {
    unsigned int hib_count;
} hash_info_bucket_t;

#endif /* MANGO_COMPAT_MACH_DEBUG_HASH_INFO_H */
