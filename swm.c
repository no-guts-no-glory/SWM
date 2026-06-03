// --------------------------------------------------------------------------
// --  filename: swm.c
// --   purpose: implement a simple X11 window manager using Xlib
// --   created: 10.05.2025
// --    author: fkish@1slash1.com
// -- tab width: 2
// --------------------------------------------------------------------------
#include "swm.h"

// --------------------------------------------------------------------------
#define MINIMIZED_HEIGHT 20
#define BORDER_2_WIDTH   (2*BORDER_WIDTH)

#define WINLIST_FNAME    "/tmp/winlist.swm"

// --------------------------------------------------------------------------
static void              spawn                        (Args   args);
static void              quit                         (Args   args);
static void              reboot                       (Args   args);
static void              shutdown                     (Args   args);

static void              client_select                (Node*  n);
static void              client_unselect              (Node*  n);

static void              client_move_to               (Args   args);
static void              client_center                (Args   args);
static void              client_kill                  (Args   args);
static void              client_next                  (Args   args);
static void              client_nth                   (Args   args);
static void              client_renumber              (Args   args);
static void              client_place                 (Args   args);
static void              client_place_resize          (Args   args);
static void              client_prev                  (Args   args);
static void              client_resize                (Args   args);
static void              client_shift                 (Args   args);
static void              clients_cascade              (Args   args);

static void              client_maximize              (Args   args);
static void              client_minimize              (Args   args);
static void              client_maximize_vertically   (Args   args);
static void              client_maximize_horizontally (Args   args);
static void              client_snap_to               (Args   args);
static void              client_to_desktop            (Args   args);
static void              client_hide_cur              (Args   args);
static void              clients_list                 (Args   args);

static void              desktop_select               (Args   args);
static void              desktop_next                 (Args   args);
static void              desktop_prev                 (Args   args);

static void              handle_buttonpress           (XEvent *ev);
static void              handle_buttonrelease         (XEvent *ev);
static void              handle_keypress              (XEvent *ev);
static void              handle_mapnotify             (XEvent *ev);
static void              handle_motionnotify          (XEvent *ev);
static void              handle_unmapnotify           (XEvent *ev);
static void              handle_configurenotify       (XEvent *ev);

static int               window_ignore                (Window win);
static void              window_resize                (Node*  n, int x, int y, int w, int h);

// --------------------------------------------------------------------------
static int               g_dx;
static int               g_dy;
static XWindowAttributes g_attr;
static XButtonEvent      g_bevent;
static int               g_dir_motion;

static Window            g_root;
static Display*          g_display;
static int               g_screen;
static int               g_screen_w;
static int               g_screen_h;
static ulong             g_clr_selected;
static ulong             g_clr_unselected;

static List*             g_clients;
static List*             g_desktops;

// --------------------------------------------------------------------------
static void (*g_event_handlers[LASTEvent])(XEvent*) = {
// --------------------------------------------------------------------------
  [KeyPress       ] = handle_keypress,
  [ButtonPress    ] = handle_buttonpress,
  [ButtonRelease  ] = handle_buttonrelease,
  [MotionNotify   ] = handle_motionnotify,
  [MapNotify      ] = handle_mapnotify,
  [UnmapNotify    ] = handle_unmapnotify,
  [ConfigureNotify] = handle_configurenotify,
};

// --------------------------------------------------------------------------
static Shortcut g_shortcuts[] = {
// --------------------------------------------------------------------------
  SHORTCUTS
  {0}
};

// --------------------------------------------------------------------------
static Application g_applications[] = {
// --------------------------------------------------------------------------
  APPLICATIONS
  {0}
};

