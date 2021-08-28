#ifndef TERMINAL_H
#define TERMINAL_H 1
#include <errno.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

int get_cursor_position(unsigned int *, unsigned int *);
void get_window_size(unsigned int *, unsigned int *);
void enable_raw_mode(void);
void disable_raw_mode(void);
#endif // For TERMINAL_H
