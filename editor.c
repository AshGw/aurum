#include "utils.h"
#include "globals.h"
#include "editor.h"

extern struct editorConfig E;
extern struct programUtils PU;
extern struct key KEY;
extern struct debugUtil DEB;
extern textbuf TEXTBUF;

/*** FILE IO ***/
// Change to open() syscall
void editorOpen(const char *filename) {
  FILE *fp = fopen(filename, "ab+");
  if (!fp) {
    char errorMessage[100];
    snprintf(errorMessage, 100, "Can not open File '%s'\r\nperrer message",
             filename);
    die(errorMessage);
  }

  textbufRead(&TEXTBUF, fp); // All lines of the file are read into TEXTBUF
  fclose(fp);
	if (TEXTBUF.size==0){
		textbufInitForEmptyFile(&TEXTBUF);
	}
}

/*** Input ***/
/// Reads and returns the key once.
int editorReadKey(void) {
	char c;
	int nread = read(STDIN_FILENO, &c, 1);
	// read returns '\0' if no input is received after 0.1 s
	// read returns number of byte read. -1 when failure.
	if (nread == -1 && errno != EAGAIN)
		return -1;

	if (!nread) return 0;

	// Test if it is a normal keypress
	PU.updated = 1;
  if (c != '\x1b' ){ 
		KEY.key[0] = c;
		return c;
	}
	
  char seq[3];
  for (int i = 0; i < 2; ++i)
    if (read(STDIN_FILENO, &seq[i], 1) < 1)
      return '\x1b';

  if (seq[0] != '[')
    return '\x1b';

 // `DELETE`: \x1b[3~
 // `PAGE_UP`: \x1b[5~
 // `PAGE_DOWN`: \x1b[6~
  switch (seq[1]) {
    case '5':
    case '6':
    case '3':
    if (read(STDIN_FILENO, &seq[2], 1) < 1)
      return '\x1b';
    else if (seq[2]!='~') 
      return '\x1b'; 
    else if (seq[1] == '3')
      KEY.key[0] = KEY_DELETE;
    else if (seq[1] == '5')
      KEY.key[0] = KEY_PAGE_UP;
    else if (seq[1] == '6')
      KEY.key[0] = KEY_PAGE_DOWN;
    break;
  case 'A':
		KEY.key[0] = KEY_ARROW_UP;
		break;
  case 'B':
		KEY.key[0] = KEY_ARROW_DOWN;
		break;
  case 'C':
		KEY.key[0] = KEY_ARROW_RIGHT;
		break;
  case 'D':
		KEY.key[0] = KEY_ARROW_LEFT;
		break;
  case 'H':
		KEY.key[0] = KEY_HOME;
		break;
  case 'F':
		KEY.key[0] = KEY_END;
		break;
  default:
    return '\x1b';
  }
	if (KEY.key[0] != '\x1b')
		return KEY.key[0];
	else return '\x1b';
}

