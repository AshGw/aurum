#ifndef GLOBALS_H
#define GLOBALS_H 1

#ifndef CTRL_KEY
#define CTRL_KEY(k) ((k) & 0x1f)
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

#include "utils.h"
#include <termios.h>

typedef struct textbuf {
  // TODO: Change size to numlines
  unsigned int size; // Total number of lines
  char **linebuf;    // A pointer storing pointer to line buffer
  int *line_length;  // line_length[n] = strlen(linebuf[n])
} textbuf;           // text_buffer holding all lines.

#include <stddef.h>

void textbuf_init(textbuf *);
unsigned int textbuf_get_nth_line_length(textbuf *, int);

#include <stdio.h>

int textbuf_read(textbuf *, FILE *);

int textbuf_init_for_empty_file(textbuf *);
void textbuf_input_char(textbuf *, char, int, int);
void textbuf_delete_char(textbuf *, int, int);
void textbuf_enter(textbuf *, unsigned int, unsigned int);
int textbuf_delete_line(textbuf *, unsigned int);
int textbuf_delete_line_break(textbuf *, unsigned int);

struct editor_config {
  unsigned int cx, cy;     // cursor position. cx horizontal, cy vertical
  // unsigned int cspx, cspy; // cursor screen position. cx horizontal, cy vertical
  // TODO: set cursor_textbuf_pos_x, cursor_textbuf_pos_y to be int; they are not unsigned int
  unsigned int cursor_textbuf_pos_x, cursor_textbuf_pos_y; // Cursor textbuf position
  unsigned int screen_rows; // number of rows that fit in the screen
  unsigned int screen_cols; // number of columns that fit in the screen
  unsigned int offset_x;    // Display offset, x direction
  unsigned int offset_y;
  unsigned int mode; // Indicator for mode
  unsigned int left_margin_size;
  struct abuf file_name;
  struct termios orig_termios;
};

struct program_utils {
  unsigned int running;
  unsigned int updated;
};

void program_utils_init(struct program_utils *);

///  The key struct can hold up to 8 keys
///  If the key value is smaller
/// than 1000, it represents the key corresponding to the ASCII code
/// key value of 1000 - 2000 represents escaped key
/// note 'ctrl(a)' = 'a' - 96 = 1
struct key {
  unsigned int key[8];
};

int key_init(struct key *);
int key_refresh(struct key *);

struct debug_util {
  struct abuf *debug_string;
};

int debug_util_init(struct debug_util *);
// The string must be null terminated
int debug_add_message(struct debug_util *, const char *);

#endif // for GLOBALS_H
