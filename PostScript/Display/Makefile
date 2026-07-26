# MINSTEP Display PostScript build.

CC = gcc
CFLAGS ?= -Wall -Wextra -Werror -std=gnu99 -g -O2
CPPFLAGS ?=
LDFLAGS ?=
LDLIBS ?= -lm

ROOT := ../..
OBJCC := $(ROOT)/Developer/objc/objcc
BUILD := build
TARGET := dps
SOURCES := DPSContext.m DPSSystem.m dps.m
OBJECTS := $(addprefix $(BUILD)/,$(SOURCES:.m=.o))

.PHONY: all clean test objc-tool

all: $(TARGET)

objc-tool:
	$(MAKE) -C $(ROOT)/Developer/objc objcc

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/%.o: %.m | $(BUILD) objc-tool
	$(CC) $(CPPFLAGS) $(CFLAGS) -x c -I. -c -o $@ $<

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

test: $(TARGET)
	./$(TARGET) --size 20x8 > $(BUILD)/smoke.txt
	@test -s $(BUILD)/smoke.txt

clean:
	rm -rf $(BUILD) $(TARGET)
