# MinSTEP Objective-C Toolchain Makefile
#
# Builds:
#   1. The objc preprocessor (objcpreproc -> objc)
#   2. The Objective-C runtime library (libobjcrt.a)
#   3. The standard library headers
#   4. Installation to prefix
#
# Usage:
#   make              Build objcc and headers
#   make objc         Build the preprocessor
#   make libobjcrt    Build the runtime library
#   make install      Install to PREFIX
#   make clean        Clean build artifacts
#   make test         Run basic tests

# ========================================================================
# Configuration
# ========================================================================

CC       = gcc
CFLAGS   ?= -Wall -Wextra -Werror -Wno-stringop-truncation -Wno-format-truncation -std=gnu99
CFLAGS   += -g -O2
CPPFLAGS ?=
CPPFLAGS += -I$(OBJCDIR)
LDFLAGS  ?=
LDLIBS   ?= -ldl -lpthread

PREFIX   ?= /usr/local
BINDIR   = $(PREFIX)/bin
LIBDIR   = $(PREFIX)/lib
INCDIR   = $(PREFIX)/include
DATADIR  = $(PREFIX)/share

# Directories
OBJCDIR  = .
OBJCRT_SRCDIR = .
OBJCRT_BUILDDIR = build

# ========================================================================
# Source Files
# ========================================================================

PREPROC_SRC  = $(OBJCDIR)/objcpreproc.c
PREPROC_BIN  = objcc

OBJCRT_SRC   = $(OBJCRT_SRCDIR)/objcrt.c
OBJCRT_HDR   = $(OBJCRT_SRCDIR)/objcrt.h
OBJCRT_OBJ   = $(OBJCRT_BUILDDIR)/objcrt.o
OBJCRT_LIB   = $(OBJCRT_BUILDDIR)/libobjcrt.a

STDHDR_SRC   = $(OBJCDIR)/objc/objc.h

# ========================================================================
# Build Targets
# ========================================================================

.PHONY: all clean install uninstall test preproc libobjcrt headers

all: preproc libobjcrt headers

# Preprocessor
preproc: $(PREPROC_BIN)

$(PREPROC_BIN): $(PREPROC_SRC)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Runtime Library
libobjcrt: $(OBJCRT_LIB)

$(OBJCRT_BUILDDIR):
	mkdir -p $(OBJCRT_BUILDDIR)

