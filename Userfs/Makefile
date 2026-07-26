# Userfs/Makefile — Build init binary for MINSTEP Userfs

CC       = /usr/bin/gcc
OBJCC    = $(CURDIR)/../Developer/objc/objcc
OBJCCRT  = $(CURDIR)/../Developer/objc/build/libobjcrt.a

OBJCC_INCLUDES = -I $(CURDIR)/../Developer/objc

INIT_SRC = init.m
INIT_BIN = init

.PHONY: all clean

all: $(INIT_BIN)

$(OBJCCRT):
	$(MAKE) -C $(CURDIR)/../Developer/objc libobjcrt

$(INIT_BIN): $(INIT_SRC) $(OBJCCRT)
	$(OBJCC) -c -Wall -Wextra -g -O2 $(OBJCC_INCLUDES) -o init.o $(INIT_SRC)
	$(CC) -static -Wall -Wextra -g -O2 -o $@ init.o $(OBJCCRT)

clean:
	rm -f $(INIT_BIN) init.o
