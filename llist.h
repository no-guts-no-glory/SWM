// --------------------------------------------------------------------------
// --  filename: llist.h
// --   purpose: define structures and prototypes for linked-lists
// --   created: 12.05.2025
// --    author: fkish@1slash1.com
// -- tab width: 2
// --------------------------------------------------------------------------
#ifndef __LLIST_H__
#define __LLIST_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

// --------------------------------------------------------------------------
#define MAX_TITLE_LEN  1024

// --------------------------------------------------------------------------
typedef struct Node   Node;
typedef struct List   List;
typedef long int      LINT;
typedef unsigned long ULONG;

// --------------------------------------------------------------------------
enum {
  STATE_NEW,
  STATE_NORMAL,
  STATE_HIDDEN,
  STATE_MINIMIZED,
  STATE_MAXIMIZED,
};

// --------------------------------------------------------------------------
struct Node {
// --------------------------------------------------------------------------
  int   x;
  int   y;
  int   w;
  int   h;
  LINT  data;
  int   state;
  int   state_prev;
  Node* next;
  Node* prev;
  Node* hidden;
  List* children;
  char  title[MAX_TITLE_LEN+1];
};

// --------------------------------------------------------------------------
struct List {
// --------------------------------------------------------------------------
  int   size;
  Node* head;
  Node* tail;
  Node* cur;
};

// --------------------------------------------------------------------------
Node* node_clone         (Node* n);
void  node_print         (FILE* ofp, Node* n);
List* list_new           (void);
void  list_free          (List* list);
Node* list_cur           (List* list);
void  list_print         (FILE* ofp, List* list);
Node* list_next          (List* list);
Node* list_prev          (List* list);
Node* list_nth           (List* list, int   n);
int   list_index         (List* list, Node* n);
Node* list_find          (List* list, LINT  data);
Node* list_select        (List* list, Node* node);
Node* list_add_head      (List* list, LINT  data);
Node* list_add_tail      (List* list, LINT  data);
Node* list_add_after_cur (List* list, LINT  data);
Node* list_append        (List* list, Node* node);
Node* list_remove        (List* list, Node* node);
void  list_move          (List* list, Node* n, int pos);
void  list_apply         (List* list, void  (*callback)(Node*));

void  bail               (char* fmt, ...);
void  emit               (FILE* ofp, char* fmt, ...);

#endif

