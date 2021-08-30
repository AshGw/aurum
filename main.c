// #include <pthread.h>

#include "editor.h"
#include "globals.h"
#include "terminal.h"
#include "utils.h"

extern struct editor_config E;
extern struct program_utils PU;
extern struct key KEY;
extern struct debugUtil DEB;
extern textbuf TEXTBUF;

/*** init ***/
void init(void) {
  E.cursor_textbuf_pos_x = 0;
  E.cursor_textbuf_pos_y = 0;
  E.offset_y = 0;
  E.offset_x = 0;
  E.mode = 1; // 1 insert mode
  E.left_margin_size = 3;
  get_window_size(&E.screen_rows, &E.screen_cols); // from "terminal.h"

  program_utils_init(&PU);
  textbuf_init(&TEXTBUF);
  key_init(&KEY);
  debug_util_init(&DEB);

  enable_raw_mode(); // from "terminal.h"; enable Terminal RAW mode
}

int main(int argc, char *argv[], char *envp[]) {
  // TODO: Refactor Initialisation and reading file
  init();
  if (argc > 1) {
    editor_open(argv[1]);
    ab_append(&E.file_name, argv[1], strnlen_s(argv[1], 256));
  }
  if (argc <= 1) {
    editor_open("aaa.txt");
  }
  // Margin Size depends on the textbuffer read.
  editor_set_margin_size(&E, &TEXTBUF);
  while (PU.running) { // PU is global struct, [P]rogram [U]tils
    if (editor_read_key() == -1)
      die("editor_read_key Failed");
    if (PU.updated) {
      if (KEY.key[0]) {
        // Incase the margin size changes
        editor_set_margin_size(&E, &TEXTBUF);
        editor_process_key_press();
        key_refresh(&KEY);
      }
      editor_refresh_screen();
      PU.updated = 0;
    }
  }

  if (argc > 1) {
    editor_save_file(argv[1]);
  }
  if (argc <= 1) {
    editor_save_file("savedTo.txt");
  }
  clear_screen();
  return 0;
}
