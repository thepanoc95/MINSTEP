# ========================================================================
#  MinSTEP Kernel / Mango Nanokernel
#  Top-level Makefile
#
#  Builds:
#    1. boot       - Boot prompt / loader
#    2. mango      - The Mango nanokernel (ObjC + C)
#
#  Usage:
#    make              Build everything
#    make boot         Build boot only
#    make mango        Build the nanokernel only
#    make clean        Clean build artifacts
#    make run          Build and run (boot -> mango)
# ========================================================================

CC       = /usr/bin/gcc
CFLAGS   ?= -Wall -Wextra -g -O2
CPPFLAGS ?=
LDFLAGS  ?=

# Include paths -- kernel headers are under mango/
INCLUDES = -I. -Imango -Imango/mach -Imango/ipc -Imango/task -Imango/loader

# ========================================================================
#  ObjC preprocessor and runtime (from Developer/objc/)
# ========================================================================

OBJCC      = $(CURDIR)/../Developer/objc/objcc
OBJCCRT    = $(CURDIR)/../Developer/objc/build/libobjcrt.a
# ObjC include flags (space-separated for objcc -I <dir> syntax)
OBJCC_INCLUDES = -I . -I mango -I mango/mach -I mango/ipc -I mango/task -I mango/loader -I $(CURDIR)/../Developer/objc

# ========================================================================
#  Source files
# ========================================================================

# boot
BOOT_SRC  = boot1.c mach.c
BOOT_BIN  = osboot1

# Mango nanokernel -- pure C sources
MANGO_C_SRC = mango/main.c \
              mach.c \
              mango/mach/klog.c \
              mango/loader/mach_loader.c

# Mango nanokernel -- Objective-C sources (compiled via objcc)
MANGO_M_SRC = mango/mach/MangoKernel.m \
              mango/mach/MangoPort.m \
              mango/task/MangoTask.m \
              mango/ipc/MangoIPC.m

# Object files for the kernel
MANGO_C_OBJ = build/main.o \
              build/mach.o \
              build/klog.o \
              build/mach_loader.o

MANGO_M_OBJ = build/MangoKernel.o \
              build/MangoPort.o \
              build/MangoTask.o \
              build/MangoIPC.o

MANGO_BIN  = mango-kernel

# ========================================================================
#  Build targets
# ========================================================================

.PHONY: all clean boot mango run install help

all: boot mango

# -- boot -----------------------------------------------------------------

boot: $(BOOT_BIN)

BOOT_OBJ  = build/boot1.o build/mach_boot.o

$(BOOT_BIN): $(BOOT_OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

build/boot1.o: boot1.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

build/mach_boot.o: mach.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

# -- mango kernel ---------------------------------------------------------

mango: $(MANGO_BIN)

build:
	mkdir -p build

# Pure C object files
build/mach.o: mach.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

build/main.o: mango/main.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

build/klog.o: mango/mach/klog.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

build/mach_loader.o: mango/loader/mach_loader.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

# Objective-C object files (compiled via objcc preprocessor)
build/MangoKernel.o: mango/mach/MangoKernel.m | build
	$(OBJCC) -c $(CFLAGS) $(OBJCC_INCLUDES) -o $@ $<

build/MangoPort.o: mango/mach/MangoPort.m | build
	$(OBJCC) -c $(CFLAGS) $(OBJCC_INCLUDES) -o $@ $<

build/MangoTask.o: mango/task/MangoTask.m | build
	$(OBJCC) -c $(CFLAGS) $(OBJCC_INCLUDES) -o $@ $<

build/MangoIPC.o: mango/ipc/MangoIPC.m | build
	$(OBJCC) -c $(CFLAGS) $(OBJCC_INCLUDES) -o $@ $<

# Link kernel: ObjC objects + C objects + libobjcrt
$(MANGO_BIN): $(MANGO_M_OBJ) $(MANGO_C_OBJ) $(OBJCCRT)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(MANGO_M_OBJ) $(MANGO_C_OBJ) $(OBJCCRT) -lpthread $(LDLIBS)

# -- run ------------------------------------------------------------------

run: boot
	./osboot1

# -- install --------------------------------------------------------------

install: all
	@echo "Installing to $(DESTDIR)$(PREFIX)/bin..."
	install -d $(DESTDIR)$(PREFIX)/bin 2>/dev/null || true
	install -m 755 $(BOOT_BIN) $(DESTDIR)$(PREFIX)/bin/osboot1
	install -m 755 $(MANGO_BIN) $(DESTDIR)$(PREFIX)/bin/mango-kernel

# -- clean ----------------------------------------------------------------

clean:
	rm -f $(BOOT_BIN) $(MANGO_BIN)
	rm -rf build

# ========================================================================
#  Help
# ========================================================================

help:
	@echo "MinSTEP Kernel / Mango Nanokernel"
	@echo ""
	@echo "Targets:"
	@echo "  all          Build boot and mango kernel"
	@echo "  boot         Build the boot prompt only"
	@echo "  mango        Build the Mango nanokernel only"
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