#ifdef WM_SHOW_EVENTS
  // ------------------------------------------------------------------------
  static char* event_name(int etype) {
  // ------------------------------------------------------------------------
    static char *type[LASTEvent] = {
      NULL,
      NULL,
      "KeyPress",
      "KeyRelease",
      "ButtonPress",
      "ButtonRelease",
      "MotionNotify",
      "EnterNotify",
      "LeaveNotify",
      "FocusIn",
      "FocusOut",
      "KeymapNotify",
      "Expose",
      "GraphicsExpose",
      "NoExpose",
      "VisibilityNotify",
      "CreateNotify",
      "DestroyNotify",
      "UnmapNotify",
      "MapNotify",
      "MapRequest",
      "ReparentNotify",
      "ConfigureNotify",
      "ConfigureRequest",
      "GravityNotify",
      "ResizeRequest",
      "CirculateNotify",
      "CirculateRequest",
      "PropertyNotify",
      "SelectionClear",
      "SelectionRequest",
      "SelectionNotify",
      "ColormapNotify",
      "ClientMessage",
      "MapNotify",
      "GenericEvent",
    };
    printf("ev: %s\n", type[etype]? type[etype]: "Unknown event type");
  }
#endif

// --------------------------------------------------------------------------
static ulong getcolor(const char* color) {
// --------------------------------------------------------------------------
  XColor c;
  Colormap map = DefaultColormap(g_display, g_screen);

  if (!XAllocNamedColor(g_display, map, color, &c, &c)) {
    bail("unknown color '%s'", color);
  }
  return c.pixel;
}

// --------------------------------------------------------------------------
static char* window_name(Window win, char* name, int size) {
// --------------------------------------------------------------------------
  char* res = NULL;
  Atom actual_type;
  int actual_format;
  unsigned char *prop = NULL;
  unsigned long nitems, bytes_after;
  Atom wm_name = XInternAtom(g_display, "WM_NAME", False);

  if (XGetWindowProperty(g_display, win, wm_name, 0, MAX_TITLE_LEN, False, AnyPropertyType,
      &actual_type, &actual_format, &nitems, &bytes_after, &prop) == Success) {
    if (nitems > 0) {
      strncpy(res = name, (char *)prop, size);
    }
    XFree(prop);
  }
  return res;
}

// --------------------------------------------------------------------------
static Node* cur_client(void) {
// --------------------------------------------------------------------------
 return list_cur(g_clients);
}

// --------------------------------------------------------------------------
static void wind_down(void) {
// --------------------------------------------------------------------------
  list_free(g_desktops);

  XSync(g_display, False);
  XSetInputFocus(g_display, g_root, RevertToPointerRoot, CurrentTime);
  XCloseDisplay(g_display);
}

// --------------------------------------------------------------------------
static void quit(Args args) {
// --------------------------------------------------------------------------
	wind_down();
  exit(args.i);
}

// --------------------------------------------------------------------------
static void shutdown(Args args) {
// --------------------------------------------------------------------------
	wind_down();
	setuid(0);
	system("sudo shutdown -P now");
}

// --------------------------------------------------------------------------
static void reboot(Args args) {
// --------------------------------------------------------------------------
	wind_down();
	setuid(0);
	system("sudo reboot");
}

// --------------------------------------------------------------------------
static void spawn(Args args) {
// --------------------------------------------------------------------------
  if (fork() == 0) {
    if (g_display) {
      close(ConnectionNumber(g_display));
    }
    setsid();
    char** arg = args.ptr;
    if (execvp(arg[0], arg) == -1) {
      fprintf(stderr, "spawn '%s' failed", arg[0]);
    }
  }
}

// --------------------------------------------------------------------------
static void client_select(Node* n) {
// --------------------------------------------------------------------------
  if (!n) {return;}

  Window win = n->data;
  list_apply(g_clients, client_unselect);

  XSetWindowBorderWidth(g_display, win, BORDER_WIDTH);
  XSetWindowBorder     (g_display, win, g_clr_selected);
  XSetInputFocus       (g_display, win, RevertToPointerRoot, CurrentTime);
  XRaiseWindow         (g_display, win);

  if (n->state == STATE_NEW) {
    XWindowAttributes attr;
    XGetWindowAttributes(g_display, win, &attr);
    n->x = attr.x;
    n->y = attr.y;
    n->w = attr.width;
    n->h = attr.height;
    n->state = STATE_NORMAL;
  }
  g_clients->cur = n;
}

