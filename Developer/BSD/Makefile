CC       = /usr/bin/gcc
CFLAGS   ?= -Wall -Wextra -g -O2
LDFLAGS  ?= -static

KERNEL_ROOT = ../../Kernel

INCLUDES = -I . \
           -I $(KERNEL_ROOT)/mango \
           -I $(KERNEL_ROOT)/mango/mach \
           -I $(KERNEL_ROOT)/mango/ipc \
           -I $(KERNEL_ROOT)/mango/task \
           -I $(KERNEL_ROOT)/mango/loader \
           -I $(KERNEL_ROOT)/mango/compat \
           -I $(KERNEL_ROOT)/mango/kal

.PHONY: all clean kickstart

all: kickstart

kickstart: kickstart.c
	$(CC) $(LDFLAGS) $(CFLAGS) $(INCLUDES) -o $@ $<

clean:
	rm -f kickstart

help:
	@echo "MinSTEP BSD Subsystem"
	@echo ""
	@echo "Targets:"
	@echo "  all       Build kickstart"
	@echo "  kickstart Build kickstart binary"
	@echo "  clean     Remove build artifacts"