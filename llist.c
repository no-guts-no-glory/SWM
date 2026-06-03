// --------------------------------------------------------------------------
// --  filename: llist.c
// --   purpose: implement linked lists
// --   created: 12.05.2025
// --    author: fkish@1slash1.com
// -- tab width: 2
// --------------------------------------------------------------------------
#include "llist.h"
#include <pthread.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// --------------------------------------------------------------------------
char* state_name(int state) {
// --------------------------------------------------------------------------
  char* name = "unknown state";
  switch (state) {
    case STATE_NEW:       name = "NEW";       break;
    case STATE_NORMAL:    name = "NORMAL";    break;
    case STATE_HIDDEN:    name = "HIDDEN";    break;
    case STATE_MINIMIZED: name = "MINIMIZED"; break;
    case STATE_MAXIMIZED: name = "MAXIMIZED"; break;
  }
  return name;
}

// --------------------------------------------------------------------------
Node* node_create(LINT data) {
// --------------------------------------------------------------------------
  Node* n = calloc(1, sizeof(Node));
  if (!n) {
   bail("node_create failed");
  }
  n->data = data;
  return n;
}

// --------------------------------------------------------------------------
Node* node_clone(Node* src) {
// --------------------------------------------------------------------------
  if (!src) {
   bail("node_clone received null");
  }
  Node* n = node_create(src->data);
  pthread_mutex_lock(&mutex);
  n->x = src->x;
  n->y = src->y;
  n->w = src->w;
  n->h = src->h;
  n->state = src->state;
  pthread_mutex_unlock(&mutex);
  return n;
}

// --------------------------------------------------------------------------
Node* list_append(List* list, Node* n) {
// --------------------------------------------------------------------------
  if (!list || !n) {
    bail("list_append got nulls");
  }

  pthread_mutex_lock(&mutex);
  n->next = n->prev = NULL;
  if (list->size == 0) {
    list->head = list->tail = n;
  } else {
    list->tail->next = n;
    n->prev = list->tail;
    list->tail = n;
  }
  list->size++;
  list->cur = n;
  pthread_mutex_unlock(&mutex);
  return n;
}

// --------------------------------------------------------------------------
Node* list_add_head(List* list, LINT data) {
// --------------------------------------------------------------------------
  if (!list) {
    bail("list_add_head received null");
  }
  Node* new = node_create(data);

  pthread_mutex_lock(&mutex);
  new->next = list->head;
  if (list->head) {
    list->head->prev = new;
  }
  if (list->tail == NULL) {
    list->tail = new;
  }
  list->size++;
  list->head = list->cur = new;
  pthread_mutex_unlock(&mutex);
  return new;
}

// --------------------------------------------------------------------------
Node* list_add_tail(List* list, LINT data) {
// --------------------------------------------------------------------------
  return list_append(list, node_create(data));
}

// --------------------------------------------------------------------------
Node* list_add_after_cur(List* list, LINT data) {
// --------------------------------------------------------------------------
  if (!list) {
    bail("list_add_after_cur: list is null");
  }

  if (list->size == 0) {
    return list_add_head(list, data);
  }

  Node* cur = list->cur;
  if (!cur) {
    bail("list_add_after_cur: current node is null");
  }
  Node* n = node_create(data);

  pthread_mutex_lock(&mutex);
  n->next = cur->next;
  n->prev = cur;

  if (cur->next) {
    cur->next->prev = n;
  }
  cur->next = n;

  if (cur == list->tail) {
    list->tail = n;
  }

  list->size++;
  list->cur = n;
  pthread_mutex_unlock(&mutex);
  return n;
}

// --------------------------------------------------------------------------
Node* list_remove(List* list, Node* remove) {
// --------------------------------------------------------------------------
  if (list == NULL || remove == NULL) {
    bail("list_remove received nulls");
  }

  pthread_mutex_lock(&mutex);
  if (list->head == remove) {
    list->head = remove->next;
  }
  if (remove->prev) {
    remove->prev->next = remove->next;
  }
  if (remove->next) {
    remove->next->prev = remove->prev;
  }
  if (list->tail == remove) {
    list->tail = remove->prev;
  }
  if (list->cur == remove) {
    if (remove->prev) {
      list->cur = remove->prev;
    } else {
      list->cur = remove->next;
    }
  }
  list->size--;
  free(remove);
  Node* n = list->cur;
  pthread_mutex_unlock(&mutex);
  return n;
}

// --------------------------------------------------------------------------
Node* list_next(List* list) {
// --------------------------------------------------------------------------
  if (!list || !list->cur) {
    return NULL;
  }

  pthread_mutex_lock(&mutex);
  Node* n = list->cur->next;
  while (n) {
    if (n->state != STATE_HIDDEN) {
      list->cur = n;
      pthread_mutex_unlock(&mutex);
      return n;
    }
    n = n->next;
  }
  n = list->cur = list->head;
  pthread_mutex_unlock(&mutex);
  return n;
}