int editorProcessKeyPress(void) {
  unsigned int c = KEY.key[0];
  const unsigned int textbufXPos = editorGetCursorTextbufPosX();
  const unsigned int textbufYPos = editorGetCursorTextbufPosY();
  switch (c){
    case '\0':
      break;
    case CTRL_KEY('s'):
      editorSaveFile(E.fileName.b);
      break;
    case CTRL_KEY('q'):
      clearScreen();
      PU.running = 0;
      break;
    case 13:  // Enter key, or ctrl('m')
      textbufEnter(&TEXTBUF, textbufXPos, textbufYPos);
      E.cursorTextbufPosX = 0;
      // Scroll down when enter is used in last line of the screen
      // TODO: REFACTOR 
      editorMoveCursor(KEY_ARROW_DOWN);
      break;
    case KEY_ARROW_LEFT:
    case KEY_ARROW_RIGHT:
    case KEY_ARROW_DOWN:
    case KEY_ARROW_UP:
      editorMoveCursor(c);
      break;
    case KEY_PAGE_UP:
    case KEY_PAGE_DOWN:
      for (unsigned int i = 0; i < E.screenrows; i++)
        editorMoveCursor(c);
      break;
    case KEY_DELETE: {
      const unsigned int len = strlen(TEXTBUF.linebuf[textbufYPos]);
      if ((textbufXPos) < len)
        textbufDeleteChar(&TEXTBUF, textbufXPos, textbufYPos);
      break;
    }
    case KEY_HOME:
    case KEY_END:
      break;
    case 127:  // Backspace
      if (textbufXPos > 0){
        textbufDeleteChar(&TEXTBUF, textbufXPos - 1, textbufYPos);
        editorMoveCursor(KEY_ARROW_LEFT);
      } else if (textbufYPos > 0 && textbufXPos<TEXTBUF.size){
        textbufDeleteLineBreak(&TEXTBUF, textbufYPos);
      }
    case 27:  // escape
      break;
    default:
      // for now ignore all control-keys
      if (c < 27) 
        break;
        // special characters are defined to be greater than 1000
      else if (c < 1000){ 
        textbufInputChar(&TEXTBUF, c, textbufXPos, textbufYPos);
        editorMoveCursor(KEY_ARROW_RIGHT);
      }
      break;
  }
  return 1;
}

void editorSaveFile(char *ptr){
	// 0644 is octal, equivalent to 110100100 in binary
	// Owner can read and write, all other can only read.
	int fd = open(ptr, O_CREAT | O_WRONLY | O_TRUNC, 0644);	
	if (fd == -1){
		const int message_size = 64;
		char *message = (char*)calloc(message_size, sizeof(char));
		snprintf(message, message_size, "Failed to open or create file: %s", ptr);
		die(message);
	}
	for (unsigned int i = 0; i < TEXTBUF.size; i++){
    if (write(fd, TEXTBUF.linebuf[i], strlen(TEXTBUF.linebuf[i])) == -1)
      die("Failed to Write to Disk!");
		if (write(fd, "\n", 1) == -1)
      die("Failed to Write to Disk!");
	}
	close(fd);
	return;
}

static int appendWelcomeMessage(struct abuf *ptr) {
  struct abuf *abptr = ptr;
  char welcome[80];
  // KILO_VERSION defined in main.c
  // snprintf is form <stdio.h>
  unsigned int welcomelen =
      snprintf(welcome, sizeof(welcome), "Kilo Editor -- Version %d.%d.%d",
               KILO_VERSION_MAJOR, KILO_VERSION_MINOR, KILO_VERSION_PATCH);
  if (welcomelen > E.screencols)
    welcomelen = E.screencols;

  // Center the Message
  unsigned int padding = (E.screencols - welcomelen) / 2;
  if (padding) {
    abAppend(abptr, "~", 1);
    padding--;
  }
  while (padding--)
    abAppend(abptr, " ", 1);

  abAppend(abptr, welcome, welcomelen);

  return 1;
}

void screenBufferAppendDebugInformation(struct abuf *abptr){
  const int buf_size = 100;
  char *buf = (char*)malloc(buf_size);
  snprintf(buf, buf_size, 
           "TexbufX: %d; TexbufY: %d; ScreenY: %d; rows: %d; cols: %d",
            editorGetCursorTextbufPosX(), editorGetCursorTextbufPosY(),
            editorGetCursorScreenPosY(),E.screenrows, E.screencols);
  abAppend(DEB.debugString, buf, strlen(buf));  // Append message to the global struct
  free(buf);
  abAppend(abptr, DEB.debugString->b, DEB.debugString->len);	
  abFree(DEB.debugString);
}