// --------------------------------------------------------------------------
static void client_draw(Node* n) {
// --------------------------------------------------------------------------
  if (n) {
    Window win = n->data;
    if (win) {
      int x = n->x, y = n->y, w = n->w, h = n->h;
      if (n->state == STATE_MINIMIZED) {
        h = MINIMIZED_HEIGHT;
      } else if (n->state == STATE_MAXIMIZED) {
        x = -BORDER_WIDTH;
        y = -BORDER_WIDTH;
        w = g_screen_w+BORDER_2_WIDTH;
        h = g_screen_h+BORDER_2_WIDTH;
      } else {
        if (w < 0 || w > (g_screen_w - BORDER_2_WIDTH)) {
          w = g_screen_w - BORDER_2_WIDTH;
        }
        if (h < 0 || h > (g_screen_h - BORDER_2_WIDTH)) {
          h = g_screen_h - BORDER_2_WIDTH;
        }
      }
      XMoveResizeWindow(g_display, win, x, y, w, h);
      XFlush(g_display);
    }
  }
}

// --------------------------------------------------------------------------
static void window_resize(Node* n, int x, int y, int w, int h) {
// --------------------------------------------------------------------------
  if (!n) {return;}

  if (n->state != STATE_MAXIMIZED) {
    n->x = x;
    n->y = y;
    n->w = w;
    if (n->state == STATE_NORMAL) {
      n->h = h;
    }
  }
  XMoveResizeWindow(g_display, n->data, x, y, w, h);
  XFlush(g_display);
}

// --------------------------------------------------------------------------
static void client_maximize(Args args) {
// --------------------------------------------------------------------------
  Node* n = cur_client();
  if (!n) {return;}

  if (n->state == STATE_MAXIMIZED) {
    n->state = STATE_NORMAL;
  } else {
    n->state = STATE_MAXIMIZED;
  }
  client_draw(n);
}

// --------------------------------------------------------------------------
static void client_minimize(Args args) {
// --------------------------------------------------------------------------
  Node* n = cur_client();
  if (!n) {return;}

  if (n->state == STATE_MINIMIZED) {
    n->state = STATE_NORMAL;
  } else {
    n->state = STATE_MINIMIZED;
  }
  client_draw(n);
}

// --------------------------------------------------------------------------
static void client_maximize_vertically(Args args) {
// --------------------------------------------------------------------------
  Node* n = cur_client();
  if (!n) {return;}

  int y = 0;
  int x = n->x, w = n->w;
  int h = g_screen_h - BORDER_2_WIDTH;
  n->state = STATE_NORMAL;
  window_resize(n, x, y, w, h);
}

// --------------------------------------------------------------------------
static void client_maximize_horizontally(Args args) {
// --------------------------------------------------------------------------
  Node* n = cur_client();
  if (!n) {return;}

  int x = 0, y = n->y, w = g_screen_w - BORDER_2_WIDTH, h = n->h;
  n->state = STATE_NORMAL;
  window_resize(n, x, y, w, h);
}

// --------------------------------------------------------------------------
static void client_snap_to(Args args) {
// --------------------------------------------------------------------------
  Node* n = cur_client();
  if (!n) {return;}

  int x = 0, y = 0, w, h;
  switch (args.i) {
    case DIRECTION_W:
      w = g_screen_w/2 - BORDER_2_WIDTH;
      h = g_screen_h   - BORDER_2_WIDTH;
      break;
    case DIRECTION_E:
      x  = g_screen_w/2;
      w  = g_screen_w/2 - BORDER_2_WIDTH;
      h  = g_screen_h   - BORDER_2_WIDTH;
      break;
    case DIRECTION_N:
      w = g_screen_w   - BORDER_2_WIDTH;
      h = g_screen_h/2 - BORDER_2_WIDTH;
      break;
    case DIRECTION_S:
      y = g_screen_h/2;
      w = g_screen_w   - BORDER_2_WIDTH;
      h = g_screen_h/2 - BORDER_2_WIDTH;
      break;
  }
  if (args.width) {
    w = args.width > 0? args.width: g_screen_w - BORDER_2_WIDTH;
  }
  if (args.height) {
    h = args.height > 0? args.height: g_screen_h - BORDER_2_WIDTH;
  }
  n->state = STATE_NORMAL;
  window_resize(n, x, y, w, h);
}

