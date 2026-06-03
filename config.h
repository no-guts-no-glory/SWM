// --------------------------------------------------------------------------
// --  filename: config.h
// --   purpose: set user configurable options
// --   created: 12.05.2025
// --    author: fkish@1slash1.com
// -- tab width: 2
// --------------------------------------------------------------------------
#define MODKEY         Mod4Mask		          // -- make sure you change ...
#define OTHER          Mod1Mask             // -- both of these
#define SHIFT          ShiftMask
#define CTRL           ControlMask
#define ALL_CTRL       (MODKEY|CTRL|OTHER)  // -- this is the only place
																						// -- where OTHER is used

// --------------------------------------------------------------------------
#define DESKTOPS       4          // -- can be anything, I've found 4 more than enough
                                  // -- in my daily work
#define BORDER_WIDTH   1
#define MIN_WIDTH      100
#define MIN_HEIGHT     100

#define SHIFT_AMOUNT   4          // -- number of pixels to move windows by

#define CLR_SELECTED   "#ffa500"
#define CLR_UNSELECTED "#7f7f7f"

#define EDITOR         "vim"
#define LAUNCHER       "launch-it"
#define BROWSER        "firefox-esr"
#define ALTBROWSER     "vieb"
#define SCREENSHOT     "take-screenshot"
#define TERMINAL       "uxterm"

// --------------------------------------------------------------------------
char* g_launcher  [] = {LAUNCHER,   NULL};
char* g_browser   [] = {BROWSER,    NULL};
char* g_altbrowser[] = {ALTBROWSER, NULL};
char* g_shooter   [] = {SCREENSHOT, NULL};
char* g_terminal  [] = {TERMINAL,   NULL};

// --------------------------------------------------------------------------
#define SHORTCUTS                                                                         \
  {ALL_CTRL,     XK_q,            quit,                          {} },                    \
  {ALL_CTRL,     XK_k,            client_kill,                   {} },                    \
  {ALL_CTRL,     XK_r,            reboot,                        {} },                    \
  {ALL_CTRL,     XK_u,            shutdown,                      {} },                    \
\
  {MODKEY,       XK_Return,       spawn,                         {.ptr = g_terminal} },   \
  {MODKEY,       XK_space,        spawn,                         {.ptr = g_launcher} },   \
\
  {MODKEY,       XK_period,       client_next,                   {} },                    \
  {MODKEY,       XK_Tab,          client_next,                   {} },                    \
  {MODKEY,       XK_comma,        client_prev,                   {} },                    \
  {MODKEY|SHIFT, XK_Tab,          client_prev,                   {} },                    \
\
  {MODKEY,       XK_1,            client_nth,                    {.i =  1} },             \
  {MODKEY,       XK_2,            client_nth,                    {.i =  2} },             \
  {MODKEY,       XK_3,            client_nth,                    {.i =  3} },             \
  {MODKEY,       XK_4,            client_nth,                    {.i =  4} },             \
  {MODKEY,       XK_5,            client_nth,                    {.i =  5} },             \
  {MODKEY,       XK_6,            client_nth,                    {.i =  6} },             \
  {MODKEY,       XK_7,            client_nth,                    {.i =  7} },             \
  {MODKEY,       XK_8,            client_nth,                    {.i =  8} },             \
  {MODKEY,       XK_9,            client_nth,                    {.i =  9} },             \
  {MODKEY,       XK_0,            client_nth,                    {.i = 10} },             \
\
  {MODKEY|SHIFT, XK_1,            client_renumber,               {.i =  1} },             \
  {MODKEY|SHIFT, XK_2,            client_renumber,               {.i =  2} },             \
  {MODKEY|SHIFT, XK_3,            client_renumber,               {.i =  3} },             \
  {MODKEY|SHIFT, XK_4,            client_renumber,               {.i =  4} },             \
  {MODKEY|SHIFT, XK_5,            client_renumber,               {.i =  5} },             \
  {MODKEY|SHIFT, XK_6,            client_renumber,               {.i =  6} },             \
  {MODKEY|SHIFT, XK_7,            client_renumber,               {.i =  7} },             \
  {MODKEY|SHIFT, XK_8,            client_renumber,               {.i =  8} },             \
  {MODKEY|SHIFT, XK_9,            client_renumber,               {.i =  9} },             \
  {MODKEY|SHIFT, XK_0,            client_renumber,               {.i = 10} },             \
\
  {MODKEY,       XK_k,            clients_cascade,               {} },                    \
  {MODKEY,       XK_m,            client_maximize,               {} },                    \
  {MODKEY,       XK_n,            client_minimize,               {} },                    \
  {MODKEY,       XK_v,            client_maximize_vertically,    {} },                    \
  {MODKEY,       XK_h,            client_maximize_horizontally,  {} },                    \
  {MODKEY,       XK_bracketleft,  client_snap_to,                {.i = DIRECTION_W } },   \
  {MODKEY,       XK_bracketright, client_snap_to,                {.i = DIRECTION_E } },   \
  {MODKEY,       XK_t,            client_snap_to,                {.i = DIRECTION_N } },   \
  {MODKEY,       XK_b,            client_snap_to,                {.i = DIRECTION_S } },   \