/*** Output ***/
void editorDrawRows(struct abuf *abptr) {
  for (unsigned int nrows = 0; nrows < E.screenrows ; nrows++) {  // number of iteration is siginificant!
    // the line number of the row to be drawn
    const unsigned int n_rows_to_draw = nrows + E.offsety;

    // Create left margin (line number)
    char *leftMargin = (char *)calloc(E.leftMarginSize, 1);
    int lineNumber;
    if (nrows ==  editorGetCursorScreenPosY()){
      lineNumber = editorGetCursorTextbufPosY() + 1; // linenumber counts from 1
      snprintf(leftMargin, E.leftMarginSize, "%d", lineNumber);
      // As in vim, the line number in the current line is aliged to right
      // Create necessary paddings
      if (strnlen_s(leftMargin, 256)<E.leftMarginSize){
        // recall E.leftMarginSize is seted to 1 more than the maximum 
        // line number to include the extra space
        for (size_t i = 0; i < E.leftMarginSize-strlen(leftMargin)-1; i++){
          abAppend(abptr, " ", 1);
        }
      }
      abAppend(abptr, "\x1b[1m", 4);
      abAppend(abptr, leftMargin, strnlen_s(leftMargin, 256));
      abAppend(abptr, "\x1b[0m", 4);
      // the extra space
      abAppend(abptr, " ", 1);
    } else {
      const int temp = nrows - editorGetCursorScreenPosY();
      lineNumber = temp > 0 ? temp : -temp; 
      // Only display relative number for lines displayed by textbuf
      if ((nrows+ E.offsety) < TEXTBUF.size){
        snprintf(leftMargin, E.leftMarginSize, "%d", lineNumber);
      } 
      else {
        snprintf(leftMargin, E.leftMarginSize, "~");
      }
      abAppend(abptr, leftMargin, strnlen_s(leftMargin, 256));
      // Create necessary paddings
      // recall E.leftMarginSize is seted to 1 more than the maximum 
      for (size_t i = 0; i < E.leftMarginSize-strlen(leftMargin)-1; i++){
        abAppend(abptr, " ", 1);
      }
      // The extra space 
      abAppend(abptr, " ", 1);
    }

    if (n_rows_to_draw >= TEXTBUF.size) {
    }
    else if (nrows == E.screenrows-1){ // For debugging purpose
      // screenBufferAppendDebugInformation(abptr);
		}
    else {
      if (TEXTBUF.linebuf == NULL) return; 
      // temp points to the string of the row to be drawn.
      char *temp = *(TEXTBUF.linebuf + n_rows_to_draw);
      const unsigned int stringlen = strlen(temp);
      // For calculate the spaces for direction scrolling.
      const unsigned int xoffset = E.offsetx >= stringlen ? stringlen : E.offsetx;
      temp += xoffset;
      // Calculate the correct display length of the buffer
      unsigned int bufferlen = stringlen - xoffset; // same as strlen(temp)
      bufferlen = (bufferlen >= E.screencols - E.leftMarginSize) ? 
        E.screencols - E.leftMarginSize : bufferlen;
      // abAppend(abptr, " ", 1);  // The space before the Line.
      abAppend(abptr, temp, bufferlen);
    }
		if (nrows<E.screenrows - 1) abAppend(abptr, "\r\n", 2);
  }
}

void editorRefreshScreen(void) {
  // init append buffer
  struct abuf ab = ABUF_INIT;

  abAppend(&ab, "\x1b[?25l", 6); // Hide cursor

  editorDrawRows(&ab);

  // Move mouse to correct position
  char buf[32];
	// move cursor, row:cols; top left is 1:1
  const unsigned int CursorScreenX = editorGetCursorScreenPosX();
  const unsigned int CursorScreenY = editorGetCursorScreenPosY();
  snprintf(buf, sizeof(buf), "\x1b[%d;%dH", CursorScreenY + 1, CursorScreenX + 1); 
  // abAppend(&ab, "\x1b[H", 3); 
  abAppend(&ab, buf, strlen(buf)); // To corrected position

  abAppend(&ab, "\x1b[?25h", 6); // Show cursor
  write(STDIN_FILENO, "\x1b[2J\x1b[H", 7);  // erase entire screen
  write(STDIN_FILENO, ab.b, ab.len);
  abFree(&ab);
}

void editorScrollDown(void) {
    E.offsety++;
}

