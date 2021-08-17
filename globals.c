#include "globals.h"
#include "editor.h"
#include "utils.h"

struct editorConfig E;
struct programUtils PU;
struct key KEY;
struct debugUtil DEB;
textbuf TEXTBUF;

void textbufInit(textbuf *t) {
  t->size = 0;
  t->linebuf = NULL;
  if (t->size != 0 || t->linebuf != NULL)
    die("Failed to Init Textbuf!");
}

unsigned int textbufGetNthLineLength(textbuf *t, int y){
	return strnlen_s(t->linebuf[y], 4096);
}

#include <string.h>
static int addtextbuf(textbuf *ptrtb, char *string) {
  (ptrtb->size)++;
  int stringLength = strlen(string);
  ptrtb->linebuf =
      (char **)realloc(ptrtb->linebuf, (ptrtb->size) * sizeof(char *));
  // ptrtb->linebuf = ptrbuf;
  (ptrtb->linebuf)[ptrtb->size - 1] = (char *)calloc(stringLength, 1);
  memcpy((ptrtb->linebuf)[ptrtb->size - 1], string,
         stringLength - 1); // ignore '\n', which is read by getline()
  (ptrtb->linebuf)[ptrtb->size - 1][stringLength - 1] = '\0';
  return 1;
}

int textbufRead(textbuf *tb, FILE *fp) {
  char *buf;
  size_t size;

  buf = NULL;
  size = 0;
  while (getline(&buf, &size, fp) >= 1) {
    addtextbuf(tb, buf);
    free(buf);
    buf = NULL;
    size = 0;
  }
  return 1;
}

int textbufInitForEmptyFile(textbuf *t){
	t->linebuf = (char **) calloc(1, sizeof(char*));
	t->linebuf[0] = (char *) calloc(1, sizeof(char*));
	t->linebuf[0][0] = '\0';
	t->size = 1;
	return 1;
}

void textbufInputChar(textbuf *ptrtb, char inputChar, int x, int y) {
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
    linebuf[x] = inputChar; // Assign Character
    linebuf[len + 1] = '\0';
    ptrtb->linebuf[y] = linebuf;
  }
}

// it deletes the x th char in the yth row
// Counting from 0th
void textbufDeleteChar(textbuf *ptrtb, int x, int y) {
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

void textbufEnter(textbuf *ptrtb, unsigned int x, unsigned int y) {
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
    const unsigned int strLength = strlen(linebuf[y + 1]);
    memmove(linebuf[y + 1], linebuf[y + 1] + x, strLength - x);
    linebuf[y + 1][strLength - x] = '\0';
    linebuf[y + 1] = realloc(linebuf[y + 1], strLength - x + 1);
		ptrtb->linebuf = linebuf;
  }
}

int textbufDeleteLine(textbuf *ptrtb, unsigned int y){
	free(ptrtb->linebuf[y]);
	memmove(&(ptrtb->linebuf[y]), &(ptrtb->linebuf[y+1]), (ptrtb->size - y - 1) * sizeof(char *));
	ptrtb->linebuf = realloc(ptrtb->linebuf, (ptrtb->size - 1));
	ptrtb->size--;
	return 1;
}

int textbufDeleteLineBreak(textbuf *t, unsigned int y){
	const int lenLower = strlen(t->linebuf[y]);
	const int lenUpper = strlen(t->linebuf[y-1]);
	t->linebuf[y-1] = realloc(t->linebuf[y-1], lenLower + lenUpper+1);	
	memcpy(&(t->linebuf[y-1][lenUpper]), t->linebuf[y], lenLower);
	t->linebuf[y-1][lenLower+lenUpper] = '\0';
	free(t->linebuf[y]);
	memmove(&(t->linebuf[y]), &(t->linebuf[y+1]), (t->size - y - 1) * sizeof(char *));
	t->linebuf = realloc(t->linebuf, (t->size - 1));
	t->size--;
	editorMoveCursor(KEY_ARROW_UP);
	editorCursorXToTextbufPos(lenUpper);
	return 1;
}

int keyInit(struct key *K) {
  K->key[0] = 0;
  if (K->key[0] != 0)
		return -1;
	return 0;
}

int keyRefresh(struct key *K){
	for (int i = 0; i < 8; i++){
		K->key[i] = 0;
		if (K->key[i] != 0) return -1;
	}
	return 0;
}

void programUtilsInit(struct programUtils *p) {
  p->running = 1;
  p->updated = 1;
}


int debugUtilInit(struct debugUtil *d){
	d->debugString = (struct abuf *)malloc(sizeof(struct abuf));
	d->debugString->len = 0;
	d->debugString->b = NULL;
	return 1;
}

/// The string must be null terminated 
// replace strlen if possible
int debugAddMessage(struct debugUtil *d, const char *string){
		abAppend(d->debugString, string, strnlen_s(string, 256));
	return 1;
}
