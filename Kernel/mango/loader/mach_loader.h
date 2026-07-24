/*
 * mango/loader/mach_loader.h
 *
 * .mach binary loader for the Mango nanokernel.
 *
 * A .mach file is a specially-formatted host-native executable
 * that the kernel can load and execute.  The format is:
 *
 *   +---------------------------+
 *   | Mach header (magic, etc.) |   <-- identifies this as a .mach file
 *   +---------------------------+
 *   | Embedded ELF/PIE binary   |   <-- the actual executable
 *   +---------------------------+
 *   | Mach interface table      |   <-- exported/required Mach services
 *   +---------------------------+
 *   | Metadata (name, version)  |   <-- human-readable info
 *   +---------------------------+
 *
 * When the kernel loads a .mach file, it:
 *   1. Reads and validates the Mach header
 *   2. Extracts the embedded ELF binary to a temporary file
 *   3. fork()s and execve()s the temporary file
 *   4. Sets up the Mach IPC bootstrap port for the new task
 *
 * This allows .mach files to be distributed as single files
 * that contain everything needed to run, while still being
 * host-executable (the embedded ELF is a valid standalone binary).
 */

#ifndef MANGO_LOADER_MACH_LOADER_H
#define MANGO_LOADER_MACH_LOADER_H

#include "../mach/mach_types.h"
#include "../task/task.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -----------------------------------------------------------------------
 *  .mach file magic
 * ----------------------------------------------------------------------- */

#define MACH_BINARY_MAGIC        0x4D414E47  /* "MANG" */
#define MACH_BINARY_MAGIC_BE     0x4D414E47
#define MACH_BINARY_MAGIC_LE     0x474E414D  /* "GNAM" (reversed) */

/* -----------------------------------------------------------------------
 *  .mach header version
 * ----------------------------------------------------------------------- */

#define MACH_BINARY_VERSION_1    1

/* -----------------------------------------------------------------------
 *  .mach header
 * ----------------------------------------------------------------------- */

#define MACH_BINARY_NAME_MAX     64
#define MACH_BINARY_VERSION_MAX  32

typedef struct mach_binary_header {
    uint32_t            magic;              /* MACH_BINARY_MAGIC            */
    uint32_t            version;            /* Format version               */
    uint32_t            header_size;        /* Size of this header          */
    uint32_t            binary_offset;      /* Offset to embedded binary    */
    uint32_t            binary_size;        /* Size of embedded binary      */
    uint32_t            interface_offset;   /* Offset to interface table    */
    uint32_t            interface_size;     /* Size of interface table      */
    uint32_t            metadata_offset;    /* Offset to metadata           */
    uint32_t            metadata_size;      /* Size of metadata             */
    uint32_t            flags;              /* Feature flags                */
    uint32_t            reserved[4];        /* Reserved for future use      */
} mach_binary_header_t;

/* -----------------------------------------------------------------------
 *  .mach interface table entry
 *
 *  Each entry describes one Mach service that the binary either
 *  exports (provides) or requires (imports).
 * ----------------------------------------------------------------------- */

#define MACH_IFACE_EXPORT   0x01
#define MACH_IFACE_IMPORT   0x02

#define MACH_IFACE_NAME_MAX 64

typedef struct mach_interface_entry {
    uint32_t            flags;              /* EXPORT or IMPORT             */
    char                name[MACH_IFACE_NAME_MAX]; /* Service name          */
    uint32_t            version;            /* Interface version            */
    uint32_t            pad;
} mach_interface_entry_t;

typedef struct mach_interface_table {
    uint32_t            count;              /* Number of entries            */
    mach_interface_entry_t entries[1];      /* Variable-length array        */
} mach_interface_table_t;

/* -----------------------------------------------------------------------
 *  .mach metadata
 * ----------------------------------------------------------------------- */

typedef struct mach_metadata {
    char                app_name[MACH_BINARY_NAME_MAX];
    char                version[MACH_BINARY_VERSION_MAX];
    char                description[256];
    uint32_t            uid;
    uint32_t            gid;
    uint32_t            pad[2];
} mach_metadata_t;

/* -----------------------------------------------------------------------
 *  Loader API
 * ----------------------------------------------------------------------- */

/* Initialize the loader subsystem */
kern_return_t mango_loader_init(void);

/* Load a .mach file and create a task for it */
kern_return_t mango_loader_load(const char *mach_path,
                                mango_task_t **out_task);

/* Validate a .mach file header */
kern_return_t mango_loader_validate(const char *mach_path,
                                    mach_binary_header_t *out_header);

/* Extract the embedded binary to a temporary file and exec it */
kern_return_t mango_loader_exec(mango_task_t *task,
                                const char *mach_path,
                                const mach_binary_header_t *header);

/* Create a temporary file with the embedded binary */
kern_return_t mango_loader_extract_binary(const char *mach_path,
                                          const mach_binary_header_t *header,
                                          char *out_tmp_path,
                                          size_t tmp_path_size);

/* Clean up temporary files for a terminated task */
kern_return_t mango_loader_cleanup(mango_task_t *task);

#ifdef __cplusplus
}
#endif

#endif /* MANGO_LOADER_MACH_LOADER_H */