// --------------------------------------------------------------------------
static void client_unselect(Node* n) {
// --------------------------------------------------------------------------
  if (n) {
    Window win = n->data;
    if (win) {
      XSetWindowBorder(g_display, win, g_clr_unselected);
      XSetInputFocus(g_display, win, RevertToPointerRoot, CurrentTime);
    }
  }
}

// --------------------------------------------------------------------------
static void client_hide(Node* n) {
// --------------------------------------------------------------------------
  if (n && n->data) {
    // XMoveResizeWindow(g_display, n->data, g_screen_w+1, 0, n->w, n->h);
    XMoveWindow(g_display, n->data, g_screen_w+1, 0);
  }
}

// --------------------------------------------------------------------------
static void client_unhide(Node* n) {
// --------------------------------------------------------------------------
  if (n && n->data) {
    if (n->state != STATE_HIDDEN) {
      // XMoveResizeWindow(g_display, n->data, n->x, n->y, n->w, n->h);
      if (n->state == STATE_MAXIMIZED) {
        XMoveWindow(g_display, n->data, -BORDER_WIDTH, -BORDER_WIDTH);
      } else {
        XMoveWindow(g_display, n->data, n->x, n->y);
      }
    }
  }
}

// --------------------------------------------------------------------------
static void client_hide_cur(Args arg) {
// --------------------------------------------------------------------------
  // -- only ONE client per desktop can be hidden
  Node* n;
  Node* dt = list_cur(g_desktops);
  if ((n = dt->hidden) != NULL) {
    n->state = n->state_prev;
    client_draw(n);
    client_select(n);
    dt->hidden = NULL;
  } else {
    n = cur_client();
    if (n && n->data) {
      n->state_prev = n->state;
      (dt->hidden = n)->state = STATE_HIDDEN;
      XMoveResizeWindow(g_display, n->data, g_screen_w+1, 0, n->w, n->h);
      Args args;
      client_next(args);
    }
  }
}

// --------------------------------------------------------------------------
static void client_kill(Args args) {
// --------------------------------------------------------------------------
  Node *n = cur_client();
  if (!n) {return;}

  Window win = n->data;
  if (win) {
    XEvent ev;
    ev.xclient.format       = 32;
    ev.xclient.window       = win;
    ev.xclient.type         = ClientMessage;
    ev.xclient.message_type = XInternAtom(g_display, "WM_PROTOCOLS", true);
    ev.xclient.data.l[0]    = XInternAtom(g_display, "WM_DELETE_WINDOW",false);
    ev.xclient.data.l[1]    = CurrentTime;
    XSendEvent(g_display, win, False, NoEventMask, &ev);
  }
}

// --------------------------------------------------------------------------
static void client_next(Args args) {
// --------------------------------------------------------------------------
  Node* n = list_next(g_clients);
  if (n) {
    client_select(n);
  }
}

// --------------------------------------------------------------------------
static void client_prev(Args args) {
// --------------------------------------------------------------------------
  Node* n = list_prev(g_clients);
  if (n) {
    client_select(n);
  }
}

// --------------------------------------------------------------------------
static void client_nth(Args args) {
// --------------------------------------------------------------------------
  Node* n = list_nth(g_clients, args.i);
  if (n) {
    client_select(n);
  }
}

// --------------------------------------------------------------------------
static void client_renumber(Args args) {
// --------------------------------------------------------------------------
  Node* n = cur_client();
  if (n) {
    list_move(g_clients, n, args.i);
  }
}

// --------------------------------------------------------------------------
static void client_move_to(Args args) {
// --------------------------------------------------------------------------
  Node* n = cur_client();
  if (!n) {return;}

  int x = args.x;
  int y = args.y;
  int w = n->w;
  int h = n->h;
	window_resize(n, x, y, w, h);
}