// --------------------------------------------------------------------------
Node* list_prev(List* list) {
// --------------------------------------------------------------------------
  if (!list || !list->cur) {
    return NULL;
  }

  pthread_mutex_lock(&mutex);
  Node* n = list->cur->prev;
  while (n) {
    if (n->state != STATE_HIDDEN) {
      list->cur = n;
      pthread_mutex_unlock(&mutex);
      return n;
    }
    n = n->prev;
  }
  n = list->cur = list->tail;
  pthread_mutex_unlock(&mutex);
  return n;
}

// --------------------------------------------------------------------------
Node* list_select(List* list, Node* node) {
// --------------------------------------------------------------------------
  if (!list || !node) {
   bail("list_select received nulls");
  }

  pthread_mutex_lock(&mutex);
  list->cur = node;
  pthread_mutex_unlock(&mutex);
  return node;
}

// --------------------------------------------------------------------------
Node *list_find(List* list, LINT data) {
// --------------------------------------------------------------------------
  if (list && data) {
    pthread_mutex_lock(&mutex);
    Node* next = list->head;
    while (next != NULL) {
      if (next->data == data) {
        pthread_mutex_unlock(&mutex);
        return next;
      }
      next = next->next;
    }
    pthread_mutex_unlock(&mutex);
  }
  return NULL;
}

// --------------------------------------------------------------------------
Node *list_nth(List* list, int n) {
// --------------------------------------------------------------------------
  if (list == NULL || n > list->size || n < 1) return NULL;

  pthread_mutex_lock(&mutex);
  Node* next = list->head;
  while (next != NULL) {
    if (--n < 1) {
      pthread_mutex_unlock(&mutex);
      return next;
    }
    next = next->next;
  }
  pthread_mutex_unlock(&mutex);
  return NULL;
}

// --------------------------------------------------------------------------
void list_apply(List* list, void (*callback)(Node*)) {
// --------------------------------------------------------------------------
  if (list && callback) {
    Node* next = list->head;
    while (next != NULL) {
      callback(next);
      next = next->next;
    }
  }
}

// --------------------------------------------------------------------------
void list_free(List* list) {
// --------------------------------------------------------------------------
  if (list) {
    Node* temp;
    Node* next = list->head;
    while (next) {
      temp = next;
      next = next->next;
      if (temp->children) list_free(temp->children);
      free(temp);
    }
    free(list);
  }
}

// --------------------------------------------------------------------------
List* list_new(void) {
// --------------------------------------------------------------------------
  List* list = calloc(1, sizeof(List));
  if (!list) {
    bail("list_new failed");
  }
  return list;
}

// --------------------------------------------------------------------------
Node *list_cur(List* list) {
// --------------------------------------------------------------------------
  if (!list) {
    bail("list_cur's list is null");
  }
  return list->cur;
}

// --------------------------------------------------------------------------
void list_move(List* list, Node* n, int pos) {
// --------------------------------------------------------------------------
  if (!list || !n || list->size < 2 || pos < 1 || pos > list->size) return;

  pthread_mutex_lock(&mutex);
  // Remove the node from its current position
  if (n == list->head) {
    list->head = n->next;
    if (list->head) {
      list->head->prev = NULL;
    }
  } else if (n == list->tail) {
    list->tail = n->prev;
    list->tail->next = NULL;
  } else {
    n->prev->next = n->next;
    n->next->prev = n->prev;
  }

  // Now find the target position
  Node* nth = list->head;
  for (int i = 1; i < pos && nth != NULL; i++) {
    nth = nth->next;
  }

  // Insert the node at the target position
  if (nth == NULL) {
    // If nth is NULL, we are inserting at the end
    list->tail->next = n;
    n->prev = list->tail;
    n->next = NULL;
    list->tail = n;
  } else {
    // Insert before nth
    n->next = nth;
    n->prev = nth->prev;
    if (nth->prev) {
      nth->prev->next = n;
    } else {
      // If nth was the head, update head
      list->head = n;
    }
    nth->prev = n;
  }
  pthread_mutex_unlock(&mutex);
}

// --------------------------------------------------------------------------
void list_print(FILE* ofp, List* list) {
// --------------------------------------------------------------------------
  static int s_indent = 0;
  if (list) {
    int n = 0;
    Node* next = list->head;
    while (next != NULL) {
      emit(ofp, "%*s%3d | %s | x %d | y %d | w %d | h %d | %s\n", s_indent, "",
        ++n, next->title, next->x, next->y, next->w, next->h, state_name(next->state));
      if (next->children != NULL) {
        s_indent++;
        list_print(ofp, next->children);
        s_indent--;
      }
      next = next->next;
    }
    emit(ofp, "\n");
  } else {
    emit(ofp, "list null\n");
  }
}

#ifdef LTEST
  #define LOG_FILE "lltest.log"

  // ------------------------------------------------------------------------
  int main(int argc, char** argv) {
  // ------------------------------------------------------------------------
    FILE* ofp  = stderr;
    List* list = list_new();
    list_print(ofp, list);

    puts("\nadding items");
    list_add_tail(list, 1);
    list_add_tail(list, 2);
    list_add_tail(list, 3);
    list_add_tail(list, 4);
    list_add_tail(list, 5);
    list_print(ofp, list);

    Node* n = list_nth(list, 1);
    list_move(list, n, 1);
    list_print(ofp, list);

    list_free(list);
    puts("done.");
    return 0;
  }
#endif

