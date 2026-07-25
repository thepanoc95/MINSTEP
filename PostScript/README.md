# MINSTEP PostScript

`Adobe/` contains the historical PostScript 1.0 printer-oriented source drop.
`Display/` contains MINSTEP's user-mode Display PostScript display system.
It keeps the Level 1 model of operand stack, current path, graphics state, and
pluggable device output, then layers windows, per-window contexts, events, z
ordering, and compositing on top so it can act as an early MINSTEP window server.

## Building

```sh
make -C Developer/objc
make -C PostScript/Display
```

## Display devices

The initial DPS display-system backends are intentionally small enough for low-memory
systems:

* `DPS_DEVICE_ASCII` renders to portable ASCII art for serial terminals and
  hosts with as little as 1 MB of RAM.
* `DPS_DEVICE_LINUX_CONSOLE` emits ANSI clear/home sequences before the same
  character-cell image, which is suitable for Linux virtual consoles and other
  POSIX terminals.
* `DPS_DEVICE_MEMORY` keeps an 8-bit grayscale framebuffer in memory for future
  MINSTEP framebuffer integration.

The display system manages root displays, windows, per-window PostScript contexts, expose/key/mouse/close events, and software compositing. The interpreter currently supports a practical Level 1 drawing subset:
`moveto`, `lineto`, `rlineto`, `newpath`, `stroke`, `rectfill`, `setgray`,
`erasepage`, and `showpage`.
