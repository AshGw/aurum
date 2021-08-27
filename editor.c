#include "utils.h"
#include "globals.h"
#include "editor.h"

extern struct editor_config E;
extern struct program_utils PU;
extern struct key KEY;
extern struct debug_util DEB;
extern textbuf TEXTBUF;

/*** FILE IO ***/
// Change to open() syscall
void editor_open(const char *filename) {
  FILE *fp = fopen(filename, "ab+");
  if (!fp) {
    char errorMessage[100];
    snprintf(errorMessage, 100, "Can not open File '%s'\r\nperrer message",
             filename);
    die(errorMessage);
  }

  textbuf_read(&TEXTBUF, fp); // All lines of the file are read into TEXTBUF
  fclose(fp);
	if (TEXTBUF.size==0){
		textbuf_init_for_empty_file(&TEXTBUF);
	}
}

/*** Input ***/
/// Reads and returns the key once.
int editor_read_key(void) {
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

int editor_process_key_press(void) {
  unsigned int c = KEY.key[0];
  const unsigned int textbufXPos = editor_get_cursor_textbuf_pos_x();
  const unsigned int textbufYPos = editor_get_cursor_textbuf_pos_y();
  switch (c){
    case '\0':
      break;
    case CTRL_KEY('s'):
      editor_save_file(E.file_name.b);
      break;
    case CTRL_KEY('q'):
      clear_screen();
      PU.running = 0;
      break;
    case 13:  // Enter key, or ctrl('m')
      textbuf_enter(&TEXTBUF, textbufXPos, textbufYPos);
      E.cursor_textbuf_pos_x = 0;
      // Scroll down when enter is used in last line of the screen
      // TODO: REFACTOR 
      editor_move_cursor(KEY_ARROW_DOWN);
      break;
    case KEY_ARROW_LEFT:
    case KEY_ARROW_RIGHT:
    case KEY_ARROW_DOWN:
    case KEY_ARROW_UP:
      editor_move_cursor(c);
      break;
    case KEY_PAGE_UP:
    case KEY_PAGE_DOWN:
      for (unsigned int i = 0; i < E.screen_rows; i++)
        editor_move_cursor(c);
      break;
    case KEY_DELETE: {
      const unsigned int len = strlen(TEXTBUF.linebuf[textbufYPos]);
      if ((textbufXPos) < len)
        textbuf_delete_char(&TEXTBUF, textbufXPos, textbufYPos);
      break;
    }
    case KEY_HOME:
    case KEY_END:
      break;
    case 127:  // Backspace
      if (textbufXPos > 0){
        textbuf_delete_char(&TEXTBUF, textbufXPos - 1, textbufYPos);
        editor_move_cursor(KEY_ARROW_LEFT);
      } else if (textbufYPos > 0 && textbufXPos<TEXTBUF.size){
        textbuf_delete_line_break(&TEXTBUF, textbufYPos);
      }
    case 27:  // escape
      break;
    default:
      // for now ignore all control-keys
      if (c < 27) 
        break;
        // special characters are defined to be greater than 1000
      else if (c < 1000){ 
        textbuf_input_char(&TEXTBUF, c, textbufXPos, textbufYPos);
        editor_move_cursor(KEY_ARROW_RIGHT);
      }
      break;
  }
  return 1;
}

void editor_save_file(char *ptr){
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

static int append_welcome_message(struct abuf *ptr) {
  struct abuf *abptr = ptr;
  char welcome[80];
  // KILO_VERSION defined in main.c
  // snprintf is form <stdio.h>
  unsigned int welcomelen =
      snprintf(welcome, sizeof(welcome), "Kilo Editor -- Version %d.%d.%d",
               KILO_VERSION_MAJOR, KILO_VERSION_MINOR, KILO_VERSION_PATCH);
  if (welcomelen > E.screen_cols)
    welcomelen = E.screen_cols;

  // Center the Message
  unsigned int padding = (E.screen_cols - welcomelen) / 2;
  if (padding) {
    ab_append(abptr, "~", 1);
    padding--;
  }
  while (padding--)
    ab_append(abptr, " ", 1);

  ab_append(abptr, welcome, welcomelen);

  return 1;
}

void screen_buffer_append_debug_information(struct abuf *abptr){
  const int buf_size = 100;
  char *buf = (char*)malloc(buf_size);
  snprintf(buf, buf_size, 
           "TexbufX: %d; TexbufY: %d; ScreenY: %d; rows: %d; cols: %d",
            editor_get_cursor_textbuf_pos_x(), editor_get_cursor_textbuf_pos_y(),
            editor_get_cursor_textbuf_pos_y(),E.screen_rows, E.screen_cols);
  ab_append(DEB.debug_string, buf, strlen(buf));  // Append message to the global struct
  free(buf);
  ab_append(abptr, DEB.debug_string->b, DEB.debug_string->len);	
  ab_free(DEB.debug_string);
}

/*** Output ***/
void editor_draw_rows(struct abuf *abptr) {
  for (unsigned int nrows = 0; nrows < E.screen_rows ; nrows++) {  // number of iteration is siginificant!
    // the line number of the row to be drawn
    const unsigned int n_rows_to_draw = nrows + E.offset_y;

    // Create left margin (line number)
    char *leftMargin = (char *)calloc(E.left_margin_size, 1);
    int lineNumber;
    if (nrows ==  editor_get_cursor_screen_pos_y()){
      lineNumber = editor_get_cursor_textbuf_pos_y() + 1; // linenumber counts from 1
      snprintf(leftMargin, E.left_margin_size, "%d", lineNumber);
      // As in vim, the line number in the current line is aliged to right
      // Create necessary paddings
      if (strnlen_s(leftMargin, 256)<E.left_margin_size){
        // recall E.leftMarginSize is seted to 1 more than the maximum 
        // line number to include the extra space
        for (size_t i = 0; i < E.left_margin_size-strlen(leftMargin)-1; i++){
          ab_append(abptr, " ", 1);
        }
      }
      ab_append(abptr, "\x1b[1m", 4);
      ab_append(abptr, leftMargin, strnlen_s(leftMargin, 256));
      ab_append(abptr, "\x1b[0m", 4);
      // the extra space
      ab_append(abptr, " ", 1);
    } else {
      const int temp = nrows - editor_get_cursor_screen_pos_y();
      lineNumber = temp > 0 ? temp : -temp; 
      // Only display relative number for lines displayed by textbuf
      if ((nrows+ E.offset_y) < TEXTBUF.size){
        snprintf(leftMargin, E.left_margin_size, "%d", lineNumber);
      } 
      else {
        snprintf(leftMargin, E.left_margin_size, "~");
      }
      ab_append(abptr, leftMargin, strnlen_s(leftMargin, 256));
      // Create necessary paddings
      // recall E.leftMarginSize is seted to 1 more than the maximum 
      for (size_t i = 0; i < E.left_margin_size-strlen(leftMargin)-1; i++){
        ab_append(abptr, " ", 1);
      }
      // The extra space 
      ab_append(abptr, " ", 1);
    }

    if (n_rows_to_draw >= TEXTBUF.size) {
    }
    else if (nrows == E.screen_rows -1){ // For debugging purpose
      // screenBufferAppendDebugInformation(abptr);
		}
    else {
      if (TEXTBUF.linebuf == NULL) return; 
      // temp points to the string of the row to be drawn.
      char *temp = *(TEXTBUF.linebuf + n_rows_to_draw);
      const unsigned int stringlen = strlen(temp);
      // For calculate the spaces for direction scrolling.
      const unsigned int xoffset = E.offset_x >= stringlen ? stringlen : E.offset_x;
      temp += xoffset;
      // Calculate the correct display length of the buffer
      unsigned int bufferlen = stringlen - xoffset; // same as strlen(temp)
      bufferlen = (bufferlen >= E.screen_cols - E.left_margin_size) ? 
        E.screen_cols - E.left_margin_size : bufferlen;
      // ab_append(abptr, " ", 1);  // The space before the Line.
      ab_append(abptr, temp, bufferlen);
    }
		if (nrows<E.screen_rows - 1) ab_append(abptr, "\r\n", 2);
  }
}

void editor_refresh_screen(void) {
  // init append buffer
  struct abuf ab = ABUF_INIT;

  ab_append(&ab, "\x1b[?25l", 6); // Hide cursor

  editor_draw_rows(&ab);

  // Move mouse to correct position
  char buf[32];
	// move cursor, row:cols; top left is 1:1
  const unsigned int CursorScreenX = editor_get_cursor_screen_pos_x();
  const unsigned int CursorScreenY = editor_get_cursor_screen_pos_y();
  snprintf(buf, sizeof(buf), "\x1b[%d;%dH", CursorScreenY + 1, CursorScreenX + 1); 
  // ab_append(&ab, "\x1b[H", 3); 
  ab_append(&ab, buf, strlen(buf)); // To corrected position

  ab_append(&ab, "\x1b[?25h", 6); // Show cursor
  write(STDIN_FILENO, "\x1b[2J\x1b[H", 7);  // erase entire screen
  write(STDIN_FILENO, ab.b, ab.len);
  ab_free(&ab);
}

void editor_scroll_down(void) {
    E.offset_y++;
}

void editor_scroll_up(void) {
  	E.offset_y--;
}

void editor_scroll_left(void) {
  E.offset_x--;
}

void editor_scroll_right(void) {
  E.offset_x++;
}

void editor_cursor_x_to_textbuf_pos(unsigned int x){
	E.cursor_textbuf_pos_x = x;
}

void editor_cursor_y_to_textbuf_pos(unsigned int y){
	E.cursor_textbuf_pos_y = y;
}

void editor_move_cursor_to_end_of_line(textbuf * txb, unsigned int y){
  // input, textbuf, line number
  editor_cursor_x_to_textbuf_pos(
    textbuf_get_nth_line_length(txb, y));
  return;
}

void editor_confine_cursor_position(textbuf *txb, int xPos, int yPos){
  if (xPos<0)
    editor_cursor_x_to_textbuf_pos(0);

  int lineLength = textbuf_get_nth_line_length(txb, yPos);
  if (xPos >= lineLength){
    editor_cursor_x_to_textbuf_pos(lineLength);
  }
  if (yPos<0)
    editor_cursor_y_to_textbuf_pos(0);
  if (yPos>(txb->size))
    editor_cursor_y_to_textbuf_pos(txb->size);
}

// TODO: LEFT/WRITE ARROW MECHANISM
int editor_move_cursor(int key) {
  switch (key) {
  case KEY_ARROW_UP: 
  case KEY_PAGE_UP:
    if (E.cursor_textbuf_pos_y > 0) {
      if (editor_get_cursor_screen_pos_y() <= 0) --E.offset_y;
      E.cursor_textbuf_pos_y--;
    }
    break;
  case KEY_ARROW_DOWN:
  case KEY_PAGE_DOWN:
    if (E.cursor_textbuf_pos_y < TEXTBUF.size - 1){
      // screenPos counts from 0, screen rows counts from 1
      if (editor_get_cursor_screen_pos_y() >= E.screen_rows-1) ++E.offset_y;
      E.cursor_textbuf_pos_y ++;
    }
    break;
  case KEY_ARROW_LEFT: 
    // E.cursorTextbufPosX is unsigned int
    if (E.cursor_textbuf_pos_x > 0) 
      E.cursor_textbuf_pos_x--;
    break;
  case KEY_ARROW_RIGHT: 
    if (editor_cursor_movable_to_right(&TEXTBUF,
        editor_get_cursor_textbuf_pos_x(),
        editor_get_cursor_textbuf_pos_y())
      )
      E.cursor_textbuf_pos_x++;
    break;
  default:
    return -1;
  }

  // Move the cursor back to valid location
  // A valid location is a location in range of the textbuffer
  editor_confine_cursor_position(&TEXTBUF,
                              editor_get_cursor_textbuf_pos_x(),
                              editor_get_cursor_textbuf_pos_y());
  return 0;
}

// Set editorConfig.leftMarginSize according to 
// the digits of the maxium line number stored in textbuf
// The minimum size of leftMarginSize is 4
void editor_set_margin_size(struct editor_config *ptr,textbuf *ptrtb){
  int NumberOflines = ptrtb->size; 
  int counter;
  // Find out the digits of greatest linenumber
  for (counter = 0; NumberOflines>0; NumberOflines /= 10, counter ++);
  // One more space for padding ' '
  counter++;
  counter = (counter >= 4) ? counter : 4;
  ptr->left_margin_size = counter;
  return;
}


int editor_get_cursor_screen_pos_x(void){
  return E.cursor_textbuf_pos_x + E.left_margin_size - E.offset_x; 
}

int editor_get_cursor_screen_pos_y(void){
  return E.cursor_textbuf_pos_y - E.offset_y;
}

int editor_get_cursor_textbuf_pos_x(void){
  return E.cursor_textbuf_pos_x;
}

int editor_get_cursor_textbuf_pos_y(void){
  return E.cursor_textbuf_pos_y;
}

// See if the cursor can be moved further to write 
// with current cursro textbuf postion and textbuffer 
int editor_cursor_movable_to_right(
  textbuf * txb, unsigned int cursorTxbPosX, unsigned int cursorTxbPosY){
  return textbuf_get_nth_line_length(txb, cursorTxbPosY) >= cursorTxbPosX + 1; 
}