// --------------------------------------------------------------------------
static void client_place(Args args) {
// --------------------------------------------------------------------------
  Node* n = cur_client();
  if (!n) {return;}

  int x = n->x;
  int y = n->y;
  int w = n->w;
  int h = n->h;
  int b = BORDER_2_WIDTH;

  switch (args.i) {
    case DIRECTION_NW:
      x = y = 0;
      break;
    case DIRECTION_N:
      y = 0;
      x = (g_screen_w - w - b) / 2;
      break;
    case DIRECTION_NE:
      y = 0;
      x = g_screen_w - w - b;
      break;
    case DIRECTION_W:
      x = 0;
      y = (g_screen_h - h - b) / 2;
      break;
    case DIRECTION_C:
      x = (g_screen_w - w - b) / 2;
      y = (g_screen_h - h - b) / 2;
      break;
    case DIRECTION_E:
      x = g_screen_w - w - b;
      y = (g_screen_h - h - b) / 2;
      break;
    case DIRECTION_SW:
      x = 0;
      y = g_screen_h - h - b;
      break;
    case DIRECTION_S:
      x = (g_screen_w - w - b) / 2;
      y = g_screen_h - h - b;
      break;
    case DIRECTION_SE:
      x = g_screen_w - w - b;
      y = g_screen_h - h - b;
      break;
  }
  window_resize(n, x, y, w, h);
}

// --------------------------------------------------------------------------
static void client_place_resize(Args args) {
// --------------------------------------------------------------------------
  Node* n = cur_client();
  if (!n) {return;}

  int w_2 = g_screen_w/2;
  int w_4 = g_screen_w/4;
  int h_2 = g_screen_h/2;
  int h_4 = g_screen_h/4;
  int bw  = BORDER_2_WIDTH;

  n->state = STATE_NORMAL;
  switch (args.i) {
    case DIRECTION_NW:
      window_resize(n, 0, 0, w_2-bw, h_2-bw);
      break;
    case DIRECTION_N:
      window_resize(n, w_4, 0, w_2-bw, h_2-bw);
      break;
    case DIRECTION_NE:
      window_resize(n, w_2, 0, w_2-bw, h_2-bw);
      break;
    case DIRECTION_W:
      window_resize(n, 0, h_4, w_2-bw, h_2-bw);
      break;
    case DIRECTION_C:
      window_resize(n, w_4, h_4, w_2-bw, h_2-bw);
      break;
    case DIRECTION_E:
      window_resize(n, w_2, h_4, w_2-bw, h_2-bw);
      break;
    case DIRECTION_SW:
      window_resize(n, 0, h_2, w_2-bw, h_2-bw);
      break;
    case DIRECTION_S:
      window_resize(n, w_4, h_2, w_2-bw, h_2-bw);
      break;
    case DIRECTION_SE:
      window_resize(n, w_2, h_2, w_2-bw, h_2-bw);
      break;
  }
}

// --------------------------------------------------------------------------
static void client_center(Args args) {
// --------------------------------------------------------------------------
  Node* n = cur_client();
  if (n) {
    int w   = n->w;
    int h   = n->h;
    int x   = (g_screen_w-w-BORDER_2_WIDTH)/2;
    int y   = (g_screen_h-h-BORDER_2_WIDTH)/2;
    window_resize(n, x, y, w, h);
  }
}

// --------------------------------------------------------------------------
static void clients_cascade(Args args) {
// --------------------------------------------------------------------------
  if (g_clients) {
    int gap = 25;
    int x   = gap;
    int y   = gap;
    int n   = g_clients->size + 1;
    int w   = g_screen_w - n*gap - BORDER_2_WIDTH;
    int h   = g_screen_h - n*gap - BORDER_2_WIDTH;

    for (Node* n = g_clients->head; n; n = n->next) {
      if (n->state != STATE_HIDDEN) {
        window_resize(n, x, y, w, h);
        x += gap;
        y += gap;
      }
    }
  }
}

