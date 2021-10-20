#ifndef UTILS_H
#define UTILS_H 1

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void tracker(void);

#ifndef ABUF_INIT
#define ABUF_INIT                                                              \
  { NULL, 0 }

struct abuf {
  char *b;
  int len;
};

void abAppend(struct abuf *, const char *s, int len);
void abFree(struct abuf *ab);
#endif

void clearScreen(void);
void die(const char *);

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE -1
#endif

#if !_POSIX_C_SOURCE >= 200809L || !defined _GNU_SOURCE
size_t strnlen_s(const char *s, size_t maxlen);
#endif

#if !defined _POSIX_C_SOURCE || !defined _GNU_SOURCE
#include <sys/types.h>
ssize_t getline(char **restrict buffer, size_t *restrict size,
                FILE *restrict fp);
#endif

#endif // UTILS_H
