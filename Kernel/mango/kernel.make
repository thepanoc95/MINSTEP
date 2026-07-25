#
# mango/kernel.make
#
# Makefile for building the mango kernel and DeviceKit drivers.
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

MANGO_ROOT = ..
OBJC_ROOT = $(MANGO_ROOT)/../Developer/objc
APPKIT_ROOT = $(MANGO_ROOT)/../Developer/appkit
FOUNDATION_ROOT = $(MANGO_ROOT)/../../Developer/foundation

# ========================================================================
# Source Files
# ========================================================================

# Mach kernel
MACH_SOURCES = \
	mach/MangoKernel.m \
	mach/MangoPort.m \
	task/MangoTask.m \
	ipc/MangoIPC.m \
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

# Kernel entry
KERNEL_SOURCES = \
	main.c

# Object files
MACH_OBJECTS = $(MACH_SOURCES:.m=.o) $(MACH_SOURCES:.c=.o)
DEVICEKIT_OBJECTS = $(DEVICEKIT_SOURCES:.m=.o) $(DEVICEKIT_SOURCES:.c=.o)
PLATFORM_OBJECTS = $(PLATFORM_SOURCES:.c=.o)
KERNEL_OBJECTS = $(KERNEL_SOURCES:.c=.o)

ALL_OBJECTS = $(MACH_OBJECTS) $(DEVICEKIT_OBJECTS) $(PLATFORM_OBJECTS) $(KERNEL_OBJECTS)

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
kernel: $(KERNEL_OBJECTS) $(MACH_OBJECTS) $(OBJC_RT)
	$(OBJC) $(OBJCFLAGS) -o vmmango $(KERNEL_OBJECTS) $(MACH_OBJECTS) $(OBJC_RT) -ldl -lpthread

# Build DeviceKit library
devicekit: $(DEVICEKIT_OBJECTS) $(PLATFORM_OBJECTS)
	ar rcs libdevicekit.a $(DEVICEKIT_OBJECTS) $(PLATFORM_OBJECTS)

# ========================================================================
# Compilation Rules
# ========================================================================

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ -I$(OBJC_ROOT)/objc

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

mach/MangoKernel.o: mach/MangoKernel.h ipc/ipc.h task/task.h loader/mach_loader.h
mach/MangoPort.o: mach/MangoPort.h mach/MachMsg.h
task/MangoTask.o: task/MangoTask.h mach/MachMsg.h

# ========================================================================
# Installation
# ========================================================================

install: all
	install -d $(DESTDIR)$(PREFIX)/bin
	install -d $(DESTDIR)$(PREFIX)/lib
	install -m 755 vmmango $(DESTDIR)$(PREFIX)/bin/
	install -m 644 libdevicekit.a $(DESTDIR)$(PREFIX)/lib/
	install -d $(DESTDIR)$(PREFIX)/include/mango
	install -m 644 devicekit/Headers/DeviceKit.h $(DESTDIR)$(PREFIX)/include/mango/

# ========================================================================
# Cleanup
# ========================================================================

clean:
	rm -f $(ALL_OBJECTS) vmmango libdevicekit.a

# ========================================================================
# Help
# ========================================================================

help:
	@echo "Mango Kernel Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all       - Build kernel and devicekit"
	@echo "  kernel    - Build vmmango executable"
	@echo "  devicekit - Build libdevicekit.a"
	@echo "  install   - Install to $(PREFIX)"
	@echo "  clean     - Remove build artifacts"
	@echo "  help      - Show this help"
	@echo ""
	@echo "Variables:"
	@echo "  PREFIX=$(PREFIX)"
	@echo "  CC=$(CC)"
	@echo "  OBJC=$(OBJC)"