\
  {MODKEY,       XK_q,            client_place,                  {.i = DIRECTION_NW} },   \
  {MODKEY,       XK_w,            client_place,                  {.i = DIRECTION_N } },   \
  {MODKEY,       XK_e,            client_place,                  {.i = DIRECTION_NE} },   \
  {MODKEY,       XK_a,            client_place,                  {.i = DIRECTION_W } },   \
  {MODKEY,       XK_s,            client_place,                  {.i = DIRECTION_C } },   \
  {MODKEY,       XK_d,            client_place,                  {.i = DIRECTION_E } },   \
  {MODKEY,       XK_z,            client_place,                  {.i = DIRECTION_SW} },   \
  {MODKEY,       XK_x,            client_place,                  {.i = DIRECTION_S } },   \
  {MODKEY,       XK_c,            client_place,                  {.i = DIRECTION_SE} },   \
\
  {MODKEY|SHIFT, XK_q,            client_place_resize,           {.i = DIRECTION_NW} },   \
  {MODKEY|SHIFT, XK_w,            client_place_resize,           {.i = DIRECTION_N } },   \
  {MODKEY|SHIFT, XK_e,            client_place_resize,           {.i = DIRECTION_NE} },   \
  {MODKEY|SHIFT, XK_a,            client_place_resize,           {.i = DIRECTION_W } },   \
  {MODKEY|SHIFT, XK_s,            client_place_resize,           {.i = DIRECTION_C } },   \
  {MODKEY|SHIFT, XK_d,            client_place_resize,           {.i = DIRECTION_E } },   \
  {MODKEY|SHIFT, XK_z,            client_place_resize,           {.i = DIRECTION_SW} },   \
  {MODKEY|SHIFT, XK_x,            client_place_resize,           {.i = DIRECTION_S } },   \
  {MODKEY|SHIFT, XK_c,            client_place_resize,           {.i = DIRECTION_SE} },   \
\
  {MODKEY,       XK_Up,           client_shift,                  {.i = DIRECTION_N} },    \
  {MODKEY,       XK_Down,         client_shift,                  {.i = DIRECTION_S} },    \
  {MODKEY,       XK_Left,         client_shift,                  {.i = DIRECTION_W} },    \
  {MODKEY,       XK_Right,        client_shift,                  {.i = DIRECTION_E} },    \
\
  {MODKEY|SHIFT, XK_Up,           client_resize,                 {.i = DIRECTION_N} },    \
  {MODKEY|SHIFT, XK_Down,         client_resize,                 {.i = DIRECTION_S} },    \
  {MODKEY|SHIFT, XK_Left,         client_resize,                 {.i = DIRECTION_W} },    \
  {MODKEY|SHIFT, XK_Right,        client_resize,                 {.i = DIRECTION_E} },    \
\
  {0,            XK_F1,           desktop_select,                {.i = 1} },              \
  {0,            XK_F2,           desktop_select,                {.i = 2} },              \
  {0,            XK_F3,           desktop_select,                {.i = 3} },              \
  {0,            XK_F4,           desktop_select,                {.i = 4} },              \
  {MODKEY|SHIFT, XK_period,       desktop_next,                  {} },                    \
  {MODKEY|SHIFT, XK_comma,        desktop_prev,                  {} },                    \
\
  {SHIFT,        XK_F1,           client_to_desktop,             {.i = 1} },              \
  {SHIFT,        XK_F2,           client_to_desktop,             {.i = 2} },              \
  {SHIFT,        XK_F3,           client_to_desktop,             {.i = 3} },              \
  {SHIFT,        XK_F4,           client_to_desktop,             {.i = 4} },              \
\
  {CTRL,         XK_F1,           spawn,                         {.ptr = g_browser} },    \
  {CTRL,         XK_F2,           spawn,                         {.ptr = g_altbrowser} }, \
  {CTRL,         XK_F3,           spawn,                         {.ptr = g_shooter} },    \
\
  {MODKEY,       XK_i,            client_hide_cur,               { } },                   \
  {MODKEY,       XK_l,            clients_list,                  { } },                   \

// --------------------------------------------------------------------------------------
#define APPLICATIONS                                                                      \
  {"firefox",       client_maximize, { } },                                               \
  {"vieb",          client_maximize, { } },                                               \
  {"Viewnior",      client_maximize, { } },                                               \
  {"keepass",       client_snap_to,  {.i = DIRECTION_E} },                                \
  {"zathura",       client_center,   {.width = 860, .height = -1} },                      \
  {"free42",        client_place,    {.i = DIRECTION_W} },                                \

