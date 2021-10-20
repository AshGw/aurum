
#include "utils.h"
#include <stdio.h>
#include <termios.h>

#ifndef GLOBALS_H
#define GLOBALS_H 1

#ifndef CTRL_KEY
#define CTRL_KEY(k) ((k)&0x1f)
#endif

#define KILO_VERSION_MAJOR 0
#define KILO_VERSION_MINOR 0
#define KILO_VERSION_PATCH 1

// Key values

#ifndef KEY_ARROW_UP
#define KEY_ARROW_UP 1065
#endif

#ifndef KEY_ARROW_DOWN
#define KEY_ARROW_DOWN 1066
#endif

#ifndef KEY_ARROW_LEFT
#define KEY_ARROW_LEFT 1067
#endif

#ifndef KEY_ARROW_RIGHT
#define KEY_ARROW_RIGHT 1068
#endif

#ifndef KEY_PAGE_UP
#define KEY_PAGE_UP 1053
#endif

#ifndef KEY_PAGE_DOWN
#define KEY_PAGE_DOWN 1054
#endif

#ifndef KEY_DELETE
#define KEY_DELETE 1051
#endif

#ifndef KEY_END
#define KEY_END 1070
#endif

#ifndef KEY_HOME
#define KEY_HOME 1072
#endif

struct editorConfig {
  unsigned int cx, cy; // cursor position. cx horizantol, cy vertical
  // unsigned int cspx, cspy; // cursor screen position. cx horizantol, cy
  // vertical
  // TODO: set cursorTextbufPosX, cursorTextbufPosY to be int; they are not
  // unsigned int
  unsigned int cursorTextbufPosX, cursorTextbufPosY; // Cursor textbuf position
  unsigned int screenrows; // number of rows that fit in the screen
  unsigned int screencols; // number of columns that fit in the screen
  unsigned int offsetx;    // Display offset, x direction
  unsigned int offsety;
  unsigned int mode; // Indicator for mode
  unsigned int leftMarginSize;
  struct abuf fileName;
  struct termios orig_termios;
};

struct programUtils {
  unsigned int running;
  unsigned int updated;
};

typedef struct textbuf {
  // TODO: Change size to numlines
  unsigned int size; // Total number of lines
  char **linebuf;    // A pointer storing pointer to line buffer
  int *lineLength;   // lineLength[n] = strlen(linebuf[n])
} textbuf;           // textbuffer holding all lines.

struct debugUtil {
  struct abuf *debugString;
};

struct key {
  unsigned int key[8];
};

void textbufInit(textbuf *);
unsigned int textbufGetNthLineLength(textbuf *, int);
int textbufRead(textbuf *, FILE *);
int textbufInitForEmptyFile(textbuf *);
void textbufInputChar(textbuf *, char, int, int);
void textbufDeleteChar(textbuf *, int, int);
void textbufEnter(textbuf *, unsigned int, unsigned int);
int textbufDeleteLine(textbuf *, unsigned int);
int textbufDeleteLineBreak(textbuf *, unsigned int);
void programUtilsInit(struct programUtils *);
int keyInit(struct key *);
int keyRefresh(struct key *);
int debugUtilInit(struct debugUtil *);
int debugAddMessage(struct debugUtil *, const char *);

#endif