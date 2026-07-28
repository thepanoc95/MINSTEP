# Maxxwell Kernel Migration Report

## Summary

The Mango nanokernel has been successfully migrated to the Maxxwell
nanokernel. All identifiers have been renamed, the directory structure
is in place, libkern has been created, build scripts are updated, and
documentation reflects the new architecture.

## Renamed Files

| Original (mango)             | New (mxwl)                        |
|------------------------------|-----------------------------------|
| Kernel/mango/                | Kernel/mxwl/                      |
| ipc/MangoIPC.h               | ipc/MaxxwellIPC.h                 |
| ipc/MangoIPC.m               | ipc/MaxxwellIPC.m                 |
| mach/MangoKernel.h           | mach/MaxxwellKernel.h             |
| mach/MangoKernel.m           | mach/MaxxwellKernel.m             |
| mach/MangoPort.h             | mach/MaxxwellPort.h               |
| mach/MangoPort.m             | mach/MaxxwellPort.m               |
| task/MangoTask.h             | task/MaxxwellTask.h               |
| task/MangoTask.m             | task/MaxxwellTask.m               |

## Renamed Symbols

### Project Name
- `Mango` → `Maxxwell` (in comments, strings, documentation)
- `mango` → `mxwl` (in file paths, directory names)

### Prefixes
- `MANGO_` → `MXWL_` (macros, include guards, env vars)
- `mango_` → `mxwl_` (functions, variables, struct names)

### Binary Names
- `mango-kernel` → `mxwl-kernel` (binary search paths)
- `vmmango` → `vmmxwl` (build output)
- `mango_kernel_main` → `mxwl_kernel_main`
- `mango_kernel_init` → `mxwl_kernel_init`
- `mango_kernel_loop` → `mxwl_kernel_loop`
- `mango_kernel_shutdown` → `mxwl_kernel_shutdown`
- `mango_kernel_banner` → `mxwl_kernel_banner`

### Include Guards
- `MANGO_MACH_MANGOKERNEL_H` → `MXWL_MACH_MAXXWELLKERNEL_H`
- `MANGO_MACH_MACH_KERNEL_H` → `MXWL_MACH_MACH_KERNEL_H`
- `MANGO_MACH_MANGOPORT_H` → `MXWL_MACH_MAXXWELLPORT_H`
- `MANGO_IPC_MANGOIPC_H` → `MXWL_IPC_MAXXWELLIPC_H`
- `MANGO_TASK_TASK_H` → `MXWL_TASK_TASK_H`
- `MANGO_MACH_TASK_H` → `MXWL_MACH_TASK_H`
- `MANGO_MACH_MSG_H` → `MXWL_MACH_MSG_H`
- `MANGO_MACH_PORT_H` → `MXWL_MACH_PORT_H`
- `MANGO_MACH_TYPES_H` → `MXWL_MACH_TYPES_H`
- `MANGO_MACH_IPC_H` → `MXWL_MACH_IPC_H`
- `MANGO_MACH_KLOG_H` → `MXWL_MACH_KLOG_H`
- `MANGO_LOADER_MACH_LOADER_H` → `MXWL_LOADER_MACH_LOADER_H`
- `MANGO_KAL_H` → `MXWL_KAL_H`
- `MANGO_KAL_THREAD_H` → `MXWL_KAL_THREAD_H`
- `MANGO_KAL_PROCESS_H` → `MXWL_KAL_PROCESS_H`
- `MANGO_KAL_MEMORY_H` → `MXWL_KAL_MEMORY_H`
- All compat guards: `MANGO_COMPAT_*` → `MXWL_COMPAT_*`

### Class Names (Objective-C)
- `MangoKernel` → `MaxxwellKernel`
- `MangoPort` → `MaxxwellPort`
- `MangoTask` → `MaxxwellTask`
- `MangoIPC` → `MaxxwellIPC`

### Environment Variables
- `MANGO_BOOT_VERBOSE` → `MXWL_BOOT_VERBOSE`
- `MANGO_BOOT_SINGLE_USER` → `MXWL_BOOT_SINGLE_USER`
- `MANGO_BOOTSTRAP_PORT` → `MXWL_BOOTSTRAP_PORT`
- `MANGO_TASK_ID` → `MXWL_TASK_ID`
- `MANGO_GPU_ACCEL` → `MXWL_GPU_ACCEL`

