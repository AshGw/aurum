#include "globals.h"
#include "editor.h"
#include "utils.h"

struct editorConfig E;
struct programUtils PU;
struct key KEY;
struct debugUtil DEB;
textbuf TEXTBUF;

void textbuf_init(textbuf *t) {
  t->size = 0;
  t->linebuf = NULL;
  if (t->size != 0 || t->linebuf != NULL)
    die("Failed to Init Textbuf!");
}

unsigned int textbuf_get_nth_line_length(textbuf *t, int y) {
  return strnlen_s(t->linebuf[y], 4096);
}

#include <string.h>
static int add_textbuf(textbuf *ptrtb, char *string) {
  (ptrtb->size)++;
  int string_length = strlen(string);
  ptrtb->linebuf =
      (char **)realloc(ptrtb->linebuf, (ptrtb->size) * sizeof(char *));
  // ptrtb->linebuf = ptrbuf;
  (ptrtb->linebuf)[ptrtb->size - 1] = (char *)calloc(string_length, 1);
  memcpy((ptrtb->linebuf)[ptrtb->size - 1], string,
         string_length - 1); // ignore '\n', which is read by getline()
  (ptrtb->linebuf)[ptrtb->size - 1][string_length - 1] = '\0';
  return 1;
}

int textbuf_read(textbuf *tb, FILE *fp) {
  char *buf;
  size_t size;

  buf = NULL;
  size = 0;
  while (getline(&buf, &size, fp) >= 1) {
    add_textbuf(tb, buf);
    free(buf);
    buf = NULL;
    size = 0;
  }
  return 1;
}

int textbuf_init_for_empty_file(textbuf *t) {
  t->linebuf = (char **)calloc(1, sizeof(char *));
  t->linebuf[0] = (char *)calloc(1, sizeof(char *));
  t->linebuf[0][0] = '\0';
  t->size = 1;
  return 1;
}

void textbuf_input_char(textbuf *ptrtb, char input_char, int x, int y) {
  char *linebuf = ptrtb->linebuf[y];
  int len = strlen(linebuf);
  if (x <= len && x >= 0) {
    // strlen does not count the final null terminator.
    linebuf = realloc(linebuf, len + 2); // extra space for null terminator
    // memmove: <string.h>, c11
    // memmove(dest, src, n)
    // This is the index:
    // 0, 1, 2, ... , x, x+1, ... len
    // there are 'x' elements from 0 to x,
    // 'len-x' element from x to the end of array
    memmove(&linebuf[x + 1], &linebuf[x], (len - x) * sizeof(char));
    linebuf[x] = input_char; // Assign Character
    linebuf[len + 1] = '\0';
    ptrtb->linebuf[y] = linebuf;
  }
}

// it deletes the x th char in the yth row
// Counting from 0th
void textbuf_delete_char(textbuf *ptrtb, int x, int y) {
  char *linebuf = ptrtb->linebuf[y];
  int len = strnlen_s(linebuf, 1024);
  // the left padding length shall be included
  // needs to take account of the null Character
  if (x < len && x >= 0) {
    linebuf = realloc(linebuf, len + 2); // extra space for null terminator
    memmove(&linebuf[x], &linebuf[x + 1], (len - x) * sizeof(char));
    linebuf = realloc(linebuf, len); // extra space for null terminator
    ptrtb->linebuf[y] = linebuf;
  }
}

void textbuf_enter(textbuf *ptrtb, unsigned int x, unsigned int y) {
  // Input: pointer to textbuf, TEXTBUFPosX, TEXTBUFPosY
  const unsigned int len = ptrtb->size;
  if (y < len) {
    ptrtb->size++;
    char **linebuf = ptrtb->linebuf;
    linebuf = realloc(linebuf, (len + 1) * sizeof(char *));
    ptrtb->linebuf = linebuf; // in case of realloc changed the pointer
    memmove(&linebuf[y + 1], &linebuf[y], (len - y) * sizeof(char *));
    // calloc is required to create a new pointer with allocated space
    linebuf[y] = (char *)calloc(1 + x, sizeof(char));
    // Modify the line above
    memcpy(linebuf[y], linebuf[y + 1], x * sizeof(char));
    linebuf[y][x] = '\0';
    // Modify the line below
    const unsigned int str_length = strlen(linebuf[y + 1]);
    memmove(linebuf[y + 1], linebuf[y + 1] + x, str_length - x);
    linebuf[y + 1][str_length - x] = '\0';
    linebuf[y + 1] = realloc(linebuf[y + 1], str_length - x + 1);
    ptrtb->linebuf = linebuf;
  }
}

int textbuf_delete_line(textbuf *ptrtb, unsigned int y) {
  free(ptrtb->linebuf[y]);
  memmove(&(ptrtb->linebuf[y]), &(ptrtb->linebuf[y + 1]), (ptrtb->size - y - 1) * sizeof(char *));
  ptrtb->linebuf = realloc(ptrtb->linebuf, (ptrtb->size - 1));
  ptrtb->size--;
  return 1;
}

int textbuf_delete_line_break(textbuf *t, unsigned int y) {
  const int len_lower = strlen(t->linebuf[y]);
  const int len_upper = strlen(t->linebuf[y - 1]);
  t->linebuf[y - 1] = realloc(t->linebuf[y - 1], len_lower + len_upper + 1);
  memcpy(&(t->linebuf[y - 1][len_upper]), t->linebuf[y], len_lower);
  t->linebuf[y - 1][len_lower + len_upper] = '\0';
  free(t->linebuf[y]);
  memmove(&(t->linebuf[y]), &(t->linebuf[y + 1]), (t->size - y - 1) * sizeof(char *));
  t->linebuf = realloc(t->linebuf, (t->size - 1));
  t->size--;
  editor_move_cursor(KEY_ARROW_UP);
  editor_cursor_x_to_textbuf_pos(len_upper);
  return 1;
}

int key_init(struct key *K) {
  K->key[0] = 0;
  if (K->key[0] != 0)
    return -1;
  return 0;
}

int key_refresh(struct key *K) {
  for (int i = 0; i < 8; i++) {
    K->key[i] = 0;
    if (K->key[i] != 0)
      return -1;
  }
  return 0;
}

void program_utils_init(struct program_utils *p) {
  p->running = 1;
  p->updated = 1;
}

int debug_util_init(struct debug_util *d) {
  d->debug_string = (struct abuf *)malloc(sizeof(struct abuf));
  d->debug_string->len = 0;
  d->debug_string->b = NULL;
  return 1;
}

/// The string must be null terminated
// replace strlen if possible
int debug_add_message(struct debug_util *d, const char *string) {
  abAppend(d->debug_string, string, strnlen_s(string, 256));
  return 1;
}
