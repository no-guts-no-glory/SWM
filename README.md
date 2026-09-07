# SWM — Simple Window Manager

SWM is a lightweight, keyboard-driven X11 window manager written in C.

Unlike a traditional tiling window manager, SWM uses stacking windows by
default while providing practical shortcuts for snapping, tiling, maximizing,
cascading, and arranging windows. It is designed for users who want quick
manual control without title bars, mouse-driven workflows, or external
configuration languages.

## Features

- Stacking window management by default
- Keyboard-driven operation
- Snap windows to screen edges and corners
- Compass-key shortcuts for quick window placement
- Horizontal and vertical maximization
- Grid and cascade arrangements
- Per-application window placement rules
- One hidden window per desktop
- Multiple desktops
- Minimal external dependencies
- Configuration through a familiar `config.h` file
- Optional `launch-it` application launcher
- Xephyr preview script for safe testing

## Why another window manager?

There are hundreds of window managers available, and many of them are
excellent. Some of my favourites include:

1. DWM
2. Sdorfehs
3. Notion
4. StumpWM

However, traditional tiling window managers can struggle with applications
that work better as floating or stacked windows, such as calculators, media
players, dialogs, and other popup-style applications.

SWM takes a different approach. It is primarily a stacking window manager, but
it provides quick keyboard shortcuts for manually tiling and positioning
windows whenever that is useful. This makes it possible to use stacking
windows for applications that need them while still arranging other windows
in a tiling-style layout.

I also wanted a window manager that was:

- Simple enough to understand and modify
- Written in standard C
- Free from external configuration languages such as Lua, Lisp, or Scheme
- Configurable through a source-level `config.h` file
- Lightweight, with minimal external dependencies

Part of the motivation was also simple curiosity: I wanted to understand how
to write a window manager.

## Status and limitations

SWM targets X11 on GNU/Linux systems and has been tested primarily on Debian.

SWM does not currently implement EWMH. As a result, some applications may
handle fullscreen, maximization, focus, or other window-state changes
unexpectedly.

Known examples include:

- Thunar
- Media players with built-in fullscreen shortcuts

For media players, using SWM's own maximize shortcut instead of the
application's built-in fullscreen shortcut usually provides the desired
result.

For Thunar, removing its configuration directory may resolve the problem:

```sh
rm -rf "$HOME/.config/Thunar"
```

Use that command carefully, since it removes Thunar's user configuration.

Wayland is not supported. SWM is an X11 window manager.

## Building and installing

SWM uses `tcc` by default, although the Makefile can be adjusted to use
`gcc`.

On Debian, install the basic build tools and X11 development packages:

```sh
sudo apt update
sudo apt install build-essential libx11-dev libx11-xcb-dev
```

Then build and install SWM:

```sh
make
make install
```

The original build command also works when a completely fresh rebuild is
required:

```sh
make -B && make install
```

By default, the SWM binary is installed in:

```text
$HOME/bin/swm
```

You can change the installation directory in the Makefile. Make sure that
`$HOME/bin` is included in your `PATH` if you want to run `swm` without using
its full path.

The exact packages required may vary between distributions. If the build
fails because a header or library is missing, install the corresponding X11
development package for your distribution.

## Testing SWM safely with Xephyr

Before using SWM as your main window manager, you can test it inside a nested
X11 session using Xephyr.

The source directory includes a script called `0-preview` for this purpose:

```sh
./0-preview
```

The preview script uses `1-xinitrc` to start several terminals when SWM
launches. These scripts are intended for testing and experimentation only.

Using Xephyr allows you to try SWM without replacing your current window
manager.

## Usage

All SWM functions are activated through keyboard shortcuts that include the
`MOD` key. By default, `MOD` is the Windows/Super key.

The exception is switching between desktops, which uses the function keys
directly:

```text
F1       Switch to desktop 1
F2       Switch to desktop 2
F3       Switch to desktop 3
...
```

Windows are placed in the centre of the screen by default. Window placement
can be overridden on a per-application-class basis by adding rules to
`config.h`.

SWM also supports one hidden window per desktop. This can be useful for
temporarily hiding a terminal or a window containing sensitive information.

## Basic keyboard shortcuts

The following are some of the default shortcuts:

