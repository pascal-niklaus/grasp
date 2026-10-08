# Grasp

Grasp **GRA**bs the screen and lets you **S**ketch on top of it during
**P**resentations.

Grasp is a Qt6 tool for Linux. It allows you to grab a screenshot (or
part of the screen), then draw on top of it — freehand, lines, arrows,
shapes, text, highlighter — directly over whatever is already on
screen. Useful for live demos, screencasts, or annotating slides on
the fly.

## History

This project started with contributions to
*[gromit-mpx](https://github.com/bk138/gromit-mpx)*. Later, I decided
to implement my own screen annotation tool with a different design
philosophy. 

Essentially, I wanted to have a true screen capture tool that was
unobtrusive to the viewer, creating the illusion of still looking at
the same screen. This way, I could use it in teaching.

Capturing the screen instead of sketching on a transparent overlay (as
in **gromit**) allows to re-visit previously captured screens, to capture
isolated screen sections, and to save the annotated screenshots.

Also, **grasp** has full undo/redo capabilities, and allwos to edit
objects including moving them around, rotating and scaling them.

## Features

- Capture the full screen or a region, on both X11 (native
  `grabWindow`) and Wayland (via PipeWire + the `ScreenCast`
  xdg-desktop-portal)
- Drawing tools: freehand pen (with smoothing), straight/orthogonal
  lines, rectangles, arrows, text, highlighter, and a "fader" pen
  whose strokes fade out over time
- Multiple scenes — switch between several saved sketches, each
  keeping its own drawing and undo history
- Undo/redo
- A floating, translucent toolbar ("dash") for picking tool, color,
  line width and arrow style
- Configurable key bindings and colors (stored via `QSettings`, in an
  `.ini` config file)
- Global keyboard shortcuts — via the `GlobalShortcuts`
  xdg-desktop-portal on Wayland; on X11, bind a hotkey in your window
  manager/DE to the D-Bus commands below
- A D-Bus interface for scripting/remote control
- Single-instance behaviour: a second invocation of `grasp` talks to
  the already-running instance over D-Bus instead of starting a new
  one

## Requirements

- Linux (X11 or Wayland)
- Qt6 (`Core`, `Gui`, `DBus`, `Widgets`)
- CMake ≥ 3.25 and a C++20 compiler
- `libpipewire-0.3` (found via pkg-config)
- On Wayland: an xdg-desktop-portal implementation that supports the
  `ScreenCast` and `GlobalShortcuts` portals

## Building

```sh
cmake -S src -B build
cmake --build build
```

> **Note:** the build currently always compiles with AddressSanitizer,
> UndefinedBehaviorSanitizer and LeakSanitizer enabled (see
> `CMakeLists.txt`). This catches memory/UB bugs early but adds
> runtime overhead — keep that in mind if you package or benchmark the
> binary.

## Installing

```sh
cmake --install build
```

This installs the `grasp` binary, its desktop entry, and icon (to the
standard CMake `GNUInstallDirs` locations).

## Usage

Run `grasp` to start it. It opens a frameless, always-on-top,
translucent window over your screen with the drawing toolbar visible;
grab a screenshot and start sketching.

Useful command-line options (see `grasp --help` for the full list):

| Option | Effect |
|---|---|
| `-g`, `--grab` | take a screenshot and open it as a new drawing |
| `--toggle` | toggle application visibility |
| `--show` / `--hide` | show / hide the application |
| `-q`, `--quit` | quit |
| `--config CONFIG-FILE` | use a specific config file |
| `--verbose LEVEL` | logging verbosity (0 = off, 1 = low, 2 = high, 3 = all) |
| `--help-dbus` | print detailed help about the D-Bus interface |

I simply install global hotkeys that send the `toggle` and `grabnew`
commands via D-Bus (see below). This allows to show/hide **grasp**
with a single keystroke, and capture the current screen and present it
in **grasp**, ready for annotation.


### D-Bus remote control

Grasp exposes a D-Bus service at `org.pascalniklaus.grasp`, object
path `/exec`, which lets you drive it remotely — handy for binding to
global hotkeys on X11, or scripting a presentation.

```
method void org.pascalniklaus.grasp.quit()
method void org.pascalniklaus.grasp.toggle()
method void org.pascalniklaus.grasp.show()
method void org.pascalniklaus.grasp.hide()
method void org.pascalniklaus.grasp.grab()
method void org.pascalniklaus.grasp.grabnew()
method void org.pascalniklaus.grasp.tool(QString tool)
method void org.pascalniklaus.grasp.arrow(QString arrow)
method void org.pascalniklaus.grasp.color(int index)
method void org.pascalniklaus.grasp.width(int width)
method void org.pascalniklaus.grasp.rect(int x0, int y0, int x1, int y1, int r)
```

Invoke them with `qdbus` or `dbus-send`, e.g.:

```sh
qdbus org.pascalniklaus.grasp /exec grabnew
dbus-send --session --dest=org.pascalniklaus.grasp --type=method_call /exec org.pascalniklaus.grasp.toggle
```

## License

GPLv3 or later — see [LICENSE](LICENSE).

## Author

Pascal Niklaus <pascal@niklaus.org> or <pascal.niklaus@uzh.ch>
