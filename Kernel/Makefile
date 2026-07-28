# ========================================================================
#  MinSTEP Kernel / Maxxwell Nanokernel
#  Top-level Makefile
#
#  Builds:
#    1. boot       - Boot prompt / loader
#    2. mxwl       - The Maxxwell nanokernel (ObjC + C)
#
#  Maxxwell was formerly known as Mango.
#
#  Usage:
#    make              Build everything
#    make boot         Build boot only
#    make mxwl         Build the nanokernel only
#    make clean        Clean build artifacts
#    make run          Build and run (boot -> mxwl)
# ========================================================================

CC       = /usr/bin/gcc
CFLAGS   ?= -Wall -Wextra -g -O2
CPPFLAGS ?=
LDFLAGS  ?= -static

# Include paths -- kernel headers are under mxwl/
INCLUDES = -I. -Imxwl -Imxwl/mach -Imxwl/ipc -Imxwl/task -Imxwl/loader -Imxwl/compat -Imxwl/kal -Imxwl/server -Imxwl/monitor -Imxwl/libkern

# ========================================================================
#  ObjC preprocessor and runtime (from Developer/objc/)
# ========================================================================

OBJCC      = objcc
OBJCCRT    = /usr/local/lib/libobjcrt.a
OBJCC_INCLUDES = -I . -I mxwl -I mxwl/mach -I mxwl/ipc -I mxwl/task -I mxwl/loader -I mxwl/kal -I mxwl/server -I mxwl/monitor -I mxwl/libkern

# ========================================================================
#  Source files
# ========================================================================

# boot
BOOT_SRC  = boot1.c mxwl/KernEntry.c
BOOT_BIN  = boot1

# KAL (Kernel Abstraction Layer) -- POSIX backend
KAL_SRC   = mxwl/kal/posix/kal_posix_process.c \
            mxwl/kal/posix/kal_posix_ipc.c \
            mxwl/kal/posix/kal_posix_memory.c \
            mxwl/kal/posix/kal_posix_signal.c \
            mxwl/kal/posix/kal_posix_thread.c \
            mxwl/kal/posix/kal_posix_terminal.c \
            mxwl/kal/posix/kal_posix_filesys.c \
            mxwl/kal/posix/kal_posix_system.c \
            mxwl/kal/posix/kal_posix_time.c

KAL_OBJ   = build/kal_posix_process.o \
            build/kal_posix_ipc.o \
            build/kal_posix_memory.o \
            build/kal_posix_signal.o \
            build/kal_posix_thread.o \
            build/kal_posix_terminal.o \
            build/kal_posix_filesys.o \
            build/kal_posix_system.o \
            build/kal_posix_time.o

# Maxxwell nanokernel -- pure C sources
MXWL_C_SRC = mxwl/main.c \
             mach.c \
             mxwl/mach/klog.c \
             mxwl/mach/mach_port.c \
             mxwl/mach/ipc.c \
             mxwl/mach/task.c \
             mxwl/loader/mach_loader.c \
             mxwl/ipc/ipc_port.c \
             mxwl/ipc/ipc_sched.c \
             mxwl/ipc/ipc_entry.c \
             mxwl/ipc/ipc_hash.c \
             mxwl/ipc/ipc_splay.c \
             mxwl/ipc/ipc_init.c \
             mxwl/ipc/ipc_stubs.c \
             ../Developer/BSD/bsd.c \
             ../Developer/BSD/bsd_ipc.c \
             ../Developer/BSD/bsd_process.c \
             ../Developer/BSD/bsd_fd.c \
             ../Developer/BSD/bsd_signal.c \
             ../Developer/BSD/bsd_memory.c \
             ../Developer/BSD/bsd_syscall.c \
             mxwl/libkern/libkern_string.c \
             mxwl/libkern/libkern_panic.c \
             mxwl/libkern/libkern_assert.c \
             mxwl/libkern/libkern_log.c \
             mxwl/libkern/libkern_tree.c \
             mxwl/libkern/libkern_hash.c \
             mxwl/libkern/libkern_memory.c \
             mxwl/libkern/libkern_util.c \
             mxwl/server/server.c \
             mxwl/server/registry.c \
             mxwl/server/manager.c \
             mxwl/server/bootstrap.c \
             mxwl/server/loader.c \
             mxwl/server/ipc.c \
             mxwl/server/message.c \
             mxwl/monitor/main.c \
             mxwl/monitor/cmdreg.c \
             mxwl/monitor/history.c \
             mxwl/monitor/completion.c \
             mxwl/monitor/commands/help.c \
             mxwl/monitor/commands/boot.c \
             mxwl/monitor/commands/server.c \
             mxwl/monitor/commands/memory.c \
             mxwl/monitor/commands/ports.c \
             mxwl/monitor/commands/tasks.c \
             mxwl/monitor/commands/debug.c \
             mxwl/monitor/commands/fs.c \
             mxwl/monitor/commands/log.c \
             mxwl/monitor/commands/sysinfo.c