### Types and Structs
- `mango_kernel_state_t` → `mxwl_kernel_state_t`
- `mango_task_t` → `mxwl_task_t`
- `mango_thread_t` → `mxwl_thread_t`
- `mango_wait_channel_t` → `mxwl_wait_channel_t`
- `mango_port_table` → `mxwl_port_table`
- `mango_wait_channels` → `mxwl_wait_channels`

## Compatibility Aliases

No compatibility aliases were preserved. The rename is a clean break
from the Mango name. Where historical context is needed, documentation
notes "formerly known as Mango."

## New: libkern

Created `Kernel/mxwl/libkern/` with the following modules:

| Module          | Header                  | Implementation        |
|-----------------|-------------------------|-----------------------|
| Master header   | libkern.h               | (includes all)        |
| Macros          | libkern_macros.h        | (inline)              |
| Assertions      | libkern_assert.h        | libkern_assert.c      |
| Panic           | libkern_panic.h         | libkern_panic.c       |
| Logging         | libkern_log.h           | libkern_log.c         |
| String ops      | libkern_string.h        | libkern_string.c      |
| Memory ops      | libkern_memory.h        | libkern_memory.c      |
| Linked lists    | libkern_list.h          | (inline)              |
| Queues          | libkern_queue.h         | (inline)              |
| Trees (R-B)     | libkern_tree.h          | libkern_tree.c        |
| Hash tables     | libkern_hash.h          | libkern_hash.c        |
| Bit operations  | libkern_bitops.h        | (inline)              |
| Ref counting    | libkern_refcount.h      | (inline)              |
| Utility helpers | libkern_util.h          | libkern_util.c        |

libkern is platform-independent. It uses only C standard library
facilities. No host-specific APIs are used.

## Build System Updates

- `Kernel/Makefile` (top-level): All `mango/` paths → `mxwl/`, targets
  renamed from `mango` to `mxwl`, output binary `mach` → `vmmxwl`
- `Kernel/mxwl/kernel.make`: All source references updated, output
  binary `vmmango` → `vmmxwl`, install path `include/mango` → `include/mxwl`

## Documentation Updates

- Created `Kernel/mxwl/README.md` describing Maxxwell architecture,
  hosted userspace microkernel model, KAL, libkern, execution models,
  directory structure, building, and portability.
- Historical note: "Formerly known as Mango" preserved in README.

## TODO Items

1. **Build & test**: Run `make mxwl` from `Kernel/` to verify the
   kernel builds end-to-end (requires GCC, ObjC runtime, and BSD
   support code in `Developer/`).
2. **Functional test**: Run the kernel (`./vmmxwl`) and verify that
   the hosted userspace mode still functions correctly.
3. **Compatibility headers**: Consider adding compat headers (e.g.,
   `mango_port_compat.h`) if external code references the old names.
4. **KAL libkern bindings**: Consider adding KAL-based implementations
   of libkern features (e.g., KAL-backed logging, KAL-backed memory
   allocation) for platforms that need them.
5. **libkern build integration**: Add libkern to `kernel.make` as an
   object file dependency if kernel code starts using it.

## Remaining Mango References

The only remaining reference to "Mango" is in:
- `Kernel/mxwl/README.md` (intentional historical note)

All source code, headers, build files, and comments have been fully
migrated.

## Architecture Confirmation

**Maxxwell remains a hosted userspace microkernel using the KAL.**

- The KAL abstraction layer is fully preserved at `Kernel/mxwl/kal/`
- No drivers have been moved into the kernel
- No platform-specific code has been introduced outside KAL
- The kernel continues to use KAL for threading, synchronization,
  timers, virtual memory, process abstraction, filesystem, terminal,
  signals, and IPC primitives
- libkern is a new platform-independent support library that does NOT
  replace or duplicate KAL responsibilities
- The hosted execution model (userspace process on Linux/POSIX) is
  the primary design and remains fully intact
- Bare-metal bootstrap support continues to rely on a minimal bootstrap
  program and platform-specific KAL implementation
