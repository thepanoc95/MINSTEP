# Maxxwell Nanokernel

Formerly known as **Mango**.

## Architecture

Maxxwell is a **hosted userspace microkernel**. It runs as a userspace
process on top of a host operating system (typically Linux or any POSIX
platform) rather than directly on bare metal. This design enables rapid
development, debugging with standard tools, and portability across host
platforms.

### Execution Models

**Primary: Hosted userspace microkernel**
The kernel runs as an ordinary userspace program. It uses the Kernel
Abstraction Layer (KAL) to interface with the host OS for threading,
synchronization, timers, virtual memory, processes, and other services.

**Secondary: Bare-metal bootstrap**
The kernel can be booted directly on hardware via a minimal bootstrap
program. In this mode, a platform-specific KAL implementation provides
the low-level primitives normally supplied by the host OS.

## Kernel Abstraction Layer (KAL)

The KAL is the portability layer that insulates the kernel from
host-specific APIs. It provides abstractions for:

- Threading and synchronization
- Timers and clocks
- Virtual memory interfaces
- Process management
- Filesystem access
- Terminal I/O
- Signal handling
- System information
- IPC primitives

To port Maxxwell to a new platform, implement the KAL backend for that
platform. The kernel itself remains platform-independent.

## libkern

`libkern/` is the platform-independent kernel support library. It provides:

- String utilities (memcpy, memset, strlen, etc.)
- Memory allocation (malloc, calloc, realloc, free)
- Panic and assertion handling
- Logging with severity levels
- Linked lists (doubly-linked, with iteration macros)
- FIFO queues
- Red-black trees
- Hash tables with common hash functions
- Bit operations (set, clear, test, rotate, population count)
- Reference counting (plain and atomic)
- Utility helpers (character classification, byte swapping, hex dump)

libkern has no host-specific dependencies. If platform functionality is
needed, it uses the KAL.

## Directory Structure

```
Kernel/mxwl/
├── libkern/          Platform-independent support library
├── compat/           Mach 4 compatibility shims
├── devicekit/        DeviceKit driver framework
├── ipc/              IPC subsystem (Mach message passing over sockets)
├── kal/              Kernel Abstraction Layer
│   ├── posix/        POSIX backend (Linux, etc.)
│   └── mac68k/       Macintosh 68k backend
├── loader/           .mach binary loader
├── mach/             Mach kernel core (ports, tasks, messages, klog)
├── task/             Task and thread management
├── main.c            Standalone kernel entry point
├── KernEntry.c       Boot1 entry point (jump2mach)
├── kernel.make       Kernel build script
└── Makefile          Top-level build
```

## Building

```sh
make mxwl          # Build the Maxxwell nanokernel
make kernel        # Build using kernel.make
```

The kernel binary is named `vmmxwl`.

## Portability

Maxxwell is designed to run on any platform with a KAL implementation.
Currently supported:

- **POSIX** (Linux, macOS, BSD) -- primary development target
- **mac68k** -- Macintosh 68k (partial, memory only)

To add a new platform, create `kal/<platform>/` with implementations of
the KAL interfaces defined in `kal/kal_*.h`.

## License

Copyright (c) 2026 MinSTEP Project. BSD License.
