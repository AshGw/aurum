#ifndef EDITOR_H
#define EDITOR_H 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int editor_read_key(void);
int editor_process_key_press(void);
void editor_refresh_screen(void);
void editor_init(void);
void editor_save_file(char *);
void editor_open(const char *);
int editor_move_cursor(int);
void editor_cursor_x_to_textbuf_pos(unsigned int);
void editor_cursor_y_to_textbuf_pos(unsigned int);
void editor_set_margin_size(struct editorConfig *, textbuf *);

void editor_scroll_down(void);

int editor_get_cursor_screen_pos_x(void);
int editor_get_cursor_screen_pos_y(void);

int editor_get_cursor_textbuf_pos_x(void);
int editor_get_cursor_textbuf_pos_y(void);
int editor_cursor_movable_to_right(textbuf *, unsigned int, unsigned int);
void editor_move_cursor_to_end_of_line(textbuf *, unsigned int);
void editor_confine_cursor_position(textbuf *, int x, int y);

#endif // For EDITOR_H