```text
WIN+Return    Start a terminal
WIN+[         Snap the current window to the left side
WIN+]         Snap the current window to the right side
WIN+l         List all windows on all desktops
WIN+k         Cascade windows
WIN+m         Maximize the current window
WIN+i         Hide the current window or unhide the hidden window
```

The exact shortcuts are defined in `config.h`.

## Compass-key shortcuts

SWM uses the `qwe asd zxc` keys as a virtual numeric keypad. This makes it
possible to position and resize windows with very little movement of the
hands, even on keyboards without a numeric keypad.

Examples:

```text
WIN+q         Snap the window to the north-west
WIN+w         Snap the window to the north
WIN+e         Snap the window to the north-east

WIN+a         Snap the window to the west
WIN+s         Centre or arrange the window
WIN+d         Snap the window to the east

WIN+z         Snap the window to the south-west
WIN+x         Snap the window to the south
WIN+c         Snap the window to the south-east
```

These shortcuts can also be used as part of the available positioning and
resizing functions. Refer to `wm-manual.html` and `config.h` for the complete
list.

## Documentation

The repository includes an HTML manual describing the default shortcuts:

```text
wm-manual.html
```

The `config.h` file contains:

- Default keyboard shortcuts
- Available window-management functions
- Desktop-related functions
- Examples of per-application placement rules
- Configuration options

The default shortcuts can also be displayed from a terminal using:

```sh
./print-shortcuts
```

## Per-application window placement

Applications can be assigned a preferred position when they start by adding
rules to `config.h`.

The existing entries provide examples of the required format. Copy an
existing rule and modify it for the application you want to configure.

Window placement is matched using the application's window class. Tools such
as `xprop` can be useful for identifying window properties:

```sh
xprop
```

After running `xprop`, click the relevant window and inspect its output.

After changing `config.h`, rebuild SWM:

```sh
make
```

## Hacking and extending SWM

At its core, SWM is primarily a list manager. It contains relatively little
traditional graphics programming.

Desktops and windows are represented using simple list structures. This keeps
window management logic straightforward and makes it easier to add or modify
layouts.

For example, frame-based layouts could be implemented by adding another level
of lists representing frames. The existing list-management functions provide
the basic building blocks needed for this kind of extension.

The source code was written from scratch, although it was influenced by ideas
from several other window managers. One notable influence is the use of a
`config.h` file for configuration instead of an external configuration
language.

As far as I know, SWM's list-based structure and configuration approach are
distinctive, but the project remains heavily inspired by the broader Unix
window-manager tradition.

## `launch-it`

The repository also includes a small application launcher called `launch-it`.

By default, it is activated with:

```text
WIN+Space
```

The launcher displays a simple dialog. Start typing an application name and
matching applications will be displayed. Use the following keys to navigate:

```text
Tab           Select the next match
Shift+Tab     Select the previous match
Enter         Launch the selected application
```

Only the five best matches are shown by default.

`launch-it` is written in Tcl/Tk and depends on the `stest` utility, which must
be installed separately.

## Runtime dependencies

On one Debian installation, the SWM binary reports the following shared
library dependencies:

```text
linux-vdso.so.1
libX11.so.6
libc.so.6
libxcb.so.1
ld-linux-x86-64.so.2
libXau.so.6
libXdmcp.so.6
libbsd.so.0
libmd.so.0
```

Most of these libraries are normally already installed on a GNU/Linux desktop
system. The development packages required to compile SWM are separate from
the runtime libraries.

## Configuration

Most configuration is done in:

```text
config.h
```

This file is used to configure:

- The modifier key
- Keyboard shortcuts
- Window-management behaviour
- Desktop shortcuts
- Application placement rules
- Available layouts and actions

After making changes to `config.h`, rebuild SWM for them to take effect:

```sh
make
```

## Disclaimer

I have used SWM every day at work and at home for an extended period without
major problems. However, no software is completely free of bugs, and SWM is
no exception.

Because I am familiar with its behaviour, it may work particularly well for
my own workflow. If you decide to use it, test it with Xephyr first and make
sure it works correctly with the applications you rely on before installing
it as your primary window manager.

SWM is provided as-is, without guarantees. Use it at your own risk.
