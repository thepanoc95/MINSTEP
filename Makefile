# Top-level MINSTEP build orchestration.

.PHONY: all clean test objc dps appkit-dps kernel help

all: objc dps

objc:
	$(MAKE) -C Developer/objc

dps: objc
	$(MAKE) -C PostScript/Display

appkit-dps: dps
	$(MAKE) -C Developer/appkit -f OBJCmakefile dps-support

kernel: objc
	$(MAKE) -C Kernel

test: objc
	$(MAKE) -C Developer/objc
	$(MAKE) -C PostScript/Display test
	$(MAKE) -C Developer/appkit -f OBJCmakefile dps-support

clean:
	$(MAKE) -C Developer/appkit -f OBJCmakefile clean
	$(MAKE) -C PostScript/Display clean
	$(MAKE) -C Developer/objc clean
	$(MAKE) -C Kernel clean

help:
	@echo "MINSTEP top-level targets:"
	@echo "  all     Build Objective-C toolchain and Display PostScript"
	@echo "  objc    Build Developer/objc"
	@echo "  dps     Build PostScript/Display"
	@echo "  appkit-dps Build AppKit DPS bridge"
	@echo "  kernel  Build Kernel"
	@echo "  test    Run toolchain and DPS smoke tests"
	@echo "  clean   Remove generated files"
