#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "utils.h"

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 0
#endif

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 0
#endif

void tracker(void) { return; }

void clearScreen(void) {
  write(STDIN_FILENO, "\x1b[2J", 4);
  write(STDIN_FILENO, "\x1b[H", 3);
}

void die(const char *s) {
  clearScreen();
  perror(s);
  write(STDIN_FILENO, "\r", 1);
  exit(1);
}

#if !_POSIX_C_SOURCE >= 200809L || !defined _GNU_SOURCE
size_t strnlen_s(const char *s, size_t maxlen) {
  size_t res;
  for (res = 0; *(s + res) != '\0' && res <= maxlen; res++)
    ;
  return res;
}
#endif

#ifdef ABUF_INIT
void abAppend(struct abuf *ab, const char *s, int len) {
  char *new = realloc(ab->b, len + ab->len);
  if (new == NULL)
    die("Fail to realloc memory for function abAppend");
  memcpy(&new[ab->len], s, len);
  ab->b = new;
  ab->len += len;
}

void abFree(struct abuf *ab) {
  free(ab->b);
  ab->b = NULL;
  ab->len = 0;
}
#endif



#if !_POSIX_C_SOURCE >= 200809L || !defined _GNU_SOURCE

ssize_t getline(char **restrict buffer, size_t *restrict size,
                FILE *restrict fp) {
  register int c;
  register char *cs = NULL;

  if (cs == NULL) {
    register int length = 0;
    while ((c = getc(fp)) != EOF) {
      cs = (char *)realloc(cs, ++length + 1);
      if ((*(cs + length - 1) = c) == '\n') {
        *(cs + length) = '\0';
        *buffer = cs;
        break;
      }
    }
    return (ssize_t)(*size = length);
  } else {
    while (--(*size) > 0 && (c = getc(fp)) != EOF) {
      if ((*cs++ = c) == '\n')
        break;
    }
    *cs = '\0';
  }
  return (ssize_t)(*size = strlen(*buffer));
}
#endif
