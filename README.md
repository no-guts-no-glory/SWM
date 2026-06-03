# SWM - Simple Window Manager

## What is it?
I'm a long time fan of simple, tiling WMs, however, they all struggle with applications which really need to be "stacked", like calculators or media players, e.t.c.

SWM is a "hybrid" in that it is a stacking window manager which allows you to quickly and easily tile windows in a very practical way.
There are shortcuts to snap windows to almost any part of the screen using "compass" directions without over-lapping each other, e.g. North, North-West e.t.c with simple key combinations that make sense - to me, anyway. It can also maximise windows horizontally or vertically, snap windows to the left or right side of the screen, arrange them in a grid thus mimicking tiling WMs - all with simple shortcuts. However, the you can pre-configure where certain app will be placed when they are started by configuring them in the config.h file - really handy.

It only draws simple borders around windows without wasting space on title-bars and buttons - all function are ***keboard driven***. I hardly ever use my mouse - with SWM it is not necessary though can be used for selecting/moving/resizing windows.

It is written in standard C with minimal external dependencies and should compile on any GNU-Linux based operating systems, though I have only access to Debian.

Give it a try, I'm sure you won't be disappointed. One warning though - it does not support EWMH and thus some programs will behave in unexpected ways. The notable ones that I have tried are Thunar and any media players that have a built-in fullscreen shortcut (most of them). However, all you need to do is use SWM build-in maximise shortcut with those players instead of the one built-in the apps and you'll get the same result without any drama.

## Why another WM?
There are, literally, hundreds of WMs and I have tried a **lot** of them, 50 or more. Most of them are great and I used them in the past in my daily work. Just a few examples of my favourites (in order of preference): the classic DWM, Sdorfehs, Notion, StumpWM. They are all great WM and are, on the technical level, probably better than SWM, they still struggle with "popup" type windows. However, I wanted something simple, with no dependencies, specially no external languages like LUA or LISP e.t.c. and something I can understand and change any time. There was also the curiosity of how to actually code one.

## Hacking it
At the heart of SWM is basically a list-manager, surprisingly with very little graphics programming. Most window managers use global variables for managing windows and I found them messy and hard to understand. With SWM desktops and windows are just simple list structures that are easily managed and allow for any layouts you desire. If you wanted frame-based window layouts you can easily add another level by creating lists of frames and arrange them any way you like, all the functions already exist in SWM and hopefully the code is clear enough for you to understand and modify if you want something different.

Though the code has been written from scratch, I did get some inspiration from the above mentioned WMs. I notable case is the use of the "config.h" file in place of external languages for configuring

## How start
There is a HTML file describing the default shortcuts, wm-manual.html. Have a look at it to get started then look at the config.h file. It gives you the default keyboard shortcuts and the functions available to deal with windows (called clients) and desktops.

Compiling it is a simple matter of running
~~~
make -B && make install
~~~

It should compile in no time at all. By default the Makefile is configured to use the tcc compiler which generates small, fast code. You can either install it on your system (recommended) or use gcc. You will need some X11 development libraries as the only dependency if not already installed on your system - any make error will tell you what you need to install.

On my Debian installation running `ldd ~/bin/swm` gives me the following dependency list:

- linux-vdso.so.1
- libX11.so.6
- libc.so.6
- libxcb.so.1
- ld-linux-x86-64.so.2
- libXau.so.6
- libXdmcp.so.6
- libbsd.so.0
- libmd.so.0

**Don't be scared of this list** as most of it will have already been installed on your system. Most likely ones missing will be libX11 and libxcb. Those can be install easily with apt-get:

~~~
sudo update
sudo apt-get install libx11-dev libx11-xcb-dev
~~~

By default the SWM binary (swm) will be installed to user $HOME/bin/ folder, you can change that in the Makefile **after** testing it first.

There is a handy little script 0-preview in the source folder which you can
use to try SWM in you current window manager using the Xephyr X11 server. 0-preview in turn uses 1-xinitrc to start some terminals when SWM starts up - just to give you some ideas on how to create startup applications. All this is for testing/experimenting only.

## Miscellany
I've included the source code for a handy little utility called launch-it.

It's a simple application launcher which is activated by pressing WIN+space, by default. It'll show a simple dialog where you can start typing the app name and it'll show matches. You can then use the TAB (or Shift+TAB) key to navigate through them - only the best 5 matches are shown by default. Pressing the Enter/Return key will launch the selected application. It is written in Tcl/Tk, a much under-rated, yet well-thought-out, programming language. Launch-it depends on the stest utility that you need to install first.