// --------------------------------------------------------------------------
static void client_shift(Args args) {
// --------------------------------------------------------------------------
  Node* n = cur_client();
  if (!n) {return;}

  if (n->state != STATE_MAXIMIZED) {
    char i = args.i;
    int shift_amount = SHIFT_AMOUNT;
    int x = n->x, y = n->y, w = n->w, h = n->h;
    if (n->state == STATE_MINIMIZED) {
      h = MINIMIZED_HEIGHT;
    }
    if (i == DIRECTION_N) {
      y -= shift_amount;
    } else if (i == DIRECTION_S) {
      y += shift_amount;
    } else if (i == DIRECTION_W) {
      x -= shift_amount;
    } else if (i == DIRECTION_E) {
      x += shift_amount;
    }
    window_resize(n, x, y, w, h);
  }
}

// --------------------------------------------------------------------------
static void client_resize(Args args) {
// --------------------------------------------------------------------------
  Node* n = cur_client();
  if (!n) {return;}

  if (n->state == STATE_NORMAL) {
    char i = args.i;
    int x = n->x, y = n->y, w = n->w, h = n->h;
    int shift_amount = SHIFT_AMOUNT;
    if (i == DIRECTION_N) {
      h -= shift_amount;
    } else if (i == DIRECTION_S) {
      h += shift_amount;
    } else if (i == DIRECTION_W) {
      w -= shift_amount;
    } else if (i == DIRECTION_E) {
      w += shift_amount;
    }
    window_resize(n, x, y, w, h);
  }
}

// --------------------------------------------------------------------------
static void clients_list(Args args) {
// --------------------------------------------------------------------------
  FILE* ofp;
  if (ofp = fopen(WINLIST_FNAME, "w")) {
    Node* next = g_desktops->head;
    while (next) {
      if (next->children && next->children->size) {
        emit(ofp, "DT.%ld\n", next->data);
        list_print(ofp, next->children);
      }
      next = next->next;
    }
    fclose(ofp);
    char cmd[1024];
    sprintf(cmd,
      "cat /tmp/winlist.swm "
      "&& echo -e '\nPress ENTER to exit\n' "
      "&& read"
      "&& rm %s", WINLIST_FNAME);
    char* opts[] = {TERMINAL, "-e", cmd, NULL};
    args.ptr = opts;
    spawn(args);
  }
}

// --------------------------------------------------------------------------
static int window_ignore(Window win) {
// --------------------------------------------------------------------------
  XWindowAttributes attr;
  XGetWindowAttributes(g_display, win, &attr);
  return attr.override_redirect;
}

// --------------------------------------------------------------------------
static void desktop_select(Args args) {
// --------------------------------------------------------------------------
  int i = args.i;
  Node* n = list_cur(g_desktops);
  if (n->data == i || i < 0 || i > DESKTOPS) {
    return;
  }

  list_apply(g_clients, client_hide);
  g_clients = list_select(g_desktops, list_nth(g_desktops, i))->children;
  list_apply(g_clients, client_unhide);

  if ((n = list_cur(g_clients)) != NULL) {
    client_select(n);
  }
}

// --------------------------------------------------------------------------
static void desktop_increment_by(int i) {
// --------------------------------------------------------------------------
  Node* n = list_cur(g_desktops);
  int next = n->data + i;
  if (next > DESKTOPS) {
    next = 1;
  } else if (next < 1) {
    next = DESKTOPS;
  }
  Args args = {.i = next};
  desktop_select(args);
}

// --------------------------------------------------------------------------
static void desktop_next(Args args) {
// --------------------------------------------------------------------------
  desktop_increment_by(1);
}

// --------------------------------------------------------------------------
static void desktop_prev(Args args) {
// --------------------------------------------------------------------------
  desktop_increment_by(-1);
}

// --------------------------------------------------------------------------
static void client_to_desktop(Args args) {
// --------------------------------------------------------------------------
  int i = args.i;
  Node* cur = cur_client();
  Node* from = list_cur(g_desktops);
  if (!cur || from->data == i || i < 0 || i > DESKTOPS) {
    return;
  }
  client_hide(cur);

  client_hide(cur);
  Node* new = node_clone(cur);
  Node* nxt = list_remove(g_clients, cur);

  if (nxt) {
    client_select(nxt);
  }
  list_append(list_nth(g_desktops, i)->children, new);
}

