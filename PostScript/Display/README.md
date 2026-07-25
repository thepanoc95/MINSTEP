# MINSTEP Display PostScript

This directory is the user-mode Display PostScript display system for MINSTEP.
It is deliberately split into two layers:

* `DPSContext` is the PostScript imaging context.  It owns an operand stack,
  graphics state, path, and grayscale backing store, and executes a Level 1
  drawing subset.
* `DPSSystem` is the display-system layer.  It manages a root display, windows,
  per-window DPS contexts, expose/key/mouse/close events, simple z ordering, and
  compositing windows onto the selected output device.

The first backends are low-memory and POSIX-friendly: portable ASCII output,
Linux-console ANSI output, and an in-memory framebuffer for future host
framebuffer integration.

## Examples

```sh
make -C PostScript/Display test
./PostScript/Display/dps --size 80x25 examples.ps
./PostScript/Display/dps --demo-system --linux-console --size 80x25
```
