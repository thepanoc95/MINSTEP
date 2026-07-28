/*
 * mxwl/kal/kal.h
 *
 * Kernel Abstraction Layer (KAL) -- master header.
 *
 * Include this single header to pull in the entire KAL.
 * All kernel subsystems should include <kal/kal.h> instead
 * of host OS headers directly.
 *
 * The KAL provides a thin, portable abstraction over host OS
 * primitives: process management, IPC, memory, signals,
 * threads, terminal I/O, filesystem, system info, and timing.
 *
 * To port Maxxwell to a new platform:
 *   1. Add platform detection in kal_platform.h
 *   2. Create kal_<platform>.c implementing all KAL APIs
 *   3. Add the new source file to the Makefile
 */

#ifndef MXWL_KAL_H
#define MXWL_KAL_H

#include "kal_platform.h"
#include "kal_process.h"
#include "kal_ipc.h"
#include "kal_memory.h"
#include "kal_signal.h"
#include "kal_thread.h"
#include "kal_terminal.h"
#include "kal_filesys.h"
#include "kal_system.h"
#include "kal_time.h"

#endif /* MXWL_KAL_H */
