#ifndef TERMINAL_H
#define TERMINAL_H 1
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <errno.h>
#include <termios.h>

int get_cursor_position(unsigned int *, unsigned int *);
void get_window_size(unsigned int *, unsigned int *);
void enable_raw_mode(void);
void disable_raw_mode(void);
#endif // For TERMINAL_H
