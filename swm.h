// --------------------------------------------------------------------------
// --  filename: swm.h
// --   purpose: define structures and prototypes for swm
// --   created: 12.05.2025
// --    author: fkish@1slash1.com
// -- tab width: 2
// --------------------------------------------------------------------------
#ifndef __SWM_H__
#define __SWM_H__

// --------------------------------------------------------------------------
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/Xatom.h>
#include <X11/Xos.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

// --------------------------------------------------------------------------
enum {
  DIRECTION_NW = 1,
  DIRECTION_N,
  DIRECTION_NE,
  DIRECTION_W,
  DIRECTION_C,
  DIRECTION_E,
  DIRECTION_SW,
  DIRECTION_S,
  DIRECTION_SE
};

// --------------------------------------------------------------------------
typedef struct {
  int   i;
  void* ptr;
  int   x;
  int   y;
  int   width;
  int   height;
} Args;

// --------------------------------------------------------------------------
typedef struct {
  int modkey;
  int key;
  void (*callback)(Args);
  Args args;
} Shortcut;

// --------------------------------------------------------------------------
typedef struct {
  char* name;
  void (*callback)(Args);
  Args  args;
} Application;

// --------------------------------------------------------------------------
char* strcasestr(const char* haystack, const char* needle);

// --------------------------------------------------------------------------
#include "config.h"
#include "llist.h"

#endif

