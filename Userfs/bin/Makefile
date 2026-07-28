# ========================================================================
#  Userfs/bin/Makefile
#
#  Userland binaries for MINSTEP BSD.
# ========================================================================

CC       = gcc
CFLAGS   ?= -Wall -Wextra -g -O2
LDFLAGS  ?= -static

OBJCC    = /home/MINSTEP/Developer/objc/objcc
OBJCCRT  = /home/MINSTEP/Developer/objc/build/libobjcrt.a

KERNEL_ROOT = /home/MINSTEP/Kernel

OBJCC_INCLUDES = -I . \
           -I $(KERNEL_ROOT)/mango \
           -I $(KERNEL_ROOT)/mango/mach \
           -I $(KERNEL_ROOT)/mango/ipc \
           -I $(KERNEL_ROOT)/mango/task \
           -I $(KERNEL_ROOT)/mango/loader \
           -I $(KERNEL_ROOT)/mango/compat \
           -I $(KERNEL_ROOT)/mango/kal \
           -I /home/MINSTEP/Developer \
           -I /home/MINSTEP/Developer/BSD \
           -I /home/MINSTEP/Developer/objc

BSD_H = /home/MINSTEP/Developer/BSD/bsd.h

.PHONY: all clean

BIN_PROGS = ls cat echo pwd date true false

all: $(BIN_PROGS)

ls: ls.m $(BSD_H) $(OBJCCRT)
	$(OBJCC) -c $(OBJCC_INCLUDES) -o ls.o $<
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ ls.o $(OBJCCRT)

cat: cat.m $(BSD_H) $(OBJCCRT)
	$(OBJCC) -c $(OBJCC_INCLUDES) -o cat.o $<
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ cat.o $(OBJCCRT)

echo: echo.m $(BSD_H) $(OBJCCRT)
	$(OBJCC) -c $(OBJCC_INCLUDES) -o echo.o $<
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ echo.o $(OBJCCRT)

pwd: pwd.m $(BSD_H) $(OBJCCRT)
	$(OBJCC) -c $(OBJCC_INCLUDES) -o pwd.o $<
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ pwd.o $(OBJCCRT)

date: date.m $(BSD_H) $(OBJCCRT)
	$(OBJCC) -c $(OBJCC_INCLUDES) -o date.o $<
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ date.o $(OBJCCRT)

true: true.m $(BSD_H) $(OBJCCRT)
	$(OBJCC) -c $(OBJCC_INCLUDES) -o true.o $<
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ true.o $(OBJCCRT)

false: false.m $(BSD_H) $(OBJCCRT)
	$(OBJCC) -c $(OBJCC_INCLUDES) -o false.o $<
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ false.o $(OBJCCRT)

clean:
	rm -f $(BIN_PROGS) *.o

help:
	@echo "MINSTEP Userfs Binaries"
	@echo ""
	@echo "Targets:"
	@echo "  all     Build all binaries"
	@echo "  clean   Remove build artifacts"