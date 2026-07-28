#
# mxwl/kernel.make
#
# Makefile for building the mxwl kernel and DeviceKit drivers.
#
# Copyright (c) 2026 MinSTEP Project
#

# ========================================================================
# Configuration
# ========================================================================

CC = gcc
CFLAGS = -Wall -g -O2
CFLAGS += -fno-strict-aliasing

# Objective-C support
OBJC = gcc
OBJCFLAGS = $(CFLAGS) -fobjc-exceptions

# Installation
PREFIX ?= /usr/local
DESTDIR ?=

# ========================================================================
# Directories
# ========================================================================

MXWL_ROOT = ..
OBJC_ROOT = $(MXWL_ROOT)/../Developer/objc
APPKIT_ROOT = $(MXWL_ROOT)/../Developer/appkit
FOUNDATION_ROOT = $(MXWL_ROOT)/../../Developer/foundation

# ========================================================================
# Source Files
# ========================================================================

# Mach kernel
MACH_SOURCES = \
	mach/MaxxwellKernel.m \
	mach/MaxxwellPort.m \
	task/MaxxwellTask.m \
	ipc/MaxxwellIPC.m \
	ipc/ipc.c \
	task/task.c \
	mach/mach_port.c \
	mach/mach_kernel.c \
	mach/klog.c \
	loader/mach_loader.c

# DeviceKit
DEVICEKIT_SOURCES = \
	devicekit/DeviceKitPlatform.c \
	devicekit/DeviceKitDriverManager.c \
	devicekit/DeviceX11Framebuffer.c \
	devicekit/DeviceNoFramebuffer.c \
	devicekit/DeviceKit.appkit.m

# Platform family
PLATFORM_SOURCES = \
	devicekit/PlatformFamily/DKit_x86_64.c \
	devicekit/PlatformFamily/DKit486.c

# libkern support library
LIBKERN_SOURCES = \
	libkern/libkern_assert.c \
	libkern/libkern_hash.c \
	libkern/libkern_log.c \
	libkern/libkern_memory.c \
	libkern/libkern_panic.c \
	libkern/libkern_string.c \
	libkern/libkern_tree.c \
	libkern/libkern_util.c

# Server subsystem (MXSI)
SERVER_SOURCES = \
	server/server.c \
	server/registry.c \
	server/message.c \
	server/ipc.c \
	server/loader.c \
	server/manager.c \
	server/bootstrap.c

# Kernel entry
KERNEL_SOURCES = \
	main.c

# Object files
MACH_OBJECTS = $(MACH_SOURCES:.m=.o) $(MACH_SOURCES:.c=.o)
DEVICEKIT_OBJECTS = $(DEVICEKIT_SOURCES:.m=.o) $(DEVICEKIT_SOURCES:.c=.o)
PLATFORM_OBJECTS = $(PLATFORM_SOURCES:.c=.o)
LIBKERN_OBJECTS = $(LIBKERN_SOURCES:.c=.o)
SERVER_OBJECTS = $(SERVER_SOURCES:.c=.o)
KERNEL_OBJECTS = $(KERNEL_SOURCES:.c=.o)

ALL_OBJECTS = $(MACH_OBJECTS) $(DEVICEKIT_OBJECTS) $(PLATFORM_OBJECTS) $(LIBKERN_OBJECTS) $(SERVER_OBJECTS) $(KERNEL_OBJECTS)

# ========================================================================
# Libraries
# ========================================================================

OBJC_RT = $(OBJC_ROOT)/build/libobjcrt.a
FOUNDATION_LIB = $(FOUNDATION_ROOT)/libfoundation.a

# ========================================================================
# Targets
# ========================================================================

.PHONY: all clean kernel devicekit install

all: kernel devicekit

# Build kernel executable
kernel: $(KERNEL_OBJECTS) $(MACH_OBJECTS) $(LIBKERN_OBJECTS) $(SERVER_OBJECTS) $(OBJC_RT)
	$(OBJC) $(OBJCFLAGS) -o vmmxwl $(KERNEL_OBJECTS) $(MACH_OBJECTS) $(LIBKERN_OBJECTS) $(SERVER_OBJECTS) $(OBJC_RT) -ldl -lpthread

# Build DeviceKit library
devicekit: $(DEVICEKIT_OBJECTS) $(PLATFORM_OBJECTS)
	ar rcs libdevicekit.a $(DEVICEKIT_OBJECTS) $(PLATFORM_OBJECTS)

# ========================================================================
# Compilation Rules
# ========================================================================

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ -I$(OBJC_ROOT)/objc -I. -Ilibkern -Iserver

