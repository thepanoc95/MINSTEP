CC = gcc
KernelDefs = postscript.h types.h globals.h errors.h procs.h except.h
KernelObjects = array.o control.o debug.o dict.o error.o exec.o math.o\
		name.o scanner.o stack.o stream.o string.o type.o vm.o
GraphPakDefs = reducer.h graphics.h gray.h graphicspriv.h
GraphPakObjects = graphpak.o reducer.o matrix.o graphics.o gray.o\
		stroke.o path.o image.o
DeviceDefs = framedevice.h x11device.h
DeviceObjects = device.o nulldevice.o framedevice.o banddevice.o\
		sundev.o versatecdev.o inputdevice.o x11device.o
FontDefs = fonts.h
FontObjects = font.o fontdisk.o fontcache.o fontshow.o fontbuild.o\
		 xylock.o bezier.o curvefit.o
UserObjects = user.o unix.o

X11CFLAGS = $(shell pkg-config --cflags x11 2>/dev/null)
X11LIBS = $(shell pkg-config --libs x11 2>/dev/null)

CFLAGS = -std=gnu89 -O -w -DLINUX $(X11CFLAGS)

PS: $(KernelObjects) $(GraphPakObjects) $(DeviceObjects) $(FontObjects)\
	$(UserObjects)
	$(CC) $(CFLAGS) $(KernelObjects) $(GraphPakObjects) $(DeviceObjects)\
		$(FontObjects) $(UserObjects) -lm $(X11LIBS) -o PS

$(KernelObjects): $(KernelDefs)
$(GraphPakDefs): $(KernelDefs)
$(DeviceDefs): $(KernelDefs) $(GraphPakDefs)
$(GraphPakObjects): $(KernelDefs) $(GraphPakDefs)
$(DeviceObjects): $(KernelDefs) $(GraphPakDefs) $(DeviceDefs)
$(FontObjects): $(FontDefs) $(GraphPakDefs) $(KernelDefs)
$(UserObjects): $(KernelDefs)