// --------------------------------------------------------------------------
static void desktops_create() {
// --------------------------------------------------------------------------
  g_desktops = list_new();
  for (int i = 1; i <= DESKTOPS; i++) {
    Node* n = list_add_tail(g_desktops, i);
    sprintf(n->title, "WS.%d", i);
    n->children = list_new();
  }
  g_clients = list_select(g_desktops, list_nth(g_desktops, 1))->children;
}

// --------------------------------------------------------------------------
static void handle_mapnotify(XEvent *ev) {
// --------------------------------------------------------------------------
  XMapEvent xmap = ev->xmap;
  Window    win  = xmap.window;
  if (win) {
    if (!window_ignore(win)) {
      char name[MAX_TITLE_LEN+1];
      Node* n = list_add_after_cur(g_clients, win);
      XSelectInput(g_display, win, PropertyChangeMask | ClientMessage);
      client_select(n);
      if (window_name(win, name, MAX_TITLE_LEN)) {
        int placed = 0;
        strncpy(n->title, name, MAX_TITLE_LEN);
        for (Application* ap = g_applications; ap->name; ap++) {
          if (strcasestr(name, ap->name) != NULL) {
            if (ap->args.width ) n->w = ap->args.width;
            if (ap->args.height) n->h = ap->args.height;
            if (n->w < 0 || n->w > g_screen_w) {
              n->w = g_screen_w - BORDER_2_WIDTH;
            }
            if (n->h < 0 || n->h > g_screen_h) {
              n->h = g_screen_h - BORDER_2_WIDTH;
            }
            (ap->callback)(ap->args);
            placed = 1;
            break;
          }
        }
        if (!placed) {
          Args args;
          client_center(args);
        }
      }
    }
  }
}

// --------------------------------------------------------------------------
static void handle_unmapnotify(XEvent *ev) {
// --------------------------------------------------------------------------
  Window win = ev->xunmap.window;
  Node* n = list_find(g_clients, win);
  if (n) {
    if ((n = list_remove(g_clients, n)) != NULL) {
      client_select(n);
    }
  }
}

// --------------------------------------------------------------------------
static void handle_configurenotify(XEvent *ev) {
// --------------------------------------------------------------------------
}

// --------------------------------------------------------------------------
static void handle_keypress(XEvent *ev) {
// --------------------------------------------------------------------------
  XKeyEvent ke  = ev->xkey;
  KeySym keysym = XLookupKeysym(&ke, 0);

  for (Shortcut *sc = g_shortcuts; sc->callback; sc++) {
    if (sc->key == keysym && sc->modkey == ke.state) {
#ifdef WM_SHOW_KEYS
      char *keyname = XKeysymToString(keysym);
      printf("  >> handle_keypress %s\n", keyname);
#endif
      sc->callback(sc->args);
      break;
    }
  }
}

// --------------------------------------------------------------------------
static void handle_buttonpress(XEvent *ev) {
// --------------------------------------------------------------------------
  Window win = ev->xbutton.subwindow;

  if (win && win != g_root) {
    XGrabPointer(g_display, win, True,
      PointerMotionMask|ButtonReleaseMask, GrabModeAsync,
      GrabModeAsync, None, None, CurrentTime);
    XGetWindowAttributes(g_display, win, &g_attr);
    g_bevent = ev->xbutton;

    Node* n = list_find(g_clients, win);
    if (n) {
      int center_x = n->x + n->w / 2;
      int center_y = n->y + n->h / 2;

      if (g_bevent.x <= center_x) {
        g_dir_motion = (g_bevent.y <= center_y)? DIRECTION_NW: DIRECTION_SW;
      } else {
        g_dir_motion = (g_bevent.y <= center_y)? DIRECTION_NE: DIRECTION_SE;
      }
      client_select(n);
    }
  }
}

// --------------------------------------------------------------------------
static void handle_buttonrelease(XEvent *ev) {
// --------------------------------------------------------------------------
  XUngrabPointer(g_display, CurrentTime);
}