# Maxxwell nanokernel -- Objective-C sources (compiled via objcc)
MXWL_M_SRC = mxwl/mach/MaxxwellKernel.m \
             mxwl/mach/MaxxwellPort.m \
             mxwl/task/MaxxwellTask.m \
             mxwl/ipc/MaxxwellIPC.m

# Object files for the kernel
MXWL_C_OBJ = build/main.o \
             build/mach.o \
             build/klog.o \
             build/mach_port.o \
             build/ipc.o \
             build/task.o \
             build/mach_loader.o \
             build/ipc_port.o \
             build/ipc_sched.o \
             build/ipc_entry.o \
             build/ipc_hash.o \
             build/ipc_splay.o \
             build/ipc_init.o \
             build/ipc_stubs.o \
             build/bsd.o \
             build/bsd_ipc.o \
             build/bsd_process.o \
             build/bsd_fd.o \
             build/bsd_signal.o \
             build/bsd_memory.o \
             build/bsd_syscall.o \
             build/libkern_string.o \
             build/libkern_panic.o \
             build/libkern_assert.o \
             build/libkern_log.o \
             build/libkern_tree.o \
             build/libkern_hash.o \
             build/libkern_memory.o \
             build/libkern_util.o \
             build/server.o \
             build/registry.o \
             build/manager.o \
             build/bootstrap.o \
             build/loader_srv.o \
             build/ipc_srv.o \
             build/message.o \
             build/monitor_main.o \
             build/cmdreg.o \
             build/history.o \
             build/completion.o \
             build/cmd_help.o \
             build/cmd_boot.o \
             build/cmd_server.o \
             build/cmd_memory.o \
             build/cmd_ports.o \
             build/cmd_tasks.o \
             build/cmd_debug.o \
             build/cmd_fs.o \
             build/cmd_log.o \
             build/cmd_sysinfo.o

MXWL_M_OBJ = build/MaxxwellKernel.o \
             build/MaxxwellPort.o \
             build/MaxxwellTask.o \
             build/MaxxwellIPC.o

MXWL_BIN  = vmmxwl

# ========================================================================
#  Build targets
# ========================================================================

.PHONY: all clean boot mxwl run install help kal

all: boot mxwl

# -- boot -----------------------------------------------------------------

boot: $(BOOT_BIN)

BOOT_OBJ  = build/boot1.o build/mach_boot.o