%.o: %.m
	$(OBJC) $(OBJCFLAGS) -c $< -o $@ -I$(OBJC_ROOT)/objc -I$(APPKIT_ROOT) -I$(FOUNDATION_ROOT) -I.

# ========================================================================
# Dependencies
# ========================================================================

devicekit/DeviceKitPlatform.o: devicekit/Headers/DeviceKit.h
devicekit/DeviceKitDriverManager.o: devicekit/Headers/DeviceKit.h
devicekit/DeviceX11Framebuffer.o: devicekit/Headers/DeviceKit.h
devicekit/DeviceNoFramebuffer.o: devicekit/Headers/DeviceKit.h
devicekit/DeviceKit.appkit.o: devicekit/Headers/DeviceKit.h

mach/MaxxwellKernel.o: mach/MaxxwellKernel.h ipc/ipc.h task/task.h loader/mach_loader.h
mach/MaxxwellPort.o: mach/MaxxwellPort.h mach/MachMsg.h
task/MaxxwellTask.o: task/MaxxwellTask.h mach/MachMsg.h

# Server subsystem dependencies
server/server.o: server/server.h mach/mach_types.h task/task.h libkern/libkern.h
server/registry.o: server/registry.h server/server.h mach/klog.h libkern/libkern.h
server/message.o: server/message.h mach/mach_types.h libkern/libkern.h
server/ipc.o: server/ipc.h server/server.h server/message.h server/registry.h \
              mach/mach_port.h mach/klog.h libkern/libkern.h
server/loader.o: server/loader.h server/server.h server/registry.h server/ipc.h \
                 mach/mach_kernel.h mach/mach_port.h loader/mach_loader.h libkern/libkern.h
server/manager.o: server/manager.h server/server.h server/registry.h server/loader.h \
                  server/ipc.h mach/klog.h mach/mach_port.h libkern/libkern.h
server/bootstrap.o: server/bootstrap.h server/manager.h server/registry.h server/loader.h \
                    server/ipc.h mach/klog.h mach/mach_kernel.h libkern/libkern.h

# libkern dependencies
libkern/libkern_assert.o: libkern/libkern_assert.h libkern/libkern_panic.h libkern/libkern_log.h
libkern/libkern_hash.o: libkern/libkern_hash.h libkern/libkern_memory.h
libkern/libkern_log.o: libkern/libkern_log.h
libkern/libkern_memory.o: libkern/libkern_memory.h
libkern/libkern_panic.o: libkern/libkern_panic.h
libkern/libkern_string.o: libkern/libkern_string.h
libkern/libkern_tree.o: libkern/libkern_tree.h
libkern/libkern_util.o: libkern/libkern_util.h

# ========================================================================
# Installation
# ========================================================================

install: all
	install -d $(DESTDIR)$(PREFIX)/bin
	install -d $(DESTDIR)$(PREFIX)/lib
	install -m 755 vmmxwl $(DESTDIR)$(PREFIX)/bin/
	install -m 644 libdevicekit.a $(DESTDIR)$(PREFIX)/lib/
	install -d $(DESTDIR)$(PREFIX)/include/mxwl
	install -m 644 devicekit/Headers/DeviceKit.h $(DESTDIR)$(PREFIX)/include/mxwl/
	install -m 644 server/mxsi.h $(DESTDIR)$(PREFIX)/include/mxwl/
	install -m 644 server/server.h $(DESTDIR)$(PREFIX)/include/mxwl/
	install -m 644 server/registry.h $(DESTDIR)$(PREFIX)/include/mxwl/
	install -m 644 server/manager.h $(DESTDIR)$(PREFIX)/include/mxwl/
	install -m 644 server/message.h $(DESTDIR)$(PREFIX)/include/mxwl/
	install -m 644 server/ipc.h $(DESTDIR)$(PREFIX)/include/mxwl/
	install -m 644 server/bootstrap.h $(DESTDIR)$(PREFIX)/include/mxwl/

# ========================================================================
# Cleanup
# ========================================================================

clean:
	rm -f $(ALL_OBJECTS) vmmxwl libdevicekit.a

# ========================================================================
# Help
# ========================================================================

help:
	@echo "Maxxwell Kernel Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all       - Build kernel, devicekit, and server subsystem"
	@echo "  kernel    - Build vmmxwl executable"
	@echo "  devicekit - Build libdevicekit.a"
	@echo "  servers   - Build server subsystem objects"
	@echo "  install   - Install to $(PREFIX)"
	@echo "  clean     - Remove build artifacts"
	@echo "  help      - Show this help"
	@echo ""
	@echo "Variables:"
	@echo "  PREFIX=$(PREFIX)"
	@echo "  CC=$(CC)"
	@echo "  OBJC=$(OBJC)"