void editorScrollUp(void) {
  	E.offsety--;
}

void editorScrollLeft(void) {
  E.offsetx--;
}

void editorScrollRight(void) {
  E.offsetx++;
}

void editorCursorXToTextbufPos(unsigned int x){
	E.cursorTextbufPosX = x;
}

void editorCursorYToTextbufPos(unsigned int y){
	E.cursorTextbufPosY = y;
}

void editorMoveCursorToEndOfLine(textbuf * txb, unsigned int y){
  // input, textbuf, line number
  editorCursorXToTextbufPos(
    textbufGetNthLineLength(txb, y));
  return;
}

void editorConfineCursorPosition(textbuf *txb, int xPos, int yPos){
  if (xPos<0)
    editorCursorXToTextbufPos(0);

  int lineLength = textbufGetNthLineLength(txb, yPos);
  if (xPos >= lineLength){
    editorCursorXToTextbufPos(lineLength);
  }
  if (yPos<0)
    editorCursorYToTextbufPos(0);
  if (yPos>(txb->size))
    editorCursorYToTextbufPos(txb->size);
}

// TODO: LEFT/WRITE ARROW MECHANISM
int editorMoveCursor(int key) {
  switch (key) {
  case KEY_ARROW_UP: 
  case KEY_PAGE_UP:
    if (E.cursorTextbufPosY > 0) {
      if (editorGetCursorScreenPosY() <= 0) --E.offsety;
      E.cursorTextbufPosY--;
    }
    break;
  case KEY_ARROW_DOWN:
  case KEY_PAGE_DOWN:
    if (E.cursorTextbufPosY < TEXTBUF.size - 1){
      // screenPos counts from 0, screen rows counts from 1
      if (editorGetCursorScreenPosY() >= E.screenrows-1) ++E.offsety;
      E.cursorTextbufPosY++;
    }
    break;
  case KEY_ARROW_LEFT: 
    // E.cursorTextbufPosX is unsigned int
    if (E.cursorTextbufPosX > 0) 
      E.cursorTextbufPosX--;
    break;
  case KEY_ARROW_RIGHT: 
    if (editorCursorMovableToRight(&TEXTBUF,
        editorGetCursorTextbufPosX(),
        editorGetCursorTextbufPosY())
      )
      E.cursorTextbufPosX++;
    break;
  default:
    return -1;
  }

  // Move the cursor back to valid location
  // A valid location is a location in range of the textbuffer
  editorConfineCursorPosition(&TEXTBUF,
                              editorGetCursorTextbufPosX(),
                              editorGetCursorTextbufPosY());
  return 0;
}

// Set editorConfig.leftMarginSize according to 
// the digits of the maxium line number stored in textbuf
// The minimum size of leftMarginSize is 4
void editorSetMarginSize(struct editorConfig *ptr,textbuf *ptrtb){
  int NumberOflines = ptrtb->size; 
  int counter;
  // Find out the digits of greatest linenumber
  for (counter = 0; NumberOflines>0; NumberOflines /= 10, counter ++);
  // One more space for padding ' '
  counter++;
  counter = (counter >= 4) ? counter : 4;
  ptr->leftMarginSize = counter;
  return;
}


int editorGetCursorScreenPosX(void){
  return E.cursorTextbufPosX + E.leftMarginSize - E.offsetx; 
}

int editorGetCursorScreenPosY(void){
  return E.cursorTextbufPosY - E.offsety;
}

int editorGetCursorTextbufPosX(void){
  return E.cursorTextbufPosX;
}

int editorGetCursorTextbufPosY(void){
  return E.cursorTextbufPosY;
}

// See if the cursor can be moved further to write 
// with current cursro textbuf postion and textbuffer 
int editorCursorMovableToRight(
  textbuf * txb, unsigned int cursorTxbPosX, unsigned int cursorTxbPosY){
  return textbufGetNthLineLength(txb, cursorTxbPosY) >= cursorTxbPosX + 1; 
}