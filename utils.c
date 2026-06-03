// --------------------------------------------------------------------------
// --  filename: utils.c
// --   purpose: implement some functions used by swm and ltest apps
// --   created: 10.05.2025
// --    author: fkish@1slash1.com
// -- tab width: 2
// --------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>

// --------------------------------------------------------------------------
#define LOG_FNAME "/tmp/log.swm"

// --------------------------------------------------------------------------
void bail(char *fmt, ...) {
// --------------------------------------------------------------------------
  FILE* ofp = fopen(LOG_FNAME, "a");
  if (ofp) {
    va_list args;
    va_start(args, fmt);
    fprintf(ofp, "ERROR: ");
    vfprintf(ofp, fmt, args);
    fprintf(ofp, "\n");
    va_end(args);
    fclose(ofp);
  }
  exit(-1);
}

// --------------------------------------------------------------------------
void emit(FILE* ofp, char *fmt, ...) {
// --------------------------------------------------------------------------
  va_list args;
  va_start(args, fmt);
  vfprintf(ofp, fmt, args);
  va_end(args);
}