// --------------------------------------------------------------------------
static void handle_motionnotify(XEvent *ev) {
// --------------------------------------------------------------------------
  Node* n = cur_client();
  if (!n) {return;}

  Window win = n->data;
  if (win == ev->xmotion.window) {
    while(XCheckTypedEvent(g_display, MotionNotify, ev)) {
      // -- wait for mouse move to stop
    }
    g_dx = ev->xbutton.x_root - g_bevent.x_root;
    g_dy = ev->xbutton.y_root - g_bevent.y_root;
    int x = g_attr.x, y = g_attr.y, w = g_attr.width, h = g_attr.height;
    if ((g_bevent.button == 1 && n->state != STATE_MAXIMIZED)
     || (g_bevent.button == 3 && n->state == STATE_MINIMIZED)) {
      // -- move
      x += g_dx;
      y += g_dy;
      window_resize(n, x, y, w, h);
    } else if (g_bevent.button == 3 && n->state == STATE_NORMAL) {
      // -- resize
      int x2 = x + w;
      int y2 = y + h;
      switch (g_dir_motion) {
        case DIRECTION_NW:
          x += g_dx;
          y += g_dy;
          break;
        case DIRECTION_NE:
          x2 += g_dx;
          y  += g_dy;
          break;
        case DIRECTION_SW:
          x  += g_dx;
          y2 += g_dy;
          break;
        case DIRECTION_SE:
          x2 += g_dx;
          y2 += g_dy;
          break;
      }
      w = x2 - x;
      h = y2 - y;
      if (w < MIN_WIDTH) {
        w = MIN_WIDTH;
      }
      if (h < MIN_HEIGHT) {
        h = MIN_HEIGHT;
      }
      window_resize(n, x, y, w, h);
    }
  }
}

// --------------------------------------------------------------------------
static void create_handlers(void) {
// --------------------------------------------------------------------------
  // -- create keyboard shortcuts
  for (Shortcut* sc = g_shortcuts; sc->callback; sc++) {
    KeyCode kc = XKeysymToKeycode(g_display, sc->key);
    if (kc) {
      XGrabKey(g_display, kc, sc->modkey, g_root, True, GrabModeAsync, GrabModeAsync);
    }
  }
  // -- grab mouse buttons 1 and 3
  XGrabButton(g_display, 1, MODKEY, g_root, True, ButtonPressMask, GrabModeAsync,GrabModeAsync, None, None);
  XGrabButton(g_display, 3, MODKEY, g_root, True, ButtonPressMask, GrabModeAsync,GrabModeAsync, None, None);
}

// --------------------------------------------------------------------------
static void sigchld(int sig) {
// --------------------------------------------------------------------------
  if (signal(SIGCHLD, sigchld) == SIG_ERR) {
    exit(1);
  }
  while (0 <waitpid(-1, NULL, WNOHANG))
    ;
}

// --------------------------------------------------------------------------
int main(int argc, char **argv) {
// --------------------------------------------------------------------------
  if (!(g_display = XOpenDisplay(NULL))) return -1;
  sigchld(0);

  g_root           = DefaultRootWindow(g_display);
  g_screen         = DefaultScreen(g_display);
  g_screen_w       = DisplayWidth(g_display, g_screen);;
  g_screen_h       = DisplayHeight(g_display, g_screen);
  g_clr_selected   = getcolor(CLR_SELECTED);
  g_clr_unselected = getcolor(CLR_UNSELECTED);

  create_handlers();
  desktops_create();

  long mask = SubstructureNotifyMask | KeyPressMask;
#ifdef WM_USE_PROPERTIES
  mask |= PropertyChangeMask;
#endif
  XSelectInput(g_display, g_root, mask);
  while (1) {
    XEvent ev;
    XNextEvent(g_display, &ev);
#ifdef WM_SHOW_EVENTS
    event_name(ev.type);
#endif
    if (g_event_handlers[ev.type]) {
      g_event_handlers[ev.type](&ev);
    }
  }
  // -- should not get here, proper exit is via quit(args) function
  return EXIT_SUCCESS;
}

