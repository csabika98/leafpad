# Leafpad on macOS

Leafpad builds and runs natively on macOS (tested on arm64) against GTK+ 3,
which Homebrew builds with the **Quartz** backend — there is no X11 or XQuartz
anywhere in the picture.

## Build

```sh
brew install autoconf automake libtool pkg-config intltool gtk+3 \
             adwaita-icon-theme gtk-mac-integration librsvg

glib-gettextize --force --copy
intltoolize --copy --force --automake
autoreconf -fi
./configure
make
```

`librsvg` is only needed to render the app icon, and `gtk-mac-integration` is
optional — configure detects it and falls back cleanly, or pass
`--disable-mac-integration`.

## Application bundle

```sh
macos/build-app.sh
open Leafpad.app
```

This installs into `Leafpad.app/Contents/Resources`, renders `leafpad.icns`
from the scalable SVG at sizes up to 1024px, writes an `Info.plist` declaring
plain-text document types, and adds a launcher that sets the GTK data paths
(Finder starts apps with a bare environment). You get a Dock icon, a proper
application name, double-click launch and Finder "Open With".

**The bundle is not self-contained.** The binary still links against the GTK+ 3
dylibs in the Homebrew prefix, so it runs on a machine that has them and is not
something to hand to someone else. Making it redistributable would mean copying
the dylibs in and rewriting their install names with `dylibbundler` or
`gtk-mac-bundler`, plus code signing.

## What the macOS integration does

With `gtk-mac-integration` present:

- menus move from inside the window to the system menu bar
- About moves into the application menu; Quit comes from macOS itself
- documents opened from Finder arrive via `NSApplicationOpenFile`, since Finder
  delivers those as an Apple Event rather than on `argv`
- Quit routes through the usual save-changes prompt

Accelerators use GTK's `<Primary>` modifier, which resolves to **Command** on
Quartz and Control everywhere else, so the Linux build is unaffected.

## Known limitations

- **Input methods.** GTK+ 3 seals `GtkTextView`'s IM context and offers no
  getter, so `check_preedit()` in `src/view.c` is inert. Leafpad's Up/Down,
  Return and Tab handling runs even while an IME is composing, which mainly
  affects CJK input.
- **Startup warning.** `gdk_atom_intern: assertion 'atom_name != NULL' failed`
  comes from inside gtk-mac-integration and is harmless.
- **Always on Top** (`Cmd-T`) depends on `gtk_window_set_keep_above()`, which
  the Quartz backend implements only partially.