$(OBJCRT_OBJ): $(OBJCRT_SRC) $(OBJCRT_HDR) | $(OBJCRT_BUILDDIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(OBJCRT_LIB): $(OBJCRT_OBJ)
	ar rcs $@ $<

# Static library for linking (includes both runtime and standard objects)
libobjcrt_static: $(OBJCRT_LIB)
	@echo "Runtime library built: $(OBJCRT_LIB)"

# Headers
headers: $(STDHDR_SRC) $(OBJCRT_HDR)
	@echo "Headers are ready"

# ========================================================================
# Object File for Runtime (for direct linking without archiving)
# ========================================================================

objcrt.o: $(OBJCRT_SRC) $(OBJCRT_HDR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

# ========================================================================
# Installation
# ========================================================================

install: all
	@echo "Installing to $(PREFIX)..."
	install -d $(BINDIR)
	install -d $(LIBDIR)
	install -d $(INCDIR)/objc
	install -d $(LIBDIR)/objc

	# Install preprocessor
	install -m 755 $(PREPROC_BIN) $(BINDIR)/objcc

	# Install runtime library
	install -m 644 $(OBJCRT_LIB) $(LIBDIR)/libobjcrt.a
	install -m 644 $(OBJCRT_OBJ) $(LIBDIR)/libobjcrt.o

	# Install headers
	install -d $(INCDIR)/objc
	@for h in $(OBJCDIR)/objc/*.h; do \
		install -m 644 "$$h" $(INCDIR)/objc/; \
	done

	@echo "Installation complete."
	@echo ""
	@echo "Usage:"
	@echo "  objcc MyClass.m -o MyClass.c    # Preprocess"
	@echo "  cc -c MyClass.c               # Compile to .o"
	@echo "  cc -o myapp MyClass.o -lobjcrt # Link"
	@echo ""

uninstall:
	@echo "Removing from $(PREFIX)..."
	rm -f $(BINDIR)/objcc
	rm -f $(LIBDIR)/libobjcrt.a
	rm -f $(LIBDIR)/libobjcrt.o
	rm -f $(INCDIR)/objc/objcrt.h
	rm -f $(INCDIR)/objc/objc.h
	-rmdir $(INCDIR)/objc 2>/dev/null || true
	@echo "Uninstall complete."

# ========================================================================
# Tests
# ========================================================================

TEST_DIR = tests

test: preproc libobjcrt
	@mkdir -p $(TEST_DIR)
	@echo "=== Running basic tests ==="
	@echo ""
	@echo "--- Test 1: Preprocessor processes @interface ---"
	@echo '@interface Foo : Object { int x; }' > $(TEST_DIR)/test1.m
	@echo '@end' >> $(TEST_DIR)/test1.m
	@echo '@implementation Foo' >> $(TEST_DIR)/test1.m
	@echo '- (int)getX { return self->x; }' >> $(TEST_DIR)/test1.m
	@echo '@end' >> $(TEST_DIR)/test1.m
	./objcc $(TEST_DIR)/test1.m -o $(TEST_DIR)/test1.c
	@test -f $(TEST_DIR)/test1.c && echo "PASS: test1.c generated" || echo "FAIL: test1.c not generated"
	@echo ""
	@echo "--- Test 2: @selector() ---"
	@echo '@interface Bar : Object' > $(TEST_DIR)/test2.m
	@echo '@end' >> $(TEST_DIR)/test2.m
	@echo '@implementation Bar' >> $(TEST_DIR)/test2.m
	@echo '- (void)test { SEL s = @selector(description); }' >> $(TEST_DIR)/test2.m
	@echo '@end' >> $(TEST_DIR)/test2.m
	./objcc $(TEST_DIR)/test2.m -o $(TEST_DIR)/test2.c
	@test -f $(TEST_DIR)/test2.c && echo "PASS: test2.c generated" || echo "FAIL: test2.c not generated"
	@echo ""
	@echo "--- Test 3: @property and @synthesize ---"
	@echo '@interface Baz : Object' > $(TEST_DIR)/test3.m
	@echo '@property (nonatomic, retain) id name;' >> $(TEST_DIR)/test3.m
	@echo '@end' >> $(TEST_DIR)/test3.m
	@echo '@implementation Baz' >> $(TEST_DIR)/test3.m
	@echo '@synthesize name = _name;' >> $(TEST_DIR)/test3.m
	@echo '@end' >> $(TEST_DIR)/test3.m
	./objcc $(TEST_DIR)/test3.m -o $(TEST_DIR)/test3.c
	@test -f $(TEST_DIR)/test3.c && echo "PASS: test3.c generated" || echo "FAIL: test3.c not generated"
	@echo ""
	@echo "--- Test 4: @protocol ---"
	@echo '@protocol MyProto' > $(TEST_DIR)/test4.m
	@echo '- (void)doSomething;' >> $(TEST_DIR)/test4.m
	@echo '@end' >> $(TEST_DIR)/test4.m
	./objcc $(TEST_DIR)/test4.m -o $(TEST_DIR)/test4.c
	@test -f $(TEST_DIR)/test4.c && echo "PASS: test4.c generated" || echo "FAIL: test4.c not generated"
	@echo ""
	@echo "--- Test 5: Runtime compilation ---"
	$(CC) $(CFLAGS) -c -o $(TEST_DIR)/objcrt.o $(OBJCRT_SRC)
	@test -f $(TEST_DIR)/objcrt.o && echo "PASS: runtime compiles" || echo "FAIL: runtime compilation"
	@echo ""
	@echo "=== All tests complete ==="

# ========================================================================
# Cleanup
# ========================================================================

clean:
	rm -f $(PREPROC_BIN)
	rm -rf $(OBJCRT_BUILDDIR)
	rm -f objcrt.o
	rm -rf $(TEST_DIR)

# ========================================================================
# Documentation
# ========================================================================

.PHONY: help info

help:
	@echo "MinSTEP Objective-C Toolchain"
	@echo ""
	@echo "Targets:"
	@echo "  all          - Build preprocessor and runtime library"
	@echo "  objcc        - Build the objc preprocessor only"
	@echo "  libobjcrt    - Build the runtime library only"
	@echo "  install      - Install to $(PREFIX)"
	@echo "  uninstall    - Remove from $(PREFIX)"
	@echo "  test         - Run basic tests"
	@echo "  clean        - Remove build artifacts"
	@echo "  help         - Show this help"
	@echo ""
	@echo "Variables:"
	@echo "  CC=$(CC)"
	@echo "  CFLAGS=$(CFLAGS)"
	@echo "  PREFIX=$(PREFIX)"

info:
	@echo "Source: $(PREPROC_SRC) $(OBJCRT_SRC)"
	@echo "Headers: $(OBJCRT_HDR) $(STDHDR_SRC)"
	@echo "Output: $(PREPROC_BIN) $(OBJCRT_LIB)"