$(BOOT_BIN): $(BOOT_OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

build/boot1.o: boot1.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

build/mach_boot.o: mxwl/KernEntry.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

# -- kal ------------------------------------------------------------------

kal: $(KAL_OBJ)

build:
	mkdir -p build

# KAL POSIX backend object files
build/kal_posix_process.o: mxwl/kal/posix/kal_posix_process.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

build/kal_posix_ipc.o: mxwl/kal/posix/kal_posix_ipc.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

build/kal_posix_memory.o: mxwl/kal/posix/kal_posix_memory.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

build/kal_posix_signal.o: mxwl/kal/posix/kal_posix_signal.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

build/kal_posix_thread.o: mxwl/kal/posix/kal_posix_thread.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

build/kal_posix_terminal.o: mxwl/kal/posix/kal_posix_terminal.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

build/kal_posix_filesys.o: mxwl/kal/posix/kal_posix_filesys.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

build/kal_posix_system.o: mxwl/kal/posix/kal_posix_system.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

build/kal_posix_time.o: mxwl/kal/posix/kal_posix_time.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

# -- mxwl kernel ----------------------------------------------------------

mxwl: $(MXWL_BIN)

# Pure C object files
build/mach.o: mxwl/KernEntry.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

# Build the ObjC runtime library from Developer/objc
$(OBJCCRT):
	$(MAKE) -C $(CURDIR)/../Developer/objc libobjcrt

build/main.o: mxwl/main.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

build/klog.o: mxwl/mach/klog.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

build/mach_port.o: mxwl/mach/mach_port.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

build/ipc.o: mxwl/ipc/ipc.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

build/task.o: mxwl/task/task.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

build/mach_loader.o: mxwl/loader/mach_loader.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

build/ipc_port.o: mxwl/ipc/ipc_port.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -Icompat -c -o $@ $<

build/ipc_sched.o: mxwl/ipc/ipc_sched.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -Icompat -c -o $@ $<

build/ipc_entry.o: mxwl/ipc/ipc_entry.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -Icompat -c -o $@ $<

build/ipc_hash.o: mxwl/ipc/ipc_hash.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -Icompat -c -o $@ $<

build/ipc_splay.o: mxwl/ipc/ipc_splay.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -Icompat -c -o $@ $<

build/ipc_init.o: mxwl/ipc/ipc_init.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -Icompat -c -o $@ $<

build/ipc_stubs.o: mxwl/ipc/ipc_stubs.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -Icompat -c -o $@ $<

# BSD subsystem source files
build/bsd.o: ../Developer/BSD/bsd.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -Icompat -c -o $@ $<

build/bsd_ipc.o: ../Developer/BSD/bsd_ipc.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

build/bsd_process.o: ../Developer/BSD/bsd_process.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

build/bsd_fd.o: ../Developer/BSD/bsd_fd.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

build/bsd_signal.o: ../Developer/BSD/bsd_signal.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

build/bsd_memory.o: ../Developer/BSD/bsd_memory.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

build/bsd_syscall.o: ../Developer/BSD/bsd_syscall.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

# -- libkern ---------------------------------------------------------------
build/libkern_string.o: mxwl/libkern/libkern_string.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

build/libkern_panic.o: mxwl/libkern/libkern_panic.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

build/libkern_assert.o: mxwl/libkern/libkern_assert.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

build/libkern_log.o: mxwl/libkern/libkern_log.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

build/libkern_tree.o: mxwl/libkern/libkern_tree.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

build/libkern_hash.o: mxwl/libkern/libkern_hash.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

build/libkern_memory.o: mxwl/libkern/libkern_memory.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

build/libkern_util.o: mxwl/libkern/libkern_util.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

# -- server subsystem ------------------------------------------------------
build/server.o: mxwl/server/server.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

build/registry.o: mxwl/server/registry.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

build/manager.o: mxwl/server/manager.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

build/bootstrap.o: mxwl/server/bootstrap.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

build/loader_srv.o: mxwl/server/loader.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

build/ipc_srv.o: mxwl/server/ipc.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

build/message.o: mxwl/server/message.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

# -- MsWaiter monitor ------------------------------------------------------
build/monitor_main.o: mxwl/monitor/main.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

build/cmdreg.o: mxwl/monitor/cmdreg.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

build/history.o: mxwl/monitor/history.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

build/completion.o: mxwl/monitor/completion.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

build/cmd_help.o: mxwl/monitor/commands/help.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

build/cmd_boot.o: mxwl/monitor/commands/boot.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

build/cmd_server.o: mxwl/monitor/commands/server.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

build/cmd_memory.o: mxwl/monitor/commands/memory.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

build/cmd_ports.o: mxwl/monitor/commands/ports.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

build/cmd_tasks.o: mxwl/monitor/commands/tasks.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

build/cmd_debug.o: mxwl/monitor/commands/debug.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

build/cmd_fs.o: mxwl/monitor/commands/fs.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

build/cmd_log.o: mxwl/monitor/commands/log.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

build/cmd_sysinfo.o: mxwl/monitor/commands/sysinfo.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

# Objective-C object files (compiled via objcc preprocessor)
build/MaxxwellKernel.o: mxwl/mach/MaxxwellKernel.m | build
	$(OBJCC) -c $(CFLAGS) $(OBJCC_INCLUDES) -o $@ $<

build/MaxxwellPort.o: mxwl/mach/MaxxwellPort.m | build
	$(OBJCC) -c $(CFLAGS) $(OBJCC_INCLUDES) -o $@ $<

build/MaxxwellTask.o: mxwl/task/MaxxwellTask.m | build
	$(OBJCC) -c $(CFLAGS) $(OBJCC_INCLUDES) -o $@ $<

build/MaxxwellIPC.o: mxwl/ipc/MaxxwellIPC.m | build
	$(OBJCC) -c $(CFLAGS) $(OBJCC_INCLUDES) -o $@ $<

# Link kernel: ObjC objects + C objects + KAL objects + libobjcrt
$(MXWL_BIN): $(MXWL_M_OBJ) $(MXWL_C_OBJ) $(KAL_OBJ) $(OBJCCRT)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(MXWL_M_OBJ) $(MXWL_C_OBJ) $(KAL_OBJ) $(OBJCCRT) -lpthread $(LDLIBS)

# -- run ------------------------------------------------------------------

run: boot
	./boot1

# -- install --------------------------------------------------------------

install: all
	@echo "Installing to $(DESTDIR)$(PREFIX)/bin..."
	install -d $(DESTDIR)$(PREFIX)/bin 2>/dev/null || true
	install -m 755 $(BOOT_BIN) $(DESTDIR)$(PREFIX)/bin/boot1
	install -m 755 $(MXWL_BIN) $(DESTDIR)$(PREFIX)/bin/vmmxwl

# -- clean ----------------------------------------------------------------

clean:
	rm -f $(BOOT_BIN) $(MXWL_BIN)
	rm -rf build

# ========================================================================
#  Help
# ========================================================================

help:
	@echo "MinSTEP Kernel / Maxxwell Nanokernel (formerly Mango)"
	@echo ""
	@echo "Targets:"
	@echo "  all          Build boot and mxwl kernel"
	@echo "  boot         Build the boot prompt only"
	@echo "  mxwl         Build the Maxxwell nanokernel only"
	@echo "  kal          Build KAL object files only"
	@echo "  run          Build and run boot"
	@echo "  install      Install to $(DESTDIR)$(PREFIX)/bin"
	@echo "  clean        Remove build artifacts"
	@echo "  help         Show this help"
	@echo ""
	@echo "Variables:"
	@echo "  CC=$(CC)"
	@echo "  CFLAGS=$(CFLAGS)"
	@echo "  PREFIX=$(PREFIX)"
	@echo "  OBJCC=$(OBJCC)